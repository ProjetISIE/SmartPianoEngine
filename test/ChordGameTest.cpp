#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "ChordGame.hpp"
#include "Mocks.hpp"
#include <doctest/doctest.h>
#include <sstream>
#include <thread>

TEST_CASE("ChordGame Flow") {
    MockTransport transport;
    MockMidiInput midi;
    GameConfig config;
    config.scale = "Do";
    config.mode = "Majeur";
    config.maxChallenges = 1;

    ChordGame game(transport, midi, config, false);
    game.start();

    std::thread gameThread([&game]() { game.play(); });

    // Challenge 1
    Message msg1 = transport.waitForSentMessage();
    CHECK(msg1.type == "chord");
    std::string notesStr = msg1.getField("notes");

    // Parse notes (space separated)
    std::vector<std::string> notes;
    std::stringstream ss(notesStr);
    std::string segment;
    while (std::getline(ss, segment, ' ')) {
        notes.push_back(segment);
    }

    // Send correct notes
    midi.pushNotes(notes);

    // Result 1
    Message res1 = transport.waitForSentMessage();
    CHECK(res1.type == "result");
    CHECK(res1.hasField("correct"));

    if (gameThread.joinable()) gameThread.join();
}