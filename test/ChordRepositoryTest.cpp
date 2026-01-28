#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ChordRepository.hpp"
#include <doctest/doctest.h>

TEST_CASE("ChordRepository initialization and retrieval") {
    ChordRepository ba;
    SUBCASE("Obtenir Accord Do I") {
        auto accord = ba.obtenirAccord("Do Majeur", "C");
        // Do Majeur (C Major) I is C E G -> MIDI 60, 64, 67 (usually)
        CHECK_FALSE(accord.empty());
        if (!accord.empty()) {
            bool hasC = false;
            for (int note : accord)
                if (note % 12 == 0)
                    hasC = true; // 0 is C in MIDI (0, 12, 24, 36, 48, 60...)
            CHECK(hasC);
        }
    }
    SUBCASE("Obtenir Accord invalide (Tonality exists, Degree missing)") {
        auto accord = ba.obtenirAccord("Do Majeur", "Z"); // Z doesn't exist
        CHECK(accord.empty());
    }
    SUBCASE("Obtenir Accord invalide (Tonality missing)") {
        auto accord = ba.obtenirAccord("NonExistent", "I");
        CHECK(accord.empty());
    }
}
