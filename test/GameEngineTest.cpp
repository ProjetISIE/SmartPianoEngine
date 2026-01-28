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
