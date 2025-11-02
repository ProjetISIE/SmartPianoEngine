#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "BaseAccords.h"
#include <doctest/doctest.h>

TEST_CASE("BaseAccords - Validation des accords") {
    BaseAccords baseAccords;

    SUBCASE("Obtenir un accord valide") {
        std::vector<int> accord = baseAccords.obtenirAccord("Do Majeur", "C");
        CHECK(accord == std::vector<int>({60, 64, 67}));
    }

    SUBCASE("Obtenir un accord inexistant") {
        std::vector<int> accord = baseAccords.obtenirAccord("Do Majeur", "Z");
        CHECK(accord.empty());
    }

    SUBCASE("Obtenir un accord dans une tonalite incorrecte") {
        std::vector<int> accord = baseAccords.obtenirAccord("Z Majeur", "C");
        CHECK(accord.empty());
    }

    SUBCASE("Validation multiple des tonalites") {
        CHECK(baseAccords.obtenirAccord("Do Majeur", "F") ==
              std::vector<int>({65, 69, 72}));
        CHECK(baseAccords.obtenirAccord("Ré Majeur", "D") ==
              std::vector<int>({62, 66, 69}));
        CHECK(baseAccords.obtenirAccord("Mi Mineur", "Em") ==
              std::vector<int>({64, 67, 71}));
    }

    SUBCASE("Validation des accords pour une tonalite mineure") {
        CHECK(baseAccords.obtenirAccord("La Mineur", "Am") ==
              std::vector<int>({69, 72, 76}));
        CHECK(baseAccords.obtenirAccord("Si Mineur", "Bm") ==
              std::vector<int>({71, 74, 78}));
    }

    SUBCASE("Validation des accords pour des tonalites majeures") {
        CHECK(baseAccords.obtenirAccord("Fa Majeur", "Bb") ==
              std::vector<int>({70, 74, 77}));
        CHECK(baseAccords.obtenirAccord("Sol Majeur", "G") ==
              std::vector<int>({67, 71, 74}));
    }
}
