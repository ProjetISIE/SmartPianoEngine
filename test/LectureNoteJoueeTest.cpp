#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "LectureNoteJouee.hpp"
#include <doctest/doctest.h>

TEST_CASE("LectureNoteJouee conversion") {
    LectureNoteJouee lnj;
    CHECK(lnj.testerConvertirNote(60) == "C4");
    CHECK(lnj.testerConvertirNote(61) == "C#4");
    CHECK(lnj.testerConvertirNote(69) == "A4");
    CHECK(lnj.testerConvertirNote(21) == "A0");
}
