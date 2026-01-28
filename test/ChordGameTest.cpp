#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ChordGame.hpp"
#include "Mocks.hpp"
#include <doctest/doctest.h>
#include <sstream>
#include <thread>

TEST_CASE("ChordGame Flow") {
    // Vérifie le déroulement d'une partie de type "Accord".
    // S'assure que les accords sont correctement demandés et que la validation
    // MIDI (plusieurs notes simultanées) fonctionne.
    MockTransport transport;
    MockMidiInput midi;
    GameConfig config;
    config.scale = "Do";
    config.mode = "Majeur";
    config.maxChallenges = 1;
    ChordGame game(transport, midi, config, false);

    transport.waitForClient(); // Simulate connected client
    game.start();
    std::thread gameThread([&game]() { game.play(); });

    // Attente du défi (Accord)
    Message msg1 = transport.waitForSentMessage();
    CHECK(msg1.getType() == "chord");
    std::string notesStr = msg1.getField("notes");

    // Analyse des notes attendues (séparées par des espaces)
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

    if (gameThread.joinable()) gameThread.join();
}

TEST_CASE("ChordGame Partial and Incorrect") {
    MockTransport transport;
    MockMidiInput midi;
    GameConfig config;
    config.maxChallenges = 1;
    ChordGame game(transport, midi, config, false);

    transport.waitForClient();
    game.start();
    std::thread gameThread([&game]() { game.play(); });

    Message msg1 = transport.waitForSentMessage();
    std::string notesStr = msg1.getField("notes");
    std::vector<std::string> notes;
    std::stringstream ss(notesStr);
    std::string segment;
    while (std::getline(ss, segment, ' ')) notes.push_back(segment);

    // Send only 1 correct note (Partial)
    if (!notes.empty()) {
        midi.pushNotes({notes[0]});
    } else {
        midi.pushNotes(std::vector<Note>{});
    }

    Message res1 = transport.waitForSentMessage();
    CHECK(res1.getType() == "result");

    // Check fields in message
    // If correctNotes not empty -> field "correct" exists.
    // If incorrectNotes not empty -> field "incorrect" exists.

    if (!notes.empty()) {
        CHECK(res1.hasField("correct"));
        // If we only played correct notes (but not all), "incorrect" field is
        // NOT present.
        CHECK_FALSE(res1.hasField("incorrect"));
    }

    if (gameThread.joinable()) gameThread.join();
}

TEST_CASE("ChordGame Inversions") {
    MockTransport transport;
    MockMidiInput midi;
    GameConfig config;
    config.scale = "c";
    config.mode = "maj";
    config.maxChallenges = 20; // Increase to ensure we hit inversions
    ChordGame game(transport, midi, config, true); // With inversions

    transport.waitForClient();
    game.start();
    std::thread gameThread([&game]() { game.play(); });

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

        // Send ready only if we expect another round
        if (i < 19) {
            transport.pushIncoming(Message("ready"));
        }
    }

    CHECK(seenInversion); // Should be very likely

    if (gameThread.joinable()) gameThread.join();
}

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
    std::thread gameThread([&game]() { game.play(); });

    Message msg = transport.waitForSentMessage();
    CHECK(msg.getType() == "chord"); // Should produce a chord (C Maj default)

    midi.pushNotes(std::vector<Note>{});
    transport.waitForSentMessage();

    if (gameThread.joinable()) gameThread.join();
}

TEST_CASE("ChordGame Completely Incorrect") {
    // Test case where no notes are correct (line 96 coverage)
    MockTransport transport;
    MockMidiInput midi;
    GameConfig config;
    config.scale = "c";
    config.mode = "maj";
    config.maxChallenges = 1;
    ChordGame game(transport, midi, config, false);

    transport.waitForClient();
    game.start();
    std::thread gameThread([&game]() { game.play(); });

    Message msg = transport.waitForSentMessage();
    CHECK(msg.getType() == "chord");
    
    // Play completely wrong notes
    midi.pushNotes({Note("c", 0), Note("d", 0)}); // Wrong octave/notes

    Message res = transport.waitForSentMessage();
    CHECK(res.getType() == "result");
    CHECK(res.hasField("incorrect"));

    if (gameThread.joinable()) gameThread.join();
}

TEST_CASE("ChordGame Ready Message Error") {
    // Test handling of non-ready message (lines 103-106)
    MockTransport transport;
    MockMidiInput midi;
    GameConfig config;
    config.maxChallenges = 2;
    ChordGame game(transport, midi, config, false);

    transport.waitForClient();
    game.start();
    std::thread gameThread([&game]() { game.play(); });

    // First challenge
    Message msg1 = transport.waitForSentMessage();
    CHECK(msg1.getType() == "chord");
    midi.pushNotes(std::vector<Note>{});
    transport.waitForSentMessage(); // result

    // Send wrong message instead of "ready"
    transport.pushIncoming(Message("wrong"));
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    if (gameThread.joinable()) gameThread.join();
}

TEST_CASE("ChordGame With Inversions Coverage") {
    // Ensure inversion > 0 is applied (line 142)
    MockTransport transport;
    MockMidiInput midi;
    GameConfig config;
    config.scale = "c";
    config.mode = "maj";
    config.maxChallenges = 50; // More attempts to hit inversion
    ChordGame game(transport, midi, config, true);

    transport.waitForClient();
    game.start();
    
    bool foundInversion = false;
    std::thread gameThread([&game]() { game.play(); });

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
        
        if (i < 49) {
            transport.pushIncoming(Message("ready"));
        }
    }

    CHECK(foundInversion); // Very likely with 50 attempts
    
    if (gameThread.joinable()) gameThread.join();
}
