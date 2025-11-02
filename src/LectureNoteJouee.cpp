#include "LectureNoteJouee.h"
#include "GestionSon.h"
#include "Logger.h"
#include <QCoreApplication>
#include <chrono>
#include <iostream>
#include <map>
#include <thread>

LectureNoteJouee::LectureNoteJouee() : midiIn(nullptr), noteDisponible(false) {
    Logger::log("[LectureNoteJouee] Ligne 12 : Instance creee");
}

LectureNoteJouee::~LectureNoteJouee() {
    Logger::log("[LectureNoteJouee] Ligne 16 : Destruction de l'instance");
    fermer();
}

GestionSon gestionSon(QCoreApplication::instance());

// Initialisation de la lecture MIDI
bool LectureNoteJouee::initialiser(bool jouerSonON) {
    this->jouerSonON = jouerSonON;
    Logger::log(
        "[LectureNoteJouee] Ligne 25 : Initialisation commencee, jouerSonON=" +
        std::to_string(jouerSonON));
    Logger::log("[LectureNoteJouee] Ligne 26 : RtMidi Version : " +
                std::string(RtMidi::getVersion()));

    // Verifier les APIs MIDI compilees
    test();

    try {
        Logger::log(
            "[LectureNoteJouee] Ligne 32 : Creation de l'objet RtMidiIn");
        midiIn = new RtMidiIn(RtMidi::Api::LINUX_ALSA, "LectureNoteJouee");
        if (!midiIn) {
            Logger::log("[LectureNoteJouee] Ligne 35 : Erreur : Impossible de "
                        "creer l'objet RtMidiIn",
                        true);
            return false;
        }

        unsigned int portCount = midiIn->getPortCount();
        Logger::log(
            "[LectureNoteJouee] Ligne 40 : Nombre de ports MIDI detectes : " +
            std::to_string(portCount));

        if (portCount == 0) {
            Logger::log(
                "[LectureNoteJouee] Ligne 43 : Aucun peripherique MIDI detecte",
                true);
            delete midiIn;
            midiIn = nullptr;
            return false;
        }

        for (unsigned int i = 0; i < portCount; ++i) {
            try {
                std::string portName = midiIn->getPortName(i);
                Logger::log("[LectureNoteJouee] Ligne 52 : Port detecte : " +
                            portName);

                if (portName.find("SWISSONIC EasyKeys49") !=
                    std::string::npos) {
                    midiIn->openPort(i);
                    Logger::log(
                        "[LectureNoteJouee] Ligne 56 : Port MIDI ouvert : " +
                        portName);

                    midiIn->ignoreTypes(false, false, false);

                    std::thread(&LectureNoteJouee::traiterMessagesMIDI, this)
                        .detach();

                    Logger::log("[LectureNoteJouee] Ligne 62 : Lecture MIDI "
                                "initialisee avec succes");
                    return true;
                }
            } catch (RtMidiError& error) {
                Logger::log("[LectureNoteJouee] Ligne 66 : Erreur : " +
                                error.getMessage(),
                            true);
            }
        }

        Logger::log("[LectureNoteJouee] Ligne 70 : Aucun port correspondant "
                    "trouve pour 'SWISSONIC EasyKeys49'",
                    true);
        delete midiIn;
        midiIn = nullptr;
        return false;
    } catch (RtMidiError& error) {
        Logger::log("[LectureNoteJouee] Ligne 75 : Exception RtMidi : " +
                        error.getMessage(),
                    true);
        if (midiIn) {
            delete midiIn;
            midiIn = nullptr;
        }
        return false;
    } catch (const std::exception& e) {
        Logger::log("[LectureNoteJouee] Ligne 82 : Exception generale : " +
                        std::string(e.what()),
                    true);
        if (midiIn) {
            delete midiIn;
            midiIn = nullptr;
        }
        return false;
    }
}

