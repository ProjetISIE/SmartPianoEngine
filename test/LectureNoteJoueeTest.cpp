#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "LectureNoteJouee.hpp"
#include <doctest/doctest.h>

TEST_CASE("LectureNoteJouee conversion") {
    LectureNoteJouee lnj;

    // testerConvertirNote is a public helper for testing protected method

    // MIDI 60 = C4
    CHECK(lnj.testerConvertirNote(60) == "C4");

    // MIDI 61 = C#4
    CHECK(lnj.testerConvertirNote(61) == "C#4");

    // MIDI 69 = A4 (440Hz)
    CHECK(lnj.testerConvertirNote(69) == "A4");

    // MIDI 21 = A0 (Piano lowest)
    CHECK(lnj.testerConvertirNote(21) == "A0");
}

// Cannot test initialiser() easily as it requires JACK.