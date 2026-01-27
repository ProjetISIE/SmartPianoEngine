#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "NoteGame.hpp"
#include "Mocks.hpp"
#include <doctest/doctest.h>
#include <thread>

TEST_CASE("NoteGame Flow") {
    MockTransport transport;
    MockMidiInput midi;
    GameConfig config;
    config.scale = "Do";
    config.mode = "Majeur";
    config.maxChallenges = 2; // Test with 2 challenges

    NoteGame game(transport, midi, config);
    game.start();

    // Run game in a separate thread because play() blocks
    std::thread gameThread([&game]() { game.play(); });

    // Challenge 1
    Message msg1 = transport.waitForSentMessage();
    CHECK(msg1.type == "note");
    std::string expectedNote = msg1.getField("note");

    // Send correct note
    midi.pushNotes({expectedNote});

    // Result 1
    Message res1 = transport.waitForSentMessage();
    CHECK(res1.type == "result");
    CHECK(res1.hasField("correct")); // Should be correct

    // Send "ready" for next challenge
    transport.pushIncoming(Message("ready"));

    // Challenge 2
    Message msg2 = transport.waitForSentMessage();
    CHECK(msg2.type == "note");
    expectedNote = msg2.getField("note");

    // Send correct note
    midi.pushNotes({expectedNote});

    // Result 2
    Message res2 = transport.waitForSentMessage();
    CHECK(res2.type == "result");
    CHECK(res2.hasField("correct"));

    // Game should finish now.
    // Wait for thread to join
    if (gameThread.joinable()) gameThread.join();
}