#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ChordGame.hpp"
#include "Mocks.hpp"
#include <doctest/doctest.h>
#include <sstream>
#include <thread>

/// Vérifie le déroulement complet d'une partie en mode Accord
/// Valide envoi défis (3 notes simultanées), validation entrée MIDI multi-notes
TEST_CASE("ChordGame Flow") {
    // Vérifie le déroulement d'une partie de type "Accord"
    // S'assure que les accords sont correctement demandés et que la validation
    // MIDI (plusieurs notes simultanées) fonctionne
    MockTransport transport;
    MockMidiInput midi;
    GameConfig config;
    config.scale = "Do";
    config.mode = "Majeur";
    config.maxChallenges = 1;
    ChordGame game(transport, midi, config, false);

    transport.waitForClient(); // Simuler client connecté
    game.start();
    std::jthread gameThread([&](std::stop_token st) { game.play(st); });

    // Attente du défi (Accord)
    Message msg1 = transport.waitForSentMessage();
    CHECK(msg1.getType() == "chord");
    std::string notesStr = msg1.getField("notes");

    // Analyse des notes attendues (séparées par espaces)
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
}

/// Vérifie gestion réponses partielles (quelques notes correctes seulement)
/// Valide présence champ "correct" sans champ "incorrect" si notes
/// partiellement bonnes
TEST_CASE("ChordGame Partial and Incorrect") {
    MockTransport transport;
    MockMidiInput midi;
    GameConfig config;
    config.maxChallenges = 1;
    ChordGame game(transport, midi, config, false);

    transport.waitForClient();
    game.start();
    std::jthread gameThread([&](std::stop_token st) { game.play(st); });

    Message msg1 = transport.waitForSentMessage();
    std::string notesStr = msg1.getField("notes");
    std::vector<std::string> notes;
    std::stringstream ss(notesStr);
    std::string segment;
    while (std::getline(ss, segment, ' ')) notes.push_back(segment);

    // Envoyer seulement 1 note correcte (Partiel)
    if (!notes.empty()) midi.pushNotes({notes[0]});
    else midi.pushNotes(std::vector<Note>{});
    Message res1 = transport.waitForSentMessage();
    CHECK(res1.getType() == "result");

    // Vérifier champs dans message
    // Si correctNotes non vide -> champ "correct" existe
    // Si incorrectNotes non vide -> champ "incorrect" existe
    if (!notes.empty()) {
        CHECK(res1.hasField("correct"));
        // Si on joue seulement notes correctes (mais pas toutes), champ
        // "incorrect" n'est PAS présent
        CHECK_FALSE(res1.hasField("incorrect"));
    }
}

/// Vérifie génération et validation accords avec renversements
/// Valide apparition renversements (1er, 2ème) sur plusieurs défis
TEST_CASE("ChordGame Inversions") {
    MockTransport transport;
    MockMidiInput midi;
    GameConfig config;
    config.scale = "c";
    config.mode = "maj";
    config.maxChallenges = 20; // Augmenter pour s'assurer de voir renversements
    ChordGame game(transport, midi, config, true); // Avec renversements
    transport.waitForClient();
    game.start();
    std::jthread gameThread([&](std::stop_token st) { game.play(st); });

    bool seenInversion = false;
    for (int i = 0; i < 20; ++i) {
        Message msg = transport.waitForSentMessage();
        if (msg.getType() == "TIMEOUT") {
            FAIL("Timeout waiting for chord challenge at index " << i);
            break;
        }
        CHECK(msg.getType() == "chord");

        std::string name = msg.getField("name");
        if (name.find("1") != std::string::npos ||
            name.find("2") != std::string::npos) {
            seenInversion = true;
        }

        midi.pushNotes(std::vector<Note>{});
        transport.waitForSentMessage(); // result

        // Envoyer ready seulement si on attend un autre tour
        if (i < 19) transport.pushIncoming(Message("ready"));
    }
    CHECK(seenInversion); // Devrait être très probable
}

