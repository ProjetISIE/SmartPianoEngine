#include "LectureNoteJouee.hpp"
#include "Logger.hpp"

#include <QResource>
#include <chrono>
#include <map>
#include <thread>

#define TSF_IMPLEMENTATION
#include "tsf.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>

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
        unsigned int nPorts = midiIn->getPortCount();
        int portToOpen = -1;
        if (nPorts == 0) {
            Logger::log("[LectureNoteJouee] Aucun périphérique physique. "
                        "Création d'un port virtuel 'input'.");
            midiIn->openVirtualPort("input");
        } else {
            Logger::log("[LectureNoteJouee] " + std::to_string(nPorts) +
                        " port(s) détecté(s). Recherche du clavier...");
            for (unsigned int i = 0; i < nPorts; i++) {
                std::string portName = midiIn->getPortName(i);
                Logger::log("  - Port " + std::to_string(i) + ": " + portName);
                if (portToOpen == -1 &&
                    portName.find("Midi Through") == std::string::npos) {
                    portToOpen = i;
                }
            }
            if (portToOpen == -1) portToOpen = 0;
            midiIn->openPort(portToOpen);
            Logger::log(
                "[LectureNoteJouee] SUCCÈS : Connecté automatiquement à '" +
                midiIn->getPortName(portToOpen) + "'");
        }
        midiIn->ignoreTypes(false, false, false);
    } catch (RtMidiError& error) {
        Logger::log("[LectureNoteJouee] Erreur Critique MIDI : " +
                        error.getMessage(),
                    true);
        return false;
    }
    QResource soundFontResource(":/piano.sf2");
    if (soundFontResource.isValid()) {
        g_tsf =
            tsf_load_memory(soundFontResource.data(), soundFontResource.size());

        if (g_tsf) {
            tsf_set_output(g_tsf, TSF_STEREO_INTERLEAVED, 44100, 0.0f);
            Logger::log(
                "[LectureNoteJouee] SoundFont chargé depuis le binaire.");
        }
    } else {
        Logger::log(
            "[LectureNoteJouee] Erreur: Ressource ':/piano.sf2' introuvable.",
            true);
    }
    if (!g_tsf) {
        Logger::log(
            "[LectureNoteJouee] ECHEC : Impossible de charger le son (Piano).",
            true);
    }
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format = ma_format_f32;
    config.playback.channels = 2;
    config.sampleRate = 44100;
    config.dataCallback = data_callback;
    config.pUserData = g_tsf;
    if (ma_device_init(NULL, &config, &device) != MA_SUCCESS) {
        Logger::log("[LectureNoteJouee] Echec de l'initialisation de Miniaudio",
                    true);
        if (g_tsf) {
            tsf_close(g_tsf);
            g_tsf = nullptr;
        }
    } else {
        ma_device_start(&device);
        audioInitialized = true;
        Logger::log("[LectureNoteJouee] Moteur Audio Démarré.");
    }
    std::thread(&LectureNoteJouee::traiterMessagesMIDI, this).detach();
    return true;
}

void LectureNoteJouee::traiterMessagesMIDI() {
    Logger::log("[LectureNoteJouee] Thread MIDI démarré");
    std::vector<unsigned char> message;
    while (midiIn) {
        try {
            midiIn->getMessage(&message);
            if (!message.empty() && message.size() >= 3) {
                int status = message[0] & 0xF0;
                int noteMidi = message[1];
                int velocite = message[2];
                if (g_tsf) {
                    if (status == 0x90 && velocite > 0) {
                        tsf_note_on(g_tsf, 0, noteMidi, velocite / 127.0f);
                    } else if (status == 0x80 ||
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
        noteDisponible = false;
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
