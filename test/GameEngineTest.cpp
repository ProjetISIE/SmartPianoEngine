#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "GameEngine.hpp"
#include "Mocks.hpp"
#include <doctest/doctest.h>
#include <thread>

TEST_CASE("GameEngine workflow") {
    // Teste le flux complet du moteur de jeu : configuration, démarrage,
    // interaction, et arrêt. Ce test simule le cycle de vie typique d'une
    // session pour s'assurer de la cohérence des états.
    MockTransport transport;
    MockMidiInput midi;
    GameEngine engine(transport, midi);

    // Démarrage du moteur dans un thread séparé
    std::thread engineThread([&engine]() { engine.run(); });
    transport.waitForClient();

    // Envoi du message de configuration (jeu de notes, Do Majeur)
    Message configMsg("config",
                      {{"game", "note"}, {"scale", "c"}, {"mode", "maj"}});
    transport.pushIncoming(configMsg);

    // Attente de l'acquittement (ACK)
    Message ack = transport.waitForSentMessage();
    CHECK(ack.getType() == "ack");
    CHECK(ack.getField("status") == "ok");

    // Envoi du signal "ready" pour lancer la partie
    transport.pushIncoming(Message("ready"));

    // Vérification de la réception du premier défi (note)
    Message challenge = transport.waitForSentMessage();
    CHECK(challenge.getType() == "note");

    // Arrêt propre du moteur
    engine.stop();
    if (engineThread.joinable()) engineThread.join();
}

TEST_CASE("GameEngine Error Handling") {
    MockTransport transport;
    MockMidiInput midi;
    GameEngine engine(transport, midi);

    std::thread engineThread([&engine]() { engine.run(); });
    transport.waitForClient();

    SUBCASE("Invalid Config (Missing Game Type)") {
        Message configMsg("config", {{"scale", "c"}, {"mode", "maj"}});
        transport.pushIncoming(configMsg);

        Message ack = transport.waitForSentMessage();
        CHECK(ack.getType() == "ack");
        CHECK(ack.getField("status") == "error");
        CHECK(ack.getField("code") == "game");
    }

    SUBCASE("Unknown Game Mode") {
        Message configMsg(
            "config", {{"game", "unknown"}, {"scale", "c"}, {"mode", "maj"}});
        transport.pushIncoming(configMsg);

        Message ack = transport.waitForSentMessage();
        CHECK(ack.getType() == "ack");
        CHECK(ack.getField("status") == "ok"); // Acks the config reception

        // But then processing fails
        Message error = transport.waitForSentMessage();
        CHECK(error.getType() == "error");
        CHECK(error.getField("code") == "internal");
    }

    SUBCASE("Unexpected Message before Config") {
        transport.pushIncoming(Message("unexpected"));
        Message error = transport.waitForSentMessage();
        CHECK(error.getType() == "error");
        CHECK(error.getField("code") == "state");
    }

    SUBCASE("MIDI Not Ready") {
        midi.setReady(false);
        midi.setInitializeResult(false);

        Message configMsg("config",
                          {{"game", "note"}, {"scale", "c"}, {"mode", "maj"}});
        transport.pushIncoming(configMsg);

        Message error = transport.waitForSentMessage();
        CHECK(error.getType() == "error");
        CHECK(error.getField("code") == "midi");
    }

    SUBCASE("Client Quit during Config") {
        transport.pushIncoming(Message("quit"));
        // Should log and continue waiting. We can verify it's still running by
        // sending valid config after.
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        Message configMsg("config",
                          {{"game", "note"}, {"scale", "c"}, {"mode", "maj"}});
        transport.pushIncoming(configMsg);
        Message ack = transport.waitForSentMessage();
        CHECK(ack.getType() == "ack");
        CHECK(ack.getField("status") == "ok");
    }

    engine.stop();
    if (engineThread.joinable()) engineThread.join();
}

TEST_CASE("GameEngine Game Modes") {
    MockTransport transport;
    MockMidiInput midi;
    GameEngine engine(transport, midi);
    std::thread engineThread([&engine]() { engine.run(); });
    transport.waitForClient();

    SUBCASE("Chord Game") {
        Message configMsg("config",
                          {{"game", "chord"}, {"scale", "c"}, {"mode", "maj"}});
        transport.pushIncoming(configMsg);
        transport.waitForSentMessage(); // Ack
        transport.pushIncoming(Message("ready"));
        Message challenge = transport.waitForSentMessage();
        CHECK(challenge.getType() == "chord");
    }

    SUBCASE("Inversed Game") {
        Message configMsg(
            "config", {{"game", "inversed"}, {"scale", "c"}, {"mode", "maj"}});
        transport.pushIncoming(configMsg);
        transport.waitForSentMessage(); // Ack
        transport.pushIncoming(Message("ready"));
        Message challenge = transport.waitForSentMessage();
        CHECK(challenge.getType() == "chord"); // Inversed is also a chord game
    }

    engine.stop();
    if (engineThread.joinable()) engineThread.join();
}
