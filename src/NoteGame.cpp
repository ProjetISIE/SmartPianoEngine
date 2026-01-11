#include "NoteGame.hpp"
#include "Logger.hpp"
#include <algorithm>
#include <chrono>

using namespace std::chrono;

NoteGame::NoteGame(ITransport& transport, IMidiInput& midi,
                   const GameConfig& config)
    : transport_(transport), midi_(midi), config_(config),
      rng_(std::random_device{}()), challengeId_(0) {
    Logger::log("[NoteGame] Instance créée");
}

void NoteGame::start() {
    Logger::log("[NoteGame] Démarrage du jeu de notes");
    challengeId_ = 0;
}

GameResult NoteGame::play() {
    GameResult result = {0, 0, 0, 0};
    auto startTime = high_resolution_clock::now();

    int perfectCount = 0;
    const int maxChallenges = config_.maxChallenges;

    for (int i = 0; i < maxChallenges; ++i) {
        // Générer une note aléatoire
        Note targetNote = generateRandomNote();
        challengeId_++;

        // Envoyer le challenge
        Message challenge("note");
        challenge.addField("note", targetNote.toString());
        challenge.addField("id", std::to_string(challengeId_));
        transport_.send(challenge);

        Logger::log("[NoteGame] Challenge " + std::to_string(challengeId_) +
                    ": " + targetNote.toString());

        // Attendre les notes jouées
        auto challengeStart = high_resolution_clock::now();
        std::vector<Note> playedNotes = midi_.readNotes();
        auto challengeEnd = high_resolution_clock::now();

        auto duration =
            duration_cast<milliseconds>(challengeEnd - challengeStart).count();

        // Vérifier si correct
        Message resultMsg("result");
        resultMsg.addField("id", std::to_string(challengeId_));
        resultMsg.addField("duration", std::to_string(duration));

        bool correct = false;
        if (!playedNotes.empty() && playedNotes[0] == targetNote) {
            resultMsg.addField("correct", targetNote.toString());
            correct = true;
            perfectCount++;
        } else {
            std::string incorrectNotes;
            for (const auto& note : playedNotes) {
                if (!incorrectNotes.empty()) incorrectNotes += " ";
                incorrectNotes += note.toString();
            }
            resultMsg.addField("incorrect", incorrectNotes);
        }

        transport_.send(resultMsg);

        Logger::log("[NoteGame] Résultat: " +
                    (correct ? "correct" : "incorrect"));

        // Attendre le message "ready" pour le prochain challenge
        if (i < maxChallenges - 1) {
            Message readyMsg = transport_.receive();
            if (readyMsg.type != "ready") {
                Logger::log("[NoteGame] Erreur: ready attendu", true);
                break;
            }
        }
    }

    auto endTime = high_resolution_clock::now();
    auto totalDuration =
        duration_cast<milliseconds>(endTime - startTime).count();

    result.duration = totalDuration;
    result.perfect = perfectCount;
    result.partial = 0;
    result.total = maxChallenges;

    Logger::log("[NoteGame] Partie terminée: " + std::to_string(perfectCount) +
                "/" + std::to_string(maxChallenges));

    return result;
}

void NoteGame::stop() { Logger::log("[NoteGame] Arrêt du jeu"); }

Note NoteGame::generateRandomNote() {
    std::vector<std::string> scaleNotes = getScaleNotes();
    std::uniform_int_distribution<> noteDist(0, scaleNotes.size() - 1);
    std::uniform_int_distribution<> octaveDist(3, 5); // Octaves 3-5

    std::string noteName = scaleNotes[noteDist(rng_)];
    int octave = octaveDist(rng_);

    return Note(noteName, octave);
}

std::vector<std::string> NoteGame::getScaleNotes() {
    // Notes de base pour chaque gamme
    std::map<std::string, std::vector<std::string>> scales;

    // Gamme majeure (ex: Do majeur = C D E F G A B)
    scales["c_maj"] = {"c", "d", "e", "f", "g", "a", "b"};
    scales["d_maj"] = {"d", "e", "f#", "g", "a", "b", "c#"};
    scales["e_maj"] = {"e", "f#", "g#", "a", "b", "c#", "d#"};
    scales["f_maj"] = {"f", "g", "a", "a#", "c", "d", "e"};
    scales["g_maj"] = {"g", "a", "b", "c", "d", "e", "f#"};
    scales["a_maj"] = {"a", "b", "c#", "d", "e", "f#", "g#"};
    scales["b_maj"] = {"b", "c#", "d#", "e", "f#", "g#", "a#"};

    // Gamme mineure (ex: La mineur = A B C D E F G)
    scales["c_min"] = {"c", "d", "d#", "f", "g", "g#", "a#"};
    scales["d_min"] = {"d", "e", "f", "g", "a", "a#", "c"};
    scales["e_min"] = {"e", "f#", "g", "a", "b", "c", "d"};
    scales["f_min"] = {"f", "g", "g#", "a#", "c", "c#", "d#"};
    scales["g_min"] = {"g", "a", "a#", "c", "d", "d#", "f"};
    scales["a_min"] = {"a", "b", "c", "d", "e", "f", "g"};
    scales["b_min"] = {"b", "c#", "d", "e", "f#", "g", "a"};

    std::string key = config_.scale + "_" + config_.mode;
    auto it = scales.find(key);

    if (it != scales.end()) {
        return it->second;
    }

    // Par défaut: Do majeur
    Logger::log("[NoteGame] Gamme inconnue, utilisation de Do majeur");
    return scales["c_maj"];
}
