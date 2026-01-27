#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "GenererNoteAleatoire.hpp"
#include <doctest/doctest.h>
#include <regex>

TEST_CASE("GenererNoteAleatoire") {
    GenererNoteAleatoire gen;

    SUBCASE("Generer note in Do Majeur") {
        std::string note = gen.generer("Do", "Majeur");
        // Format check: Note name + Octave
        // e.g., C4, D#5.
        // Do Majeur: C, D, E, F, G, A, B. No accidentals except natural.
        // Wait, C, D, E... are names.
        // Regex: ^[A-G][#]?[0-9]$
        std::regex pattern("^[A-G][#]?[0-9]$");
        CHECK(std::regex_match(note, pattern));

        // Do Majeur should not have sharps/flats generally (except F# is not in
        // Do Maj, B is). The implementation has: "Do", {"C", "D", "E", "F",
        // "G", "A", "B"} So no # or b.
        CHECK(note.find('#') == std::string::npos);
        CHECK(note.find('b') == std::string::npos);
    }

    SUBCASE("Generer note in Sol Majeur") {
        // Sol (G Major): G, A, B, C, D, E, F#
        std::string note;
        // Since it's random, we might need multiple tries to see if we get F#.
        // But we can just check validity of one sample.
        note = gen.generer("Sol", "Majeur");
        CHECK(!note.empty());
    }

    SUBCASE("Generer Accord Do Majeur") {
        auto [nom, notes] = gen.genererAccord("Do", "Majeur");
        CHECK(!nom.empty());
        CHECK(notes.size() == 3);
        // Basic check of format
        for (const auto& n : notes) {
            std::regex pattern("^[A-G][#]?[0-9]$");
            CHECK(std::regex_match(n, pattern));
        }
    }

    SUBCASE("Generer Accord Renversement") {
        auto [nom, notes, renv] = gen.genererAccordRenversement("Do", "Majeur");
        CHECK(!nom.empty());
        CHECK(notes.size() == 3);
        CHECK(renv >= 1);
        CHECK(renv <= 3);
    }

    SUBCASE("Invalid parameters") {
        std::string note = gen.generer("Invalid", "Majeur");
        CHECK(note == "");

        auto [nom, notes] = gen.genererAccord("Invalid", "Majeur");
        CHECK(nom == "");
        CHECK(notes.empty());
    }
}