#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "BaseAccords.hpp"
#include <doctest/doctest.h>

TEST_CASE("BaseAccords initialization and retrieval") {
    BaseAccords ba;

    // Check some known chords if possible.
    // Since I don't see the .cpp, I rely on common music theory or what I might
    // expect. Or I just check that it returns something non-empty for likely
    // keys. The header comment says: tonalite "Do", degre "I" -> {Notes MIDI}

    SUBCASE("Obtenir Accord Do I") {
        auto accord = ba.obtenirAccord("Do Majeur", "C");
        // Do Majeur (C Major) I is C E G -> MIDI 60, 64, 67 (usually)
        CHECK_FALSE(accord.empty());
        if (!accord.empty()) {
            // Check for C (60, 72, etc)
            // Just checking it's not empty is a good start if we don't know the
            // exact mapping used in .cpp But usually 60 is Middle C.
            bool hasC = false;
            for (int note : accord) {
                if (note % 12 == 0)
                    hasC = true; // 0 is C in MIDI (0, 12, 24, 36, 48, 60...)
            }
            CHECK(hasC);
        }
    }

    SUBCASE("Obtenir Accord invalid") {
        auto accord = ba.obtenirAccord("NonExistent", "I");
        CHECK(accord.empty());
    }
}