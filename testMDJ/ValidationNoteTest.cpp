#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "../include/ValidationNote.h"

// Test de la validation d'une seule note
TEST_CASE("ValidationNote - Validation d'une note") {
    ValidationNote validation;

    CHECK(validation.valider("C4", "C4") == true);   // Identique
    CHECK(validation.valider("D4", "C4") == false);  // Different
}

// Test de la validation d'un accord sans renversement
TEST_CASE("ValidationNote - Validation d'un accord sans renversement") {
    ValidationNote validation;

    SUBCASE("Accord correct") {
        std::vector<std::string> joue = {"C4", "E4", "G4"};
        std::vector<std::string> attendu = {"C4", "E4", "G4"};
        CHECK(validation.validerAccordSR(joue, attendu) == true);
    }

    SUBCASE("Accord correct dans le desordre") {
        std::vector<std::string> joue = {"E4", "G4", "C4"};
        std::vector<std::string> attendu = {"C4", "E4", "G4"};
        CHECK(validation.validerAccordSR(joue, attendu) == true);
    }

    SUBCASE("Accord incorrect") {
        std::vector<std::string> joue = {"C4", "E4", "A4"};
        std::vector<std::string> attendu = {"C4", "E4", "G4"};
        CHECK(validation.validerAccordSR(joue, attendu) == false);
    }

    SUBCASE("Accord de taille differente") {
        std::vector<std::string> joue = {"C4", "E4"};
        std::vector<std::string> attendu = {"C4", "E4", "G4"};
        CHECK(validation.validerAccordSR(joue, attendu) == false);
    }
}

// Test de la validation d'un accord avec renversement
TEST_CASE("ValidationNote - Validation d'un accord avec renversement") {
    ValidationNote validation;

    SUBCASE("Renversement correct") {
        std::vector<std::string> joue = {"E4", "G4", "C5"};
        std::vector<std::string> attendu = {"C4", "E4", "G4"};
        CHECK(validation.validerAccordRenversement(joue, attendu, 2) == true);
    }

    SUBCASE("Renversement incorrect") {
        std::vector<std::string> joue = {"C4", "E4", "G4"};
        std::vector<std::string> attendu = {"C4", "E4", "G4"};
        CHECK(validation.validerAccordRenversement(joue, attendu, 2) == false);
    }

    SUBCASE("Accord de taille differente") {
        std::vector<std::string> joue = {"C4", "E4"};
        std::vector<std::string> attendu = {"C4", "E4", "G4"};
        CHECK(validation.validerAccordRenversement(joue, attendu, 1) == false);
    }
}
