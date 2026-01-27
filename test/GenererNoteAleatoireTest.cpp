#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "GenererNoteAleatoire.hpp"
#include <doctest/doctest.h>
#include <regex>

TEST_CASE("GenererNoteAleatoire") {
    GenererNoteAleatoire gen;
    SUBCASE("Generer note in Do Majeur") {
        std::string note = gen.generer("Do", "Majeur");
        std::regex pattern("^[A-G][#]?[0-9]$");
        CHECK(std::regex_match(note, pattern));
        CHECK(note.find('#') == std::string::npos);
        CHECK(note.find('b') == std::string::npos);
    }

    SUBCASE("Generer note in Sol Majeur") {
        std::string note;
        note = gen.generer("Sol", "Majeur");
        CHECK(!note.empty());
    }

    SUBCASE("Generer Accord Do Majeur") {
        auto [nom, notes] = gen.genererAccord("Do", "Majeur");
        CHECK(!nom.empty());
        CHECK(notes.size() == 3);
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
