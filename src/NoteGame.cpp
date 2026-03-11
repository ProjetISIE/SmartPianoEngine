#include "NoteGame.hpp"
#include "Logger.hpp"
#include <chrono>

using namespace std::chrono;

NoteGame::NoteGame(ITransport& transport, IMidiInput& midi,
                   ChallengeFactory& factory, const GameConfig& config)
    : transport(transport), midi(midi), factory(factory), config(config),
      challengeId(0) {
    Logger::log("[NoteGame] Instance créée");
}

void NoteGame::start() {
    Logger::log("[NoteGame] Démarrage du jeu de notes");
    this->challengeId = 0;
}

GameResult NoteGame::play() {
    GameResult result = {0, 0, 0, 0};
    auto startTime = high_resolution_clock::now();
    int perfectCount = 0;
    const int maxChallenges = this->config.maxChallenges;

    for (int i = 0; i < maxChallenges; ++i) {
        // Générer une note via la factory commune
        std::string targetNoteStr =
            factory.generateNote(config.scale, config.mode);
        this->challengeId++;

        // Envoyer le challenge
        Message challenge("note", {{"note", targetNoteStr},
                                   {"id", std::to_string(this->challengeId)}});
        this->transport.send(challenge);

        Logger::log("[NoteGame] Challenge {}: {}", this->challengeId,
                    targetNoteStr);

        // Attendre les notes jouées
        auto challengeStart = high_resolution_clock::now();
        std::vector<Note> playedNotes = this->midi.readNotes();
        auto challengeEnd = high_resolution_clock::now();

        auto duration =
            duration_cast<milliseconds>(challengeEnd - challengeStart).count();

        // Valider avec AnswerValidator
        bool correct = false;
        std::map<std::string, std::string> resultFields{
            {"id", std::to_string(this->challengeId)},
            {"duration", std::to_string(duration)}};

        if (!playedNotes.empty()) {
            std::string playedNoteStr = playedNotes[0].toString();
            if (validator.valider(playedNoteStr, targetNoteStr)) {
                resultFields["correct"] = playedNoteStr;
                correct = true;
                perfectCount++;
            } else {
                resultFields["incorrect"] = playedNoteStr;
            }
        }

        Message resultMsg("result", resultFields);
        this->transport.send(resultMsg);

        Logger::log("[NoteGame] Résultat: {}",
                    correct ? "correct" : "incorrect");

        if (i < maxChallenges - 1) {
            Message readyMsg = this->transport.receive();
            if (readyMsg.getType() != "ready") break;
        }
    }

    auto endTime = high_resolution_clock::now();
    result.duration = duration_cast<milliseconds>(endTime - startTime).count();
    result.perfect = perfectCount;
    result.total = maxChallenges;

    return result;
}

void NoteGame::stop() {
    Logger::log("[NoteGame] Arrêt du jeu");
    this->midi.close();
}