/// Vérifie comportement avec gamme inconnue (doit fallback Do Majeur)
/// Valide génération accord malgré paramètres invalides
TEST_CASE("ChordGame Unknown Scale") {
    MockTransport transport;
    MockMidiInput midi;
    GameConfig config;
    config.scale = "unknown";
    config.mode = "mode";
    config.maxChallenges = 1;
    ChordGame game(transport, midi, config, false);

    transport.waitForClient();
    game.start();
    std::jthread gameThread([&](std::stop_token st) { game.play(st); });

    Message msg = transport.waitForSentMessage();
    CHECK(msg.getType() == "chord"); // Devrait produire accord (Do Maj défaut)
    midi.pushNotes(std::vector<Note>{});
    transport.waitForSentMessage();
}

/// Vérifie gestion réponses complètement incorrectes (aucune note correcte)
/// Valide présence champ "incorrect" sans champ "correct"
TEST_CASE("ChordGame Completely Incorrect") {
    // Cas de test où aucune note n'est correcte (couverture ligne 96)
    MockTransport transport;
    MockMidiInput midi;
    GameConfig config;
    config.scale = "c";
    config.mode = "maj";
    config.maxChallenges = 1;
    ChordGame game(transport, midi, config, false);

    transport.waitForClient();
    game.start();
    std::jthread gameThread([&](std::stop_token st) { game.play(st); });

    Message msg = transport.waitForSentMessage();
    CHECK(msg.getType() == "chord");

    // Jouer notes complètement fausses
    midi.pushNotes({Note("c", 0), Note("d", 0)}); // Mauvaise octave/notes
    Message res = transport.waitForSentMessage();
    CHECK(res.getType() == "result");
    CHECK(res.hasField("incorrect"));
}

/// Vérifie gestion message non-ready entre défis (erreur protocole)
/// Valide robustesse face messages inattendus
TEST_CASE("ChordGame Ready Message Error") {
    // Test gestion message non-ready (lignes 103-106)
    MockTransport transport;
    MockMidiInput midi;
    GameConfig config;
    config.maxChallenges = 2;
    ChordGame game(transport, midi, config, false);

    transport.waitForClient();
    game.start();
    std::jthread gameThread([&](std::stop_token st) { game.play(st); });

    // Premier défi
    Message msg1 = transport.waitForSentMessage();
    CHECK(msg1.getType() == "chord");
    midi.pushNotes(std::vector<Note>{});
    transport.waitForSentMessage(); // result

    // Envoyer mauvais message au lieu de "ready"
    transport.pushIncoming(Message("wrong"));
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

/// Vérifie couverture renversements appliqués (inversion > 0)
/// S'assure que sur 50 tentatives au moins un renversement apparaît
TEST_CASE("ChordGame With Inversions Coverage") {
    // S'assurer inversion > 0 est appliquée (ligne 142)
    MockTransport transport;
    MockMidiInput midi;
    GameConfig config;
    config.scale = "c";
    config.mode = "maj";
    config.maxChallenges = 50; // Plus tentatives pour atteindre renversement
    ChordGame game(transport, midi, config, true);

    transport.waitForClient();
    game.start();

    bool foundInversion = false;
    std::jthread gameThread([&](std::stop_token st) { game.play(st); });

    for (int i = 0; i < 50; ++i) {
        Message msg = transport.waitForSentMessage();
        if (msg.getType() == "chord") {
            std::string name = msg.getField("name");
            if (name.find("1") != std::string::npos ||
                name.find("2") != std::string::npos) {
                foundInversion = true;
            }
        }

        midi.pushNotes(std::vector<Note>{});
        transport.waitForSentMessage(); // result
        if (i < 49) transport.pushIncoming(Message("ready"));
    }

    CHECK(foundInversion); // Très probable avec 50 tentatives
}

