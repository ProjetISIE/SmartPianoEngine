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
