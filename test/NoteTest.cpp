#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "Note.hpp"
#include <doctest/doctest.h>

/// Vérifie la construction et l'analyse des notes musicales
/// Test le constructeur par défaut, paramétré et par chaîne
TEST_CASE("Note construction and parsing") {
    /// Vérifie que le constructeur par défaut crée un Do4 (c4)
    SUBCASE("Default constructor") {
        Note n;
        CHECK(n.getName() == "c");
        CHECK(n.getOctave() == 4);
        CHECK(n.toString() == "c4");
    }
    /// Vérifie la création d'une note avec nom et octave explicites
    SUBCASE("Parameterized constructor") {
        Note n("d#", 5);
        CHECK(n.getName() == "d#");
        CHECK(n.getOctave() == 5);
        CHECK(n.toString() == "d#5");
    }
    /// Vérifie l'analyse correcte de chaînes (gb3, f2) vers des objets Note
    SUBCASE("String parsing constructor") {
        Note n1("gb3");
        CHECK(n1.getName() == "gb");
        CHECK(n1.getOctave() == 3);

        Note n2("f2");
        CHECK(n2.getName() == "f");
        CHECK(n2.getOctave() == 2);
    }
}

/// Vérifie que les constructions invalides lèvent des exceptions appropriées
/// Test les octaves hors limites et formats de chaînes malformés
TEST_CASE("Note invalid construction") {
    /// Vérifie le rejet des octaves invalides (< 0 ou > 8)
    SUBCASE("Invalid octave in constructor") {
        CHECK_THROWS_AS(Note("c", 9), std::invalid_argument);
        CHECK_THROWS_AS(Note("c", -1), std::invalid_argument);
    }
    /// Vérifie le rejet des formats invalides (nom invalide, octave manquante,
    /// etc.)
    SUBCASE("Invalid string format") {
        CHECK_THROWS_AS(Note(""), std::invalid_argument);
        CHECK_THROWS_AS(Note("h4"), std::invalid_argument); // invalid note name
        CHECK_THROWS_AS(Note("c9"), std::invalid_argument); // invalid octave
        CHECK_THROWS_AS(Note("c#"), std::invalid_argument); // missing octave
        CHECK_THROWS_AS(Note("cc4"), std::invalid_argument); // too many chars
    }
}

/// Vérifie les opérateurs d'égalité et d'inégalité entre notes
/// Deux notes identiques doivent être égales quel que soit le constructeur
/// utilisé
TEST_CASE("Note equality") {
    Note n1("c4");
    Note n2("c", 4);
    Note n3("d4");
    CHECK(n1 == n2);
    CHECK(n1 != n3);
}
