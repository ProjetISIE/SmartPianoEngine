#include "LectureNoteJouee.hpp"
#include "Logger.hpp"

#define TSF_IMPLEMENTATION
#include "tsf.h"

#define MINIAUDIO_IMPLEMENTATION
#include <miniaudio/miniaudio.h>

#include <chrono>
#include <map>
#include <thread>

#ifndef SOUNDFONT_PATH
#define SOUNDFONT_PATH "piano.sf2"
#endif

LectureNoteJouee::LectureNoteJouee()
    : midiIn(nullptr), soundFont(nullptr), audioDevice(nullptr),
      noteDisponible(false) {
    audioDevice = new ma_device();
    Logger::log("[LectureNoteJouee] Instance créée");
}

LectureNoteJouee::~LectureNoteJouee() {
    Logger::log("[LectureNoteJouee] Destruction de l'instance");
    fermer();
    delete audioDevice; // Nettoyage final de la mémoire
}

void LectureNoteJouee::audioCallback(ma_device* pDevice, void* pOutput,
                                     const void* pInput,
                                     unsigned int frameCount) {
    LectureNoteJouee* self = (LectureNoteJouee*)pDevice->pUserData;
    if (!self->soundFont) {
        memset(pOutput, 0,
               frameCount * ma_get_bytes_per_frame(pDevice->playback.format,
                                                   pDevice->playback.channels));
        return;
    }
    std::lock_guard<std::mutex> lock(self->synthMutex);
    tsf_render_short(self->soundFont, (short*)pOutput, frameCount, 0);
}

bool LectureNoteJouee::initialiser() {
    Logger::log("[LectureNoteJouee] Initialisation Audio & MIDI...");
    Logger::log("[LectureNoteJouee] Chargement du SoundFont : " SOUNDFONT_PATH);
    soundFont = tsf_load_filename(SOUNDFONT_PATH);
    if (!soundFont) {
        Logger::log("[LectureNoteJouee] Erreur chargement systeme. Tentative "
                    "locale 'piano.sf2'...",
                    true);
        soundFont = tsf_load_filename("piano.sf2");
        if (!soundFont) {
            Logger::log("[LectureNoteJouee] CRITIQUE : Impossible de charger "
                        "le SoundFont.",
                        true);
            return false;
        }
    }
    tsf_set_output(soundFont, TSF_STEREO_INTERLEAVED, 44100, 0);
    ma_device_config config = ma_device_config_init(ma_device_type_playback);
    config.playback.format = ma_format_s16; // 16-bit
    config.playback.channels = 2;           // Stéréo
    config.sampleRate = 44100;
    config.dataCallback = audioCallback; // Notre fonction statique ci-dessus
    config.pUserData = this; // On passe 'this' pour y accéder dans le callback

    if (ma_device_init(nullptr, &config, audioDevice) != MA_SUCCESS) {
        Logger::log("[LectureNoteJouee] ERREUR : Impossible d'initialiser le "
                    "périphérique audio.",
                    true);
        return false;
    }

    if (ma_device_start(audioDevice) != MA_SUCCESS) {
        Logger::log(
            "[LectureNoteJouee] ERREUR : Impossible de démarrer le flux audio.",
            true);
        return false;
    }
    Logger::log("[LectureNoteJouee] Moteur Audio démarré avec succès.");
    try {
        midiIn = new RtMidiIn(RtMidi::Api::UNSPECIFIED, "SmartPianoEngine");
        midiIn->openVirtualPort("input");
        midiIn->ignoreTypes(false, false, false);
        std::thread(&LectureNoteJouee::traiterMessagesMIDI, this).detach();
        Logger::log("[LectureNoteJouee] Entrée MIDI initialisée.");
        return true;
    } catch (RtMidiError& error) {
        Logger::log("[LectureNoteJouee] Erreur MIDI : " + error.getMessage(),
                    true);
        return false;
    }
}

void LectureNoteJouee::traiterMessagesMIDI() {
    Logger::log("[LectureNoteJouee] Thread MIDI démarré");
    std::vector<unsigned char> message;

    using namespace std::chrono;
    milliseconds startAccord;
    milliseconds delaiAccord =
        milliseconds{500}; // Délai pour grouper les notes en accord
    bool isAccordEnCours = false;
    std::vector<std::string> notesAccord;

    while (midiIn) { // Tant que l'objet existe
        if (isAccordEnCours && duration_cast<milliseconds>(
                                   system_clock::now().time_since_epoch()) -
                                       startAccord >
                                   delaiAccord) {
            isAccordEnCours = false;
            notesAccord.clear();
        }
        try {
            midiIn->getMessage(&message);
            if (!message.empty() && message.size() >= 3) {
                int status = message[0] & 0xF0;
                int noteMidi = message[1];
                int velocite = message[2];
                {
                    std::lock_guard<std::mutex> lock(synthMutex);
                    if (status == 0x90 && velocite > 0) {
                        tsf_note_on(soundFont, 0, noteMidi, velocite / 127.0f);
                    } else if (status == 0x80 ||
                               (status == 0x90 && velocite == 0)) {
                        tsf_note_off(soundFont, 0, noteMidi);
                    }
                }
                if (status == 0x90 && velocite > 0) {
                    if (!isAccordEnCours) {
                        isAccordEnCours = true;
                        startAccord = duration_cast<milliseconds>(
                            system_clock::now().time_since_epoch());
                    }

                    std::string noteStr = convertirNote(noteMidi);
                    notesAccord.push_back(noteStr);
                    Logger::log("[LectureNoteJouee] Note reçue : " + noteStr);

                    {
                        std::lock_guard<std::mutex> lock(noteMutex);
                        dernierAccord = notesAccord;
                        noteDisponible = true;
                    }
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
    if (audioDevice) {
        ma_device_uninit(audioDevice);
    }
    if (soundFont) {
        tsf_close(soundFont);
        soundFont = nullptr;
    }
    if (midiIn) {
        delete midiIn;
        midiIn = nullptr;
    }
}
