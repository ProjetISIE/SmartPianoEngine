#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "AnswerValidator.hpp"
#include <doctest/doctest.h>

/// Vérifie la validation de notes individuelles
/// Compare notes jouées vs attendues (égalité stricte)
TEST_CASE("AnswerValidator::valider") {
    AnswerValidator vn;
    CHECK(vn.valider("c4", "c4"));
    CHECK_FALSE(vn.valider("c4", "d4"));
    CHECK(vn.valider("d#5", "d#5"));
}

/// Vérifie la validation d'accords sans renversement
/// Les notes doivent correspondre, ordre non important
TEST_CASE("AnswerValidator::validerAccordSR (Sans Renversement)") {
    AnswerValidator vn;
    std::vector<std::string> attendu = {"c4", "e4", "g4"};

    /// Vérifie accord correct joué dans le même ordre
    SUBCASE("Correct chord same order") {
        std::vector<std::string> joue = {"c4", "e4", "g4"};
        CHECK(vn.validerAccordSR(joue, attendu));
    }

    /// Vérifie accord correct joué dans ordre différent
    SUBCASE("Correct chord different order") {
        std::vector<std::string> joue = {"g4", "c4", "e4"};
        CHECK(vn.validerAccordSR(joue, attendu));
    }

    /// Vérifie rejet accord avec note incorrecte
    SUBCASE("Incorrect chord") {
        std::vector<std::string> joue = {"c4", "e4", "a4"};
        CHECK_FALSE(vn.validerAccordSR(joue, attendu));
    }

    /// Vérifie rejet accord avec nombre incorrect de notes
    SUBCASE("Incorrect size") {
        std::vector<std::string> joue = {"c4", "e4"};
        CHECK_FALSE(vn.validerAccordSR(joue, attendu));
    }
}

/// Vérifie la validation d'accords avec renversements (1er, 2ème)
/// Les notes doivent correspondre au renversement spécifié et être dans une
/// octave
TEST_CASE("AnswerValidator::validerAccordRenversement") {
    AnswerValidator vn;
    std::vector<std::string> attendu = {"c4", "e4",
                                        "g4"}; // Accord Do Majeur fondamental

    /// Vérifie position fondamentale (renversement=1)
    SUBCASE("Root position (renversement=1)") {
        std::vector<std::string> joue = {"c4", "e4", "g4"};
        CHECK(vn.validerAccordRenversement(joue, attendu, 1));
    }

    /// Vérifie 1er renversement (renversement=2): mi-sol-do
    SUBCASE("1st inversion (renversement=2)") {
        std::vector<std::string> joue = {"e4", "g4", "c5"};
        CHECK(vn.validerAccordRenversement(joue, attendu, 2));
    }

    /// Vérifie 2ème renversement (renversement=3): sol-do-mi
    SUBCASE("2nd inversion (renversement=3)") {
        std::vector<std::string> joue = {"g4", "c5", "e5"};
        CHECK(vn.validerAccordRenversement(joue, attendu, 3));
    }

    /// Vérifie rejet renversement incorrect
    SUBCASE("Wrong inversion") {
        std::vector<std::string> joue = {"c4", "e4", "g4"};
        CHECK_FALSE(vn.validerAccordRenversement(joue, attendu, 2));
    }

    /// Vérifie rejet notes trop éloignées (hors octave)
    SUBCASE("Note too far apart (outside octave)") {
        std::vector<std::string> joue = {"c4", "e6", "g6"}; // Écart trop large
        CHECK_FALSE(vn.validerAccordRenversement(joue, attendu, 1));
    }
}
