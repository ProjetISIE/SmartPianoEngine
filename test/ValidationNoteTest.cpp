#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ValidationNote.hpp"
#include <doctest/doctest.h>

TEST_CASE("ValidationNote::valider") {
    ValidationNote vn;
    CHECK(vn.valider("c4", "c4"));
    CHECK_FALSE(vn.valider("c4", "d4"));
    CHECK(vn.valider("d#5", "d#5"));
}

TEST_CASE("ValidationNote::validerAccordSR (Sans Renversement)") {
    ValidationNote vn;
    std::vector<std::string> attendu = {"c4", "e4", "g4"};

    SUBCASE("Correct chord same order") {
        std::vector<std::string> joue = {"c4", "e4", "g4"};
        CHECK(vn.validerAccordSR(joue, attendu));
    }

    SUBCASE("Correct chord different order") {
        // SR ignores order? The doc says "ignorant l'ordre des notes"
        std::vector<std::string> joue = {"g4", "c4", "e4"};
        CHECK(vn.validerAccordSR(joue, attendu));
    }

    SUBCASE("Incorrect chord") {
        std::vector<std::string> joue = {"c4", "e4", "a4"};
        CHECK_FALSE(vn.validerAccordSR(joue, attendu));
    }

    SUBCASE("Incorrect size") {
        std::vector<std::string> joue = {"c4", "e4"};
        CHECK_FALSE(vn.validerAccordSR(joue, attendu));
    }
}

// Note: Testing validerAccordRenversement might require knowing how it's
// implemented. Based on the header doc: renversement = 1 => Pas de renversement
// renversement = 2 => 1er renversement
// renversement = 3 => 2eme renversement
// It says "recalibrant les notes".
// Without seeing the .cpp, I assume it checks if the played chord matches the
// expected chord *after* applying the inversion logic to expected or played?
// Usually, C major root position: C E G
// 1st inversion: E G C (often C becomes C one octave higher, e.g. E4 G4 C5)
// The doc example says: joue {"E4", "G4", "C5"}, attendu {"C4", "E4", "G4"},
// renv=2. So I will test that specific case.

TEST_CASE("ValidationNote::validerAccordRenversement") {
    ValidationNote vn;
    std::vector<std::string> attendu = {"c4", "e4", "g4"}; // C Major Root

    SUBCASE("Root position (renversement=1)") {
        std::vector<std::string> joue = {"c4", "e4", "g4"};
        CHECK(vn.validerAccordRenversement(joue, attendu, 1));
    }

    SUBCASE("1st inversion (renversement=2)") {
        // C4 becomes C5
        std::vector<std::string> joue = {"e4", "g4", "c5"};
        CHECK(vn.validerAccordRenversement(joue, attendu, 2));
    }

    SUBCASE("2nd inversion (renversement=3)") {
        // C4->C5, E4->E5
        std::vector<std::string> joue = {"g4", "c5", "e5"};
        CHECK(vn.validerAccordRenversement(joue, attendu, 3));
    }

    SUBCASE("Wrong inversion") {
        std::vector<std::string> joue = {"c4", "e4", "g4"};
        CHECK_FALSE(vn.validerAccordRenversement(joue, attendu, 2));
    }
}