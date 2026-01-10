#include "LectureNoteJouee.hpp"
#include "Logger.hpp"
#include <QCoreApplication>
#include <chrono>
#include <map>
#include <thread>

LectureNoteJouee::LectureNoteJouee()
    : midiIn(nullptr), midiOut(nullptr), noteDisponible(false) {
    Logger::log("[LectureNoteJouee] Instance créée");
}

LectureNoteJouee::~LectureNoteJouee() {
    Logger::log("[LectureNoteJouee] Instance détruite");
    fermer();
}

bool LectureNoteJouee::initialiser() {
    Logger::log("[LectureNoteJouee] Initialisation MIDI (JACK)...");
    try {
        midiIn = new RtMidiIn(RtMidi::Api::UNIX_JACK, "SmartPianoEngine");
        midiOut = new RtMidiOut(RtMidi::Api::UNIX_JACK, "SmartPianoEngine");
    } catch (RtMidiError& error) {
        Logger::log("[LectureNoteJouee] Erreur creation RtMidi JACK: " +
                        error.getMessage(),
                    true);

        return false;
    }
    try {
        midiIn->openVirtualPort("input");
        midiOut->openVirtualPort("output");

        midiIn->ignoreTypes(false, false, false);
        std::thread(&LectureNoteJouee::traiterMessagesMIDI, this).detach();

        Logger::log("[LectureNoteJouee] Ports JACK ouverts avec succes.");
        return true;

    } catch (RtMidiError& error) {
        Logger::log("[LectureNoteJouee] Erreur ouverture ports: " +
                        error.getMessage(),
                    true);

        return false;
    }
}

void LectureNoteJouee::traiterMessagesMIDI() {
    Logger::log("[LectureNoteJouee] Thread MIDI demarre");
    std::vector<unsigned char> message;
    using namespace std::chrono;
    milliseconds startAccord;
    milliseconds delaiAccord = milliseconds{500};
    bool isAccordEnCours = false;
    std::vector<std::string> notesAccord;
    while (midiIn) { // Tant que le message existe
    // while (true) {
    //     if (isAccordEnCours && duration_cast<milliseconds>(
    //                                system_clock::now().time_since_epoch()) -
    //                                    startAccord >
    //                                delaiAccord) {
    //         isAccordEnCours = false;
    //         notesAccord.clear();
    //     }
        try {
            if (!midiIn) break;
            midiIn->getMessage(&message);
            if (!message.empty() && message.size() >= 3) {
                int status = message[0] & 0xF0;
                int noteMidi = message[1];
                int velocite = message[2];
                if (status == 0x90 && velocite > 0) {
                    std::string noteStr = convertirNote(noteMidi);
                    notesAccord.push_back(noteStr);
                    Logger::log("[LectureNoteJouee] Note reçue : " + noteStr);
            // if (!message.empty()) {
            //     if (midiOut) {
            //         midiOut->sendMessage(&message);
            //     }
            //     if (message.size() >= 3) {
            //         int status = message[0] & 0xF0;
            //         int noteMidi = message[1];
            //         int velocite = message[2];
            //         if (status == 0x90 && velocite > 0) {
            //             if (!isAccordEnCours) {
            //                 isAccordEnCours = true;
            //                 startAccord = duration_cast<milliseconds>(
            //                     system_clock::now().time_since_epoch());
            //             }
            //             std::string note = convertirNote(noteMidi);
            //             notesAccord.push_back(note);
            //             Logger::log("[LectureNoteJouee] Note recue : " + note);
            //             {
            //                 std::lock_guard<std::mutex> lock(noteMutex);
            //                 dernierAccord = notesAccord;
            //                 noteDisponible = true;
            //             }
            //         }
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
    if (midiIn) {
        delete midiIn;
        midiIn = nullptr;
    }
    if (midiOut) {
        delete midiOut;
        midiOut = nullptr;
    }
}

void LectureNoteJouee::test() {} // DEBUG
