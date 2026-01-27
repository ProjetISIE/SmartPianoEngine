#include "GameEngine.hpp"
#include "Mocks.hpp" // Remplace le client par quelque chose déterministe
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <thread>

TEST_CASE("Full System Integration Test (Mocked IO)") {
    MockTransport transport;
    MockMidiInput midi;
    GameEngine engine(transport, midi);
    std::thread engineThread([&engine]() { engine.run(); });
    transport.waitForClient();
    Message configMsg("config", {{"game", "note"},
                                 {"scale", "c"},
                                 {"mode", "maj"},
                                 {"max_challenges", "2"}});
    transport.pushIncoming(configMsg);
    Message ack = transport.waitForSentMessage();
    CHECK(ack.getType() == "ack");
    CHECK(ack.getField("status") == "ok");
    transport.pushIncoming(Message("ready"));
    for (int i = 0; i < 10; ++i) {
        Message challenge = transport.waitForSentMessage();
        if (challenge.getType() == "over" || challenge.getType() == "TIMEOUT")
            break;
        CHECK(challenge.getType() == "note");
        std::string expectedNote = challenge.getField("note");
        midi.pushNotes({expectedNote});
        Message result = transport.waitForSentMessage();
        CHECK(result.getType() == "result");
        CHECK(result.hasField("correct"));
        if (i < 9) transport.pushIncoming(Message("ready"));
    }
    Message endMsg = transport.waitForSentMessage();
    CHECK(endMsg.getType() == "over");
    CHECK(endMsg.hasField("total"));
    engine.stop();
    if (engineThread.joinable()) engineThread.join();
}
