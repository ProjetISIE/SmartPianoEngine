#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "NoteGame.hpp"
#include "Mocks.hpp"
#include <doctest/doctest.h>
#include <thread>

/// Vérifie le déroulement complet d'une partie en mode Note
/// Valide envoi défis, validation réponses correctes et enchaînement défis
TEST_CASE("NoteGame Flow") {
    // Vérifie le déroulement d'une partie de type "Note"
    // Le test valide que le jeu envoie les défis (notes) et valide correctement
    // les entrées MIDI simulées
    MockTransport transport;
    MockMidiInput midi;
    GameConfig config;
    config.scale = "Do";
    config.mode = "Majeur";
    config.maxChallenges = 2; // Test avec 2 défis pour vérifier l'enchaînement
    NoteGame game(transport, midi, config);

    game.start();
    std::thread gameThread([&game]() { game.play(); });

    // Premier défi
    Message msg1 = transport.waitForSentMessage();
    CHECK(msg1.getType() == "note");
    std::string expectedNote = msg1.getField("note");

    // Simulation de la réponse correcte via MIDI
    midi.pushNotes({expectedNote});

    // Vérification du résultat (attendu correct)
    Message res1 = transport.waitForSentMessage();
    CHECK(res1.getType() == "result");
    CHECK(res1.hasField("correct"));

    // Signal pour passer au défi suivant
    transport.pushIncoming(Message("ready"));

    // Deuxième défi
    Message msg2 = transport.waitForSentMessage();
    CHECK(msg2.getType() == "note");
    expectedNote = msg2.getField("note");
    midi.pushNotes({expectedNote});

    Message res2 = transport.waitForSentMessage();
    CHECK(res2.getType() == "result");
    CHECK(res2.hasField("correct"));

    if (gameThread.joinable()) gameThread.join();
}

/// Vérifie détection et signalement réponse incorrecte
/// Valide présence champ "incorrect" dans résultat
TEST_CASE("NoteGame Incorrect Answer") {
    MockTransport transport;
    MockMidiInput midi;
    GameConfig config;
    config.scale = "c";
    config.mode = "maj";
    config.maxChallenges = 1;
    NoteGame game(transport, midi, config);

    game.start();
    std::thread gameThread([&game]() { game.play(); });

    Message msg1 = transport.waitForSentMessage();
    std::string expectedNote = msg1.getField("note");

    // Envoyer note incorrecte
    std::string wrongNote = (expectedNote == "c4") ? "d4" : "c4";
    midi.pushNotes({wrongNote});

    Message res1 = transport.waitForSentMessage();
    CHECK(res1.getType() == "result");
    CHECK(res1.hasField("incorrect"));
    CHECK_FALSE(res1.hasField("correct"));

    if (gameThread.joinable()) gameThread.join();
}

/// Vérifie comportement repli gamme invalide (doit utiliser Do Majeur par
/// défaut) Valide que défis sont quand même générés avec gamme fallback
TEST_CASE("NoteGame Unknown Scale Fallback") {
    MockTransport transport;
    MockMidiInput midi;
    GameConfig config;
    config.scale = "unknown";
    config.mode = "mode";
    config.maxChallenges = 1;
    NoteGame game(transport, midi, config);

    game.start();
    std::thread gameThread([&game]() { game.play(); });

    // Devrait fallback sur Do Majeur, donc générer une note
    Message msg1 = transport.waitForSentMessage();
    CHECK(msg1.getType() == "note");

    midi.close(); // Arrêter attente notes
    if (gameThread.joinable()) gameThread.join();
}

/// Vérifie détection plusieurs notes incorrectes simultanées
/// Valide que champ "incorrect" contient toutes notes fausses avec espaces
TEST_CASE("NoteGame Multiple Incorrect Notes") {
    MockTransport transport;
    MockMidiInput midi;
    GameConfig config;
    config.scale = "c";
    config.mode = "maj";
    config.maxChallenges = 1;
    NoteGame game(transport, midi, config);

    game.start();
    std::thread gameThread([&game]() { game.play(); });

    Message msg1 = transport.waitForSentMessage();
    std::string expectedNote = msg1.getField("note");

    // Envoyer plusieurs notes incorrectes (couvre ligne 60 branch)
    std::vector<std::string> wrongNotes;
    if (expectedNote != "c4") wrongNotes.push_back("c4");
    if (expectedNote != "d4") wrongNotes.push_back("d4");
    if (expectedNote != "e4") wrongNotes.push_back("e4");
    midi.pushNotes(wrongNotes);

    Message res1 = transport.waitForSentMessage();
    CHECK(res1.getType() == "result");
    CHECK(res1.hasField("incorrect"));
    // Vérifier que champ incorrect contient espaces (ligne 60)
    std::string incorrect = res1.getField("incorrect");
    CHECK(incorrect.find(" ") != std::string::npos);

    if (gameThread.joinable()) gameThread.join();
}
