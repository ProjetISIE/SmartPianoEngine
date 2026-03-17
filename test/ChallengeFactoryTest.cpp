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
        // On vérifie qu'il n'y a pas d'altération (# ou b en 2ème position)
        if (note.size() > 2) { // Cas avec altération ex: c#4
             CHECK(note[1] != '#');
             CHECK(note[1] != 'b');
        } else {
             // Si taille 2, format est "c4", donc pas d'altération
             CHECK(note.size() == 2);
        }
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

    /// Vérifie que les accords ne dépassent pas une octave (12 demi-tons)
    SUBCASE("Chord Span") {
        auto noteToValue = [](const std::string& n) {
            static const std::map<char, int> baseValues = {
                {'c', 0}, {'d', 2}, {'e', 4}, {'f', 5},
                {'g', 7}, {'a', 9}, {'b', 11}};
            int val = baseValues.at(n[0]);
            size_t i = 1;
            if (i < n.size() && (n[i] == '#' || n[i] == 'b')) {
                if (n[i] == '#') val += 1;
                else val -= 1;
                i++;
            }
            int octave = n[i] - '0';
            return val + (octave + 1) * 12;
        };

        for (int i = 0; i < 100; ++i) {
            auto [nom, notes, renv] = gen.generateInversedChord("c", "maj");
            int minVal = 1000, maxVal = -1000;
            for (const auto& n : notes) {
                int val = noteToValue(n);
                if (val < minVal) minVal = val;
                if (val > maxVal) maxVal = val;
            }
            CHECK(maxVal - minVal <= 12);
        }
    }

    /// Vérifie qu'on n'a pas deux fois le même accord de suite (racine +
    /// octave)
    SUBCASE("Chord Entropy") {
        std::string lastFull;
        for (int i = 0; i < 50; ++i) {
            auto [nom, notes] = gen.generateChord("c", "maj");
            std::string currentFull = nom;
            for (const auto& n : notes) currentFull += n;

            if (!lastFull.empty()) {
                CHECK(currentFull != lastFull);
            }
            lastFull = currentFull;
        }
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
