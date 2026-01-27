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

TEST_CASE("ValidationNote::validerAccordRenversement") {
    ValidationNote vn;
    std::vector<std::string> attendu = {"c4", "e4", "g4"}; // C Major Root

    SUBCASE("Root position (renversement=1)") {
        std::vector<std::string> joue = {"c4", "e4", "g4"};
        CHECK(vn.validerAccordRenversement(joue, attendu, 1));
    }

    SUBCASE("1st inversion (renversement=2)") {
        std::vector<std::string> joue = {"e4", "g4", "c5"};
        CHECK(vn.validerAccordRenversement(joue, attendu, 2));
    }

    SUBCASE("2nd inversion (renversement=3)") {
        std::vector<std::string> joue = {"g4", "c5", "e5"};
        CHECK(vn.validerAccordRenversement(joue, attendu, 3));
    }

    SUBCASE("Wrong inversion") {
        std::vector<std::string> joue = {"c4", "e4", "g4"};
        CHECK_FALSE(vn.validerAccordRenversement(joue, attendu, 2));
    }
}
