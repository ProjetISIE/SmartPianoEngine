#include "ChordGame.hpp"
#include "Logger.hpp"
#include <chrono>
#include <thread>

using namespace std::chrono;

ChordGame::ChordGame(ITransport& transport, IMidiInput& midi,
                     ChallengeFactory& factory, const GameConfig& config,
                     bool withInversions)
    : transport(transport), midi(midi), factory(factory), config(config),
      withInversions(withInversions), challengeId(0) {
    Logger::log("[ChordGame] Instance créée (inversions={})",
                withInversions ? "oui" : "non");
}

void ChordGame::start() {
    Logger::log("[ChordGame] Démarrage du jeu d'accords");
    this->challengeId = 0;
}

GameResult ChordGame::play() {
    GameResult result = {0, 0, 0, 0};
    auto startTime = high_resolution_clock::now();
    int perfectCount = 0;
    int partialCount = 0;
    const int maxChallenges = this->config.maxChallenges;

    for (int i = 0; i < maxChallenges; ++i) {
        // Générer un accord via la factory
        std::string name;
        std::vector<std::string> targetNotes;
        int inversion = 1;

        if (withInversions) {
            auto tuple =
                factory.generateInversedChord(config.scale, config.mode);
            name = std::get<0>(tuple);
            targetNotes = std::get<1>(tuple);
            inversion = std::get<2>(tuple);
        } else {
            auto pair = factory.generateChord(config.scale, config.mode);
            name = pair.first;
            targetNotes = pair.second;
        }

        this->challengeId++;

        // Envoyer le challenge
        std::string notesStr;
        for (size_t j = 0; j < targetNotes.size(); ++j) {
            if (j > 0) notesStr += " ";
            notesStr += targetNotes[j];
        }

        Message challenge("chord", {{"name", name},
                                    {"notes", notesStr},
                                    {"id", std::to_string(this->challengeId)}});
        this->transport.send(challenge);

        Logger::log("[ChordGame] Challenge {}: {}", this->challengeId, name);

        // Attendre les notes jouées (non-bloquant avec interruption possible)
        auto challengeStart = high_resolution_clock::now();
        std::vector<Note> playedNotes;
        bool quitRequested = false;

        while (!this->midi.hasNotes()) {
            if (this->transport.hasMessage()) {
                Message msg = this->transport.receive();
                if (msg.getType() == "quit") {
                    Logger::log(
                        "[ChordGame] Quitter demandé pendant challenge");
                    quitRequested = true;
                    break;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        if (quitRequested) {
            result.total = i;
            break;
        }

        playedNotes = this->midi.readNotes();
        auto challengeEnd = high_resolution_clock::now();

        auto duration =
            duration_cast<milliseconds>(challengeEnd - challengeStart).count();

        // Valider avec AnswerValidator
        std::vector<std::string> playedNotesStr;
        for (const auto& n : playedNotes)
            playedNotesStr.push_back(n.toString());

        bool isValid = false;
        if (withInversions) {
            isValid = validator.validerAccordRenversement(
                playedNotesStr, targetNotes, inversion);
        } else {
            isValid = validator.validerAccordSR(playedNotesStr, targetNotes);
        }

        // Déterminer correct/incorrect pour le message résultat
        std::string correctNotes;
        std::string incorrectNotes;
        int correctCount = 0;

        for (const auto& played : playedNotesStr) {
            bool found = false;
            for (const auto& expected : targetNotes) {
                if (played == expected) {
                    if (!correctNotes.empty()) correctNotes += " ";
                    correctNotes += played;
                    found = true;
                    correctCount++;
                    break;
                }
            }
            if (!found) {
                if (!incorrectNotes.empty()) incorrectNotes += " ";
                incorrectNotes += played;
            }
        }

        std::map<std::string, std::string> resultFields{
            {"id", std::to_string(this->challengeId)},
            {"duration", std::to_string(duration)}};
        if (!correctNotes.empty()) resultFields["correct"] = correctNotes;
        if (!incorrectNotes.empty()) resultFields["incorrect"] = incorrectNotes;
        this->transport.send(Message("result", resultFields));

        if (isValid && incorrectNotes.empty()) {
            perfectCount++;
            Logger::log("[ChordGame] Résultat: parfait");
        } else if (correctCount > 0) {
            partialCount++;
            Logger::log("[ChordGame] Résultat: partiel");
        }

        if (i < maxChallenges - 1) {
            Message readyMsg = this->transport.receive();
            if (readyMsg.getType() == "quit") {
                Logger::log("[ChordGame] Client demande arrêt session");
                result.total = i + 1;
                break;
            }
            if (readyMsg.getType() != "ready") {
                Logger::err("[ChordGame] Attendu 'ready', reçu '{}'",
                            readyMsg.getType());
                result.total = i + 1;
                break;
            }
        }
        result.total = i + 1;
    }

    auto endTime = high_resolution_clock::now();
    result.duration = duration_cast<milliseconds>(endTime - startTime).count();
    result.perfect = perfectCount;
    result.partial = partialCount;

    return result;
}

void ChordGame::stop() {
    Logger::log("[ChordGame] Arrêt du jeu");
    this->midi.close();
}
