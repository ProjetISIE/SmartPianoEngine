#include "LectureNoteJouee.hpp"
#include "Logger.hpp"

#include <chrono>
#include <map>
#include <thread>

LectureNoteJouee::LectureNoteJouee() : midiIn(nullptr), noteDisponible(false) {
    Logger::log("[LectureNoteJouee] Instance créée");
}

LectureNoteJouee::~LectureNoteJouee() {
    Logger::log("[LectureNoteJouee] Instance détruite");
    fermer();
}

bool LectureNoteJouee::initialiser() {
    Logger::log("[LectureNoteJouee] Initialisation Audio & MIDI...");
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
    std::vector<std::string> notesAccord;
    while (midiIn) { // Tant que le message existe
        try {
            midiIn->getMessage(&message);
            if (!message.empty() && message.size() >= 3) {
                int status = message[0] & 0xF0;
                int noteMidi = message[1];
                int velocite = message[2];
                if (status == 0x90 && velocite > 0) {
                    std::string noteStr = convertirNote(noteMidi);
                    notesAccord.push_back(noteStr);
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
    if (midiIn) {
        delete midiIn;
        midiIn = nullptr;
    }
}
