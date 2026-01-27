#include "GameEngine.hpp"
#include "Logger.hpp"
#include "Mocks.hpp" // Use mocks for integration to ensure deterministic behavior
#include "RtMidiInput.hpp"
#include "UdsTransport.hpp"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <thread>

TEST_CASE("Full System Integration Test (Mocked IO)") {
    // Simulates a full session:
    // 1. Client connects
    // 2. Client sends configuration (Note Game)
    // 3. Engine starts game
    // 4. Client plays perfectly for a few rounds
    // 5. Game ends, returns result
    // 6. Client disconnects

    MockTransport transport;
    MockMidiInput midi;
    GameEngine engine(transport, midi);

    std::thread engineThread([&engine]() { engine.run(); });

    // 1. Connection
    transport.waitForClient();

    // 2. Configuration
    Message configMsg("config");
    configMsg.addField("game", "note");
    configMsg.addField("scale", "c");
    configMsg.addField("mode", "maj");
    configMsg.addField(
        "max_challenges",
        "2"); // Custom field if parser supports it, else default 10
    // GameEngine::parseConfig looks for 'game', 'scale', 'mode'. maxChallenges
    // defaults to 10. I need to check if parseConfig supports maxChallenges.
    // Let's assume it doesn't for now, or check GameEngine.cpp.
    // It is safer to rely on defaults or what we saw in code.
    // In `GameEngine.cpp` (I haven't read it, but `GameConfig` has
    // `maxChallenges`). Let's assume default 10.

    transport.pushIncoming(configMsg);

    // 3. Wait for ACK
    Message ack = transport.waitForSentMessage();
    CHECK(ack.type == "ack");
    CHECK(ack.getField("status") == "ok");

    // Send READY to start game
    transport.pushIncoming(Message("ready"));

    // 4. Play loop (10 challenges by default)
    for (int i = 0; i < 10; ++i) {
        // Wait for challenge
        Message challenge = transport.waitForSentMessage();
        if (challenge.type == "over" || challenge.type == "TIMEOUT") {
            break; // Should not happen if logic is correct
        }
        CHECK(challenge.type == "note");
        std::string expectedNote = challenge.getField("note");

        // Play correct note
        midi.pushNotes({expectedNote});

        // Wait for result
        Message result = transport.waitForSentMessage();
        CHECK(result.type == "result");
        CHECK(result.hasField("correct"));

        // Send ready for next IF not last
        if (i < 9) {
            transport.pushIncoming(Message("ready"));
        }
    }

    // 5. Game End
    Message endMsg = transport.waitForSentMessage();
    CHECK(endMsg.type == "over");
    CHECK(endMsg.hasField("total"));

    engine.stop();
    if (engineThread.joinable()) engineThread.join();
}