// Traitement des messages MIDI
void LectureNoteJouee::traiterMessagesMIDI() {
    Logger::log(
        "[LectureNoteJouee] Ligne 94 : Thread de traitement MIDI demarre");
    std::vector<unsigned char> message;

    using namespace std::chrono;
    milliseconds startAccord;
    milliseconds delaiAccord = milliseconds{500};
    bool isAccordEnCours = false;
    std::vector<std::string> notesAccord;

    while (true) {
        if (isAccordEnCours && duration_cast<milliseconds>(
                                   system_clock::now().time_since_epoch()) -
                                       startAccord >
                                   delaiAccord) {
            isAccordEnCours = false;

            Logger::log("[LectureNoteJouee] Ligne 107 : Accord termine, nombre "
                        "de notes : " +
                        std::to_string(notesAccord.size()));
            notesAccord.clear();
        }

        try {
            if (!midiIn) {
                Logger::log("[LectureNoteJouee] Ligne 113 : Erreur : Objet "
                            "midiIn non initialise",
                            true);
                break;
            }

            midiIn->getMessage(&message);

            if (!message.empty()) {
                if (message.size() >= 3 && (message[0] & 0xF0) == 0x90) {
                    int noteMidi = message[1];
                    int velocite = message[2];

                    if (velocite > 0) {
                        if (!isAccordEnCours) {
                            isAccordEnCours = true;
                            startAccord = duration_cast<milliseconds>(
                                system_clock::now().time_since_epoch());
                        }
                        std::string note = convertirNote(noteMidi);
                        notesAccord.push_back(note);

                        Logger::log(
                            "[LectureNoteJouee] Ligne 132 : Note recue : " +
                            note);

                        {
                            std::lock_guard<std::mutex> lock(noteMutex);
                            dernierAccord = notesAccord;
                            noteDisponible = true;
                        }
                        if (jouerSonON) {
                            std::thread(&GestionSon::jouerSon, &gestionSon,
                                        note)
                                .detach();
                            Logger::log("[LectureNoteJouee] Ligne 141 : Son "
                                        "joue pour la note : " +
                                        note);
                        }
                    }
                }
            }
        } catch (const std::exception& e) {
            Logger::log("[LectureNoteJouee] Ligne 147 : Exception : " +
                            std::string(e.what()),
                        true);
        } catch (...) {
            Logger::log("[LectureNoteJouee] Ligne 149 : Erreur inconnue", true);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

// Conversion d'une note MIDI en nom de note
std::string LectureNoteJouee::convertirNote(int noteMidi) {
    static const std::map<int, std::string> notes = {
        {0, "C"},  {1, "C#"}, {2, "D"},  {3, "D#"}, {4, "E"},   {5, "F"},
        {6, "F#"}, {7, "G"},  {8, "G#"}, {9, "A"},  {10, "A#"}, {11, "B"}};
    int octave = (noteMidi / 12) - 1;
    int noteIndex = noteMidi % 12;
    std::string result = notes.at(noteIndex) + std::to_string(octave);

    Logger::log("[LectureNoteJouee] Ligne 166 : Conversion MIDI : " +
                std::to_string(noteMidi) + " -> " + result);
    return result;
}

// Lecture d'un accord
std::vector<std::string> LectureNoteJouee::lireNote() {
    Logger::log("[LectureNoteJouee] Ligne 172 : En attente d'un accord...");
    while (!noteDisponible.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::vector<std::string> accord;
    {
        std::lock_guard<std::mutex> lock(noteMutex);
        accord = dernierAccord;
        noteDisponible = false;
    }

    Logger::log("[LectureNoteJouee] Ligne 184 : Accord lu, nombre de notes : " +
                std::to_string(accord.size()));
    return accord;
}

// Fermeture de l'objet MIDI
void LectureNoteJouee::fermer() {
    if (midiIn) {
        delete midiIn;
        midiIn = nullptr;
        Logger::log("[LectureNoteJouee] Ligne 193 : MIDI ferme");
    }
}

// Test des APIs MIDI disponibles
void LectureNoteJouee::test() {
    std::vector<RtMidi::Api> apis;
    RtMidi::getCompiledApi(apis);

    for (const auto& api : apis) {
        Logger::log("[LectureNoteJouee] Ligne 203 : API disponible : " +
                    RtMidi::getApiName(api));
    }
}