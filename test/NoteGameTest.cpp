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
    std::thread gameThread([&game]() { game.play(); });
    Message msg1 = transport.waitForSentMessage();
    CHECK(msg1.getType() == "note");
    std::string expectedNote = msg1.getField("note");
    midi.pushNotes({expectedNote});
    Message res1 = transport.waitForSentMessage();
    CHECK(res1.getType() == "result");
    CHECK(res1.hasField("correct")); // Should be correct
    transport.pushIncoming(Message("ready"));
    Message msg2 = transport.waitForSentMessage();
    CHECK(msg2.getType() == "note");
    expectedNote = msg2.getField("note");
    midi.pushNotes({expectedNote});
    Message res2 = transport.waitForSentMessage();
    CHECK(res2.getType() == "result");
    CHECK(res2.hasField("correct"));
    if (gameThread.joinable()) gameThread.join();
}
