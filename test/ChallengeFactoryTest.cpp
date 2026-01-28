#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ChallengeFactory.hpp"
#include <doctest/doctest.h>
#include <regex>

/// Vérifie la génération aléatoire de défis musicaux (notes et accords)
/// Test génération notes, accords simples et avec renversements
TEST_CASE("ChallengeFactory") {
    ChallengeFactory gen;
    /// Vérifie génération note aléatoire dans gamme Do Majeur (sans
    /// altérations)
    SUBCASE("Generer note in Do Majeur") {
        std::string note = gen.generer("Do", "Majeur");
        std::regex pattern("^[A-G][#]?[0-9]$");
        CHECK(std::regex_match(note, pattern));
        CHECK(note.find('#') == std::string::npos);
        CHECK(note.find('b') == std::string::npos);
    }

    /// Vérifie génération note dans gamme Sol Majeur
    SUBCASE("Generer note in Sol Majeur") {
        std::string note;
        note = gen.generer("Sol", "Majeur");
        CHECK(!note.empty());
    }

    /// Vérifie génération accord (3 notes) avec nom et notes valides
    SUBCASE("Generer Accord Do Majeur") {
        auto [nom, notes] = gen.genererAccord("Do", "Majeur");
        CHECK(!nom.empty());
        CHECK(notes.size() == 3);
        for (const auto& n : notes) {
            std::regex pattern("^[A-G][#]?[0-9]$");
            CHECK(std::regex_match(n, pattern));
        }
    }

    /// Vérifie génération accord avec renversement aléatoire (1-3)
    SUBCASE("Generer Accord Renversement") {
        auto [nom, notes, renv] = gen.genererAccordRenversement("Do", "Majeur");
        CHECK(!nom.empty());
        CHECK(notes.size() == 3);
        CHECK(renv >= 1);
        CHECK(renv <= 3);
    }

    /// Vérifie comportement avec paramètres invalides (gamme inconnue)
    SUBCASE("Invalid parameters") {
        std::string note = gen.generer("Invalid", "Majeur");
        CHECK(note == "");
        auto [nom, notes] = gen.genererAccord("Invalid", "Majeur");
        CHECK(nom == "");
        CHECK(notes.empty());
    }
}
