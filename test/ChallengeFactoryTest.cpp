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
    SUBCASE("Generate note in Do Majeur") {
        std::string note = gen.generateNote("c", "maj");
        std::regex pattern("^[a-g][#b]?[0-9]$");
        CHECK(std::regex_match(note, pattern));
        CHECK(note.find('#') == std::string::npos);
        CHECK(note.find('b') == std::string::npos);
    }

    /// Vérifie génération note dans gamme Sol Majeur
    SUBCASE("Generate note in Sol Majeur") {
        std::string note = gen.generateNote("g", "maj");
        CHECK(!note.empty());
    }

    /// Vérifie génération accord (3 notes) avec nom et notes valides
    SUBCASE("Generate Accord Do Majeur") {
        auto [nom, notes] = gen.generateChord("c", "maj");
        CHECK(!nom.empty());
        CHECK(notes.size() == 3);
        for (const auto& n : notes) {
            std::regex pattern("^[a-g][#b]?[0-9]$");
            CHECK(std::regex_match(n, pattern));
        }
    }

    /// Vérifie génération accord avec renversement aléatoire (1-3)
    SUBCASE("Generate Accord Inversed") {
        auto [nom, notes, renv] = gen.generateInversedChord("c", "maj");
        CHECK(!nom.empty());
        CHECK(notes.size() == 3);
        CHECK(renv >= 1);
        CHECK(renv <= 3);
    }

    /// Vérifie comportement avec paramètres invalides (gamme inconnue)
    SUBCASE("Invalid parameters fallback") {
        // Devrait fallback sur Do Majeur selon implémentation (au lieu de
        // chaine vide)
        std::string note = gen.generateNote("invalid", "maj");
        CHECK(!note.empty());
        auto [nom, notes] = gen.generateChord("invalid", "maj");
        CHECK(!nom.empty());
        CHECK(notes.size() == 3);
    }
}
