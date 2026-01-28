#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ChordRepository.hpp"
#include <doctest/doctest.h>

/// Vérifie l'initialisation et récupération des accords depuis le référentiel
/// Test accords valides (Do Majeur I) et gestion erreurs (tonalité/degré
/// invalide)
TEST_CASE("ChordRepository initialization and retrieval") {
    ChordRepository ba;
    /// Vérifie obtention accord Do Majeur I (contient bien un Do/C)
    SUBCASE("Obtenir Accord Do I") {
        auto accord = ba.obtenirAccord("Do Majeur", "C");
        // Do Majeur I est C E G -> MIDI 60, 64, 67 (typiquement)
        CHECK_FALSE(accord.empty());
        if (!accord.empty()) {
            bool hasC = false;
            for (int note : accord)
                if (note % 12 == 0)
                    hasC = true; // 0 est Do en MIDI (0, 12, 24, 36, 48, 60...)
            CHECK(hasC);
        }
    }
    /// Vérifie retour vide pour degré invalide (tonalité existe mais pas degré
    /// Z)
    SUBCASE("Obtenir Accord invalide (Tonality exists, Degree missing)") {
        auto accord = ba.obtenirAccord("Do Majeur", "Z"); // Z n'existe pas
        CHECK(accord.empty());
    }
    /// Vérifie retour vide pour tonalité inexistante
    SUBCASE("Obtenir Accord invalide (Tonality missing)") {
        auto accord = ba.obtenirAccord("NonExistent", "I");
        CHECK(accord.empty());
    }
}
