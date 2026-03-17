#include "NoteGame.hpp"
#include "Logger.hpp"
#include <chrono>
#include <thread>

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
        std::string targetNoteStr =
            factory.generateNote(config.scale, config.mode);
        this->challengeId++;

        Message challenge("note", {{"note", targetNoteStr},
                                   {"id", std::to_string(this->challengeId)}});
        this->transport.send(challenge);

        Logger::log("[NoteGame] Défi {} envoyé: {}", this->challengeId,
                    targetNoteStr);

        auto challengeStart = high_resolution_clock::now();
        std::vector<Note> playedNotes;
        bool quitRequested = false;

        // Boucle d'attente non-bloquante pour MIDI ou Transport
        while (!this->midi.hasNotes()) {
            if (this->transport.hasMessage()) {
                Message msg = this->transport.receive();
                if (msg.getType() == "quit") {
                    Logger::log(
                        "[NoteGame] Quitter demandé pendant le challenge");
                    quitRequested = true;
                    break;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        if (quitRequested) {
            result.total = i; // Ce challenge n'est pas compté
            break;
        }

        playedNotes = this->midi.readNotes();
        auto challengeEnd = high_resolution_clock::now();

        auto duration =
            duration_cast<milliseconds>(challengeEnd - challengeStart).count();

        bool correct = false;
        std::string correctNotes;
        std::string incorrectNotes;

        for (const auto& played : playedNotes) {
            std::string playedNoteStr = played.toString();
            if (validator.valider(playedNoteStr, targetNoteStr)) {
                if (!correctNotes.empty()) correctNotes += " ";
                correctNotes += playedNoteStr;
                correct = true;
            } else {
                if (!incorrectNotes.empty()) incorrectNotes += " ";
                incorrectNotes += playedNoteStr;
            }
        }

        std::map<std::string, std::string> resultFields{
            {"id", std::to_string(this->challengeId)},
            {"duration", std::to_string(duration)}};

        if (!correctNotes.empty()) resultFields["correct"] = correctNotes;
        if (!incorrectNotes.empty()) resultFields["incorrect"] = incorrectNotes;

        if (correctNotes.empty() && incorrectNotes.empty()) {
            resultFields["incorrect"] = "none";
            Logger::log("[NoteGame] Aucune note jouée");
        } else {
            Logger::log("[NoteGame] Résultat: correct='{}' incorrect='{}'",
                        correctNotes, incorrectNotes);
        }

        if (correct && incorrectNotes.empty() && playedNotes.size() == 1) {
            perfectCount++;
        }

        this->transport.send(Message("result", resultFields));

        if (i < maxChallenges - 1) {
            Message readyMsg = this->transport.receive();
            if (readyMsg.getType() == "quit") {
                Logger::log(
                    "[NoteGame] Client demande arrêt pendant la session");
                result.total = i + 1;
                break;
            }
            if (readyMsg.getType() != "ready") {
                Logger::err("[NoteGame] Attendu 'ready', reçu '{}'",
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

    return result;
}

void NoteGame::stop() {
    Logger::log("[NoteGame] Arrêt du jeu");
    this->midi.close();
}
