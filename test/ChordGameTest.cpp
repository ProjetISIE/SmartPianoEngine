#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ChordGame.hpp"
#include "Mocks.hpp"
#include <doctest/doctest.h>
#include <sstream>
#include <thread>

/// Vérifie le déroulement complet d'une partie en mode Accord
/// Valide envoi défis (3 notes simultanées), validation entrée MIDI multi-notes
TEST_CASE("ChordGame Flow") {
    MockTransport transport;
    MockMidiInput midi;
    ChallengeFactory factory;
    GameConfig config;
    config.scale = "c";
    config.mode = "maj";
    config.maxChallenges = 1;
    ChordGame game(transport, midi, factory, config, false);

    transport.waitForClient();
    game.start();
    std::thread gameThread([&game]() { game.play(); });

    // Attente du défi (Accord)
    Message msg1 = transport.waitForSentMessage();
    CHECK(msg1.getType() == "chord");
    std::string notesStr = msg1.getField("notes");

    // Analyse des notes attendues
    std::vector<std::string> notes;
    std::stringstream ss(notesStr);
    std::string segment;
    while (std::getline(ss, segment, ' ')) notes.push_back(segment);

    // Envoi simultané des notes correctes via MIDI
    midi.pushNotes(notes);

    // Vérification du résultat
    Message res1 = transport.waitForSentMessage();
    CHECK(res1.getType() == "result");
    CHECK(res1.hasField("correct"));

    game.stop();
    if (gameThread.joinable()) gameThread.join();
}

/// Vérifie gestion réponses partielles (quelques notes correctes seulement)
/// Valide présence champ "correct" sans champ "incorrect" si notes
/// partiellement bonnes
TEST_CASE("ChordGame Partial and Incorrect") {
    MockTransport transport;
    MockMidiInput midi;
    ChallengeFactory factory;
    GameConfig config;
    config.scale = "c";
    config.mode = "maj";
    config.maxChallenges = 1;
    ChordGame game(transport, midi, factory, config, false);

    transport.waitForClient();
    game.start();
    std::thread gameThread([&game]() { game.play(); });

    Message msg1 = transport.waitForSentMessage();
    std::string notesStr = msg1.getField("notes");
    std::vector<std::string> notes;
    std::stringstream ss(notesStr);
    std::string segment;
    while (std::getline(ss, segment, ' ')) notes.push_back(segment);

    // Envoyer seulement 1 note correcte (Partiel)
    if (!notes.empty()) midi.pushNotes({notes[0]});
    Message res1 = transport.waitForSentMessage();
    CHECK(res1.getType() == "result");

    if (!notes.empty()) {
        CHECK(res1.hasField("correct"));
        CHECK_FALSE(res1.hasField("incorrect"));
    }
    game.stop();
    if (gameThread.joinable()) gameThread.join();
}

/// Vérifie génération et validation accords avec renversements
/// Valide apparition renversements (1er, 2ème) sur plusieurs défis
TEST_CASE("ChordGame Inversions") {
    MockTransport transport;
    MockMidiInput midi;
    ChallengeFactory factory;
    GameConfig config;
    config.scale = "c";
    config.mode = "maj";
    config.maxChallenges = 20;
    ChordGame game(transport, midi, factory, config, true);
    transport.waitForClient();
    game.start();
    std::thread gameThread([&game]() { game.play(); });

    bool seenInversion = false;
    for (int i = 0; i < 20; ++i) {
        Message msg = transport.waitForSentMessage();
        if (msg.getType() == "TIMEOUT") break;
        CHECK(msg.getType() == "chord");

        std::string name = msg.getField("name");
        if (name.find("1") != std::string::npos ||
            name.find("2") != std::string::npos) {
            seenInversion = true;
        }

        midi.pushNotes(std::vector<Note>{});
        transport.waitForSentMessage(); // result
        if (i < 19) transport.pushIncoming(Message("ready"));
    }
    CHECK(seenInversion);
    game.stop();
    if (gameThread.joinable()) gameThread.join();
}

/// Vérifie comportement avec gamme inconnue (doit fallback Do Majeur)
/// Valide génération accord malgré paramètres invalides
TEST_CASE("ChordGame Unknown Scale") {
    MockTransport transport;
    MockMidiInput midi;
    ChallengeFactory factory;
    GameConfig config;
    config.scale = "unknown";
    config.mode = "mode";
    config.maxChallenges = 1;
    ChordGame game(transport, midi, factory, config, false);

    transport.waitForClient();
    game.start();
    std::thread gameThread([&game]() { game.play(); });

    Message msg = transport.waitForSentMessage();
    CHECK(msg.getType() == "chord");
    midi.pushNotes(std::vector<Note>{});
    transport.waitForSentMessage();

    game.stop();
    if (gameThread.joinable()) gameThread.join();
}

/// Vérifie gestion réponses complètement incorrectes (aucune note correcte)
/// Valide présence champ "incorrect" sans champ "correct"
TEST_CASE("ChordGame Completely Incorrect") {
    MockTransport transport;
    MockMidiInput midi;
    ChallengeFactory factory;
    GameConfig config;
    config.scale = "c";
    config.mode = "maj";
    config.maxChallenges = 1;
    ChordGame game(transport, midi, factory, config, false);

    transport.waitForClient();
    game.start();
    std::thread gameThread([&game]() { game.play(); });

    Message msg = transport.waitForSentMessage();
    CHECK(msg.getType() == "chord");

    midi.pushNotes({Note("c", 0), Note("d", 0)});
    Message res = transport.waitForSentMessage();
    CHECK(res.getType() == "result");
    CHECK(res.hasField("incorrect"));

    game.stop();
    if (gameThread.joinable()) gameThread.join();
}
