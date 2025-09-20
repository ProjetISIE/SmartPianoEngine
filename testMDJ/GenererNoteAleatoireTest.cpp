#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "../include/GenererNoteAleatoire.h"
#include "../include/BaseAccords.h"

// Test de la generation d'une note aleatoire dans une gamme donnee
TEST_CASE("GenererNoteAleatoire - generation de note") {
    GenererNoteAleatoire generateur;

    SUBCASE("Generation de note dans une gamme majeure valide") {
        std::string note = generateur.generer("Do", "Majeur");
        CHECK_FALSE(note.empty()); // Verifie que la note n'est pas vide
    }

    SUBCASE("Generation de note dans une gamme mineure valide") {
        std::string note = generateur.generer("La", "Mineur");
        CHECK_FALSE(note.empty()); // Verifie que la note n'est pas vide
    }

    SUBCASE("Generation de note avec une gamme invalide") {
        std::string note = generateur.generer("XYZ", "Majeur");
        CHECK(note.empty()); // Verifie qu'on obtient une chaine vide pour gamme invalide
    }

    SUBCASE("Generation de note avec un mode invalide") {
        std::string note = generateur.generer("Do", "Inconnu");
        CHECK(note.empty()); // Verifie qu'on obtient une chaine vide pour mode invalide
    }
}

// Test de la generation d'un accord aleatoire
TEST_CASE("GenererNoteAleatoire - generation d'accord") {
    GenererNoteAleatoire generateur;

    SUBCASE("Generation d'un accord dans une gamme et mode valides") {
        auto [nomAccord, notes] = generateur.genererAccord("Do", "Majeur");
        CHECK_FALSE(nomAccord.empty()); // Verifie que le nom de l'accord n'est pas vide
        CHECK_FALSE(notes.empty()); // Verifie que l'accord contient bien des notes
        CHECK(notes.size() >= 3); // Verifie qu'il y a au moins 3 notes (accord standard)
    }

    SUBCASE("Generation d'un accord avec gamme invalide") {
        auto [nomAccord, notes] = generateur.genererAccord("XYZ", "Majeur");
        CHECK(nomAccord.empty()); // Verifie que l'accord est invalide
        CHECK(notes.empty()); // Verifie qu'il n'y a pas de notes
    }
}

// Test de la generation d'un accord avec renversement
TEST_CASE("GenererNoteAleatoire - generation d'accord avec renversement") {
    GenererNoteAleatoire generateur;

    SUBCASE("Generation d'un accord renverse dans une gamme et mode valides") {
        auto [nomAccord, notes, renversement] = generateur.genererAccordRenversement("Do", "Majeur");
        CHECK_FALSE(nomAccord.empty()); // Verifie que le nom de l'accord n'est pas vide
        CHECK_FALSE(notes.empty()); // Verifie que l'accord contient bien des notes
        CHECK(notes.size() >= 3); // Verifie qu'il y a au moins 3 notes
        CHECK(renversement >= 1);
        CHECK(renversement <= 3); // Verifie que le renversement est entre 1 et 3
    }

    SUBCASE("Generation d'un accord renverse avec gamme invalide") {
        auto [nomAccord, notes, renversement] = generateur.genererAccordRenversement("XYZ", "Majeur");
        CHECK(nomAccord.empty()); // Verifie que l'accord est invalide
        CHECK(notes.empty()); // Verifie qu'il n'y a pas de notes
        CHECK(renversement == 0); // Verifie que le renversement est invalide
    }
}
