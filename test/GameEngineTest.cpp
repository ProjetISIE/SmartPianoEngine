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

    // 1. Engine waits for client connection
    transport.waitForClient(); // Wait for engine to call waitForClient?
    // No, mock waitForClient sets connected=true immediately.
    // Engine calls transport.start(), then waitForClient(), then receive().

    // We need to push a configuration message for the engine to consume.
    Message configMsg("config");
    configMsg.addField("game", "note");
    configMsg.addField("scale", "c");
    configMsg.addField("mode", "maj");
    transport.pushIncoming(configMsg);

    // Engine should receive config, send ACK, then start game.
    // The game (NoteGame) will send a "note" challenge.

    // Wait for ACK
    Message ack = transport.waitForSentMessage();
    CHECK(ack.type == "ack");
    CHECK(ack.getField("status") == "ok");

    // Send READY to start game session
    transport.pushIncoming(Message("ready"));

    // Wait for Challenge 1
    Message challenge = transport.waitForSentMessage();
    CHECK(challenge.type == "note");

    // Stop engine
    engine.stop();
    if (engineThread.joinable()) engineThread.join();
}