#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../include/LectureNoteJouee.h"
#include <doctest/doctest.h>
#include <vector>

class TestLectureNoteJouee : public LectureNoteJouee {
  public:
    std::string testerConvertirNote(int noteMidi) {
        return convertirNote(noteMidi);
    }

    void simulerAccord(std::vector<std::string> notes) {
        std::lock_guard<std::mutex> lock(noteMutex);
        dernierAccord = notes;
        noteDisponible = true;
    }

    std::vector<std::string> getDernierAccord() { return dernierAccord; }
};

// Test de la conversion des notes MIDI
TEST_CASE("LectureNoteJouee - Conversion des notes MIDI") {
    TestLectureNoteJouee lecteur;
    CHECK(lecteur.testerConvertirNote(60) == "C4");
    CHECK(lecteur.testerConvertirNote(61) == "C#4");
    CHECK(lecteur.testerConvertirNote(62) == "D4");
    CHECK(lecteur.testerConvertirNote(69) == "A4");
    CHECK(lecteur.testerConvertirNote(72) == "C5");
}

// Test de la lecture d'un accord
TEST_CASE("LectureNoteJouee - Lecture d'un accord") {
    TestLectureNoteJouee lecteur;
    std::vector<std::string> notesSimulees = {"C4", "E4", "G4"};
    lecteur.simulerAccord(notesSimulees);

    std::vector<std::string> accordLu = lecteur.getDernierAccord();
    CHECK(accordLu == notesSimulees);
}

// Test de la fermeture propre de l'objet MIDI
TEST_CASE("LectureNoteJouee - Fermeture MIDI") {
    TestLectureNoteJouee lecteur;
    CHECK_NOTHROW(lecteur.fermer());
}
