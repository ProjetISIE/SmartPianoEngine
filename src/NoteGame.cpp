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

    Logger::log("[NoteGame] Début boucle: {} challenges", maxChallenges);

    for (int i = 0; i < maxChallenges; ++i) {
        Logger::log("[NoteGame] Début challenge {}/{}", i + 1, maxChallenges);
        
        std::string targetNoteStr =
            factory.generateNote(config.scale, config.mode);
        this->challengeId++;

        Message challenge("note", {{"note", targetNoteStr},
                                   {"id", std::to_string(this->challengeId)}});
        this->transport.send(challenge);

        Logger::log("[NoteGame] Défi envoyé: id={} note={}", this->challengeId,
                    targetNoteStr);

        auto challengeStart = high_resolution_clock::now();
        std::vector<Note> playedNotes = this->midi.readNotes();
        auto challengeEnd = high_resolution_clock::now();

        auto duration =
            duration_cast<milliseconds>(challengeEnd - challengeStart).count();
        Logger::log("[NoteGame] Notes reçues (nb={})", playedNotes.size());

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
        }

        if (correct && incorrectNotes.empty() && playedNotes.size() == 1) {
            perfectCount++;
        }

        this->transport.send(Message("result", resultFields));
        Logger::log("[NoteGame] Résultat envoyé");

        if (i < maxChallenges - 1) {
            Logger::log("[NoteGame] En attente de 'ready'...");
            Message readyMsg = this->transport.receive();
            Logger::log("[NoteGame] Message reçu après result: '{}'", readyMsg.getType());
            if (readyMsg.getType() != "ready") {
                Logger::err("[NoteGame] Abandon car attendu 'ready', reçu '{}'",
                            readyMsg.getType());
                break;
            }
        }
    }

    auto endTime = high_resolution_clock::now();
    result.duration = duration_cast<milliseconds>(endTime - startTime).count();
    result.perfect = perfectCount;
    result.total = maxChallenges;

    Logger::log("[NoteGame] play() se termine");
    return result;
}

void NoteGame::stop() {
    Logger::log("[NoteGame] stop() appelé");
    this->midi.close();
}
