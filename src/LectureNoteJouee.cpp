#include "LectureNoteJouee.hpp"
#include "Logger.hpp"

#include <QByteArray>
#include <QFile>
#include <QStandardPaths>
#include <chrono>
#include <map>
#include <thread>

#define TSF_IMPLEMENTATION
#include "tsf.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>

#ifndef SOUNDFONT_PATH
#define SOUNDFONT_PATH "piano.sf2"
#endif

LectureNoteJouee::LectureNoteJouee()
    : midiIn(nullptr), g_tsf(nullptr), audioInitialized(false),
      noteDisponible(false) {
    Logger::log("[LectureNoteJouee] Instance créée");
}

LectureNoteJouee::~LectureNoteJouee() {
    Logger::log("[LectureNoteJouee] Instance détruite");
    fermer();
}

void LectureNoteJouee::data_callback(ma_device* pDevice, void* pOutput,
                                     [[maybe_unused]] const void* pInput,
                                     ma_uint32 frameCount) {
    tsf* g_tsf = (tsf*)pDevice->pUserData;
    tsf_render_float(g_tsf, (float*)pOutput, frameCount, 0);
}

bool LectureNoteJouee::initialiser() {
    Logger::log("[LectureNoteJouee] Initialisation Audio & MIDI...");
    try {
        midiIn = new RtMidiIn(RtMidi::Api::UNSPECIFIED, "SmartPianoEngine");
        midiIn->openVirtualPort("input");
        midiIn->ignoreTypes(false, false, false);
        Logger::log("[LectureNoteJouee] Entrée MIDI initialisée.");
    } catch (RtMidiError& error) {
        Logger::log("[LectureNoteJouee] Erreur MIDI : " + error.getMessage(),
                    true);
        return false;
    }
    QFile soundFontFile(":/piano.sf2");
    if (soundFontFile.open(QIODevice::ReadOnly)) {
        QByteArray sfData = soundFontFile.readAll();
        g_tsf = tsf_load_memory(sfData.constData(), sfData.size());
        if (g_tsf) {
            Logger::log(
                "[LectureNoteJouee] SoundFont intégré chargé avec succès.");
        }
    }
    if (!g_tsf) {
        Logger::log("[LectureNoteJouee] Erreur: Impossible de charger le "
                    "SoundFont depuis les ressources.",
                    true);
    } else {
        tsf_set_output(g_tsf, TSF_STEREO_INTERLEAVED, 44100, 0.0f);
    }
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format = ma_format_f32; // Format Float 32-bit
    config.playback.channels = 2;           // Stéréo
    config.sampleRate = 44100;
    config.dataCallback = data_callback; // Notre fonction de rendu
    config.pUserData = g_tsf;            // On passe le pointeur TSF au callback
    if (ma_device_init(NULL, &config, &device) != MA_SUCCESS) {
        Logger::log("[LectureNoteJouee] Echec de l'initialisation de Miniaudio",
                    true);
        if (g_tsf) {
            tsf_close(g_tsf);
            g_tsf = nullptr;
        }
    } else {
        ma_device_start(&device); // Démarrage du thread audio
        audioInitialized = true;
        Logger::log("[LectureNoteJouee] Moteur Audio Démarré (Miniaudio).");
    }
    std::thread(&LectureNoteJouee::traiterMessagesMIDI, this).detach();
    return true;
}

void LectureNoteJouee::traiterMessagesMIDI() {
    Logger::log("[LectureNoteJouee] Thread MIDI démarré");
    std::vector<unsigned char> message;
    std::vector<std::string> notesAccord;
    while (midiIn) {
        try {
            midiIn->getMessage(&message);
            if (!message.empty() && message.size() >= 3) {
                int status = message[0] & 0xF0;
                int noteMidi = message[1];
                int velocite = message[2];
                if (g_tsf) {
                    // Note ON (0x90) avec vélocité > 0
                    if (status == 0x90 && velocite > 0) {
                        tsf_note_on(g_tsf, 0, noteMidi, velocite / 127.0f);
                    }
                    // Note OFF (0x80) ou Note ON avec vélocité 0
                    else if (status == 0x80 ||
                             (status == 0x90 && velocite == 0)) {
                        tsf_note_off(g_tsf, 0, noteMidi);
                    }
                }
                if (status == 0x90 && velocite > 0) {
                    std::string noteStr = convertirNote(noteMidi);
                    {
                        std::lock_guard<std::mutex> lock(noteMutex);
                        dernierAccord.clear();
                        dernierAccord.push_back(noteStr);
                        noteDisponible = true;
                    }
                    Logger::log("[LectureNoteJouee] Note reçue : " + noteStr);
                }
            }
        } catch (const std::exception& e) {
            Logger::log(
                std::string("[LectureNoteJouee] Exception: ") + e.what(), true);
        }
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

std::string LectureNoteJouee::convertirNote(int noteMidi) {
    static const std::map<int, std::string> notes = {
        {0, "C"},  {1, "C#"}, {2, "D"},  {3, "D#"}, {4, "E"},   {5, "F"},
        {6, "F#"}, {7, "G"},  {8, "G#"}, {9, "A"},  {10, "A#"}, {11, "B"}};
    int octave = (noteMidi / 12) - 1;
    int noteIndex = noteMidi % 12;
    if (notes.find(noteIndex) == notes.end()) return "Unknown";
    return notes.at(noteIndex) + std::to_string(octave);
}

std::vector<std::string> LectureNoteJouee::lireNote() {
    while (!noteDisponible.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::vector<std::string> accord;
    {
        std::lock_guard<std::mutex> lock(noteMutex);
        accord = dernierAccord;
        noteDisponible = false; // Reset du flag
    }
    return accord;
}

void LectureNoteJouee::fermer() {
    Logger::log("[LectureNoteJouee] Fermeture des ressources...");
    if (audioInitialized) {
        ma_device_uninit(&device);
        audioInitialized = false;
    }
    if (g_tsf) {
        tsf_close(g_tsf);
        g_tsf = nullptr;
    }
    if (midiIn) {
        delete midiIn;
        midiIn = nullptr;
    }
}
