#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "GameEngine.hpp"
#include "Mocks.hpp"
#include <doctest/doctest.h>
#include <thread>

TEST_CASE("GameEngine workflow") {
    MockTransport transport;
    MockMidiInput midi;
    GameEngine engine(transport, midi);
    std::thread engineThread([&engine]() { engine.run(); });
    transport.waitForClient();
    Message configMsg("config",
                      {{"game", "note"}, {"scale", "c"}, {"mode", "maj"}});
    transport.pushIncoming(configMsg);
    // Wait for ACK
    Message ack = transport.waitForSentMessage();
    CHECK(ack.getType() == "ack");
    CHECK(ack.getField("status") == "ok");
    // Send READY to start game session
    transport.pushIncoming(Message("ready"));
    // Wait for Challenge 1
    Message challenge = transport.waitForSentMessage();
    CHECK(challenge.getType() == "note");
    // Stop engine
    engine.stop();
    if (engineThread.joinable()) engineThread.join();
}
