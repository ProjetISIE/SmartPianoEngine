#include "NoteGame.hpp"
#include "Logger.hpp"
#include <chrono>

using namespace std::chrono;

NoteGame::NoteGame(ITransport& transport, IMidiInput& midi,
                   const GameConfig& config)
    : transport(transport), midi(midi), config(config),
      rng(std::random_device{}()), challengeId(0) {
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
        // Générer une note aléatoire
        Note targetNote = generateRandomNote();
        this->challengeId++;

        // Envoyer le challenge
        Message challenge("note", {{"note", targetNote.toString()},
                                   {"id", std::to_string(this->challengeId)}});
        this->transport.send(challenge);

        Logger::log("[NoteGame] Challenge {}: {}", this->challengeId,
                    targetNote.toString());

        // Attendre les notes jouées
        auto challengeStart = high_resolution_clock::now();
        std::vector<Note> playedNotes = this->midi.readNotes();
        auto challengeEnd = high_resolution_clock::now();

        auto duration =
            duration_cast<milliseconds>(challengeEnd - challengeStart).count();

        // Vérifier si correct
        bool correct = false;
        std::map<std::string, std::string> resultFields{
            {"id", std::to_string(this->challengeId)},
            {"duration", std::to_string(duration)}};

        if (!playedNotes.empty() && playedNotes[0] == targetNote) {
            resultFields["correct"] = targetNote.toString();
            correct = true;
            perfectCount++;
        } else {
            std::string incorrectNotes;
            for (const auto& note : playedNotes) {
                if (!incorrectNotes.empty()) incorrectNotes += " ";
                incorrectNotes += note.toString();
            }
            resultFields["incorrect"] = incorrectNotes;
        }

        Message resultMsg("result", resultFields);
        this->transport.send(resultMsg);

        Logger::log("[NoteGame] Résultat: {}",
                    correct ? "correct" : "incorrect");

        // Attendre le message "ready" pour le prochain challenge
        if (i < maxChallenges - 1) {
            Message readyMsg = this->transport.receive();
            if (readyMsg.getType() != "ready") {
                Logger::err("[NoteGame] Erreur: ready attendu");
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

    Logger::log("[NoteGame] Partie terminée: {}/{}", perfectCount,
                maxChallenges);

    return result;
}

void NoteGame::stop() { Logger::log("[NoteGame] Arrêt du jeu"); }

Note NoteGame::generateRandomNote() {
    std::vector<std::string> scaleNotes = getScaleNotes();
    std::uniform_int_distribution<> noteDist(0, scaleNotes.size() - 1);
    std::uniform_int_distribution<> octaveDist(3, 5); // Octaves 3-5

    std::string noteName = scaleNotes[noteDist(rng)];
    int octave = octaveDist(rng);

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

    std::string key = config.scale + "_" + config.mode;
    auto it = scales.find(key);

    if (it != scales.end()) return it->second;

    // Par défaut: Do majeur
    Logger::log("[NoteGame] Gamme inconnue, utilisation de Do majeur");
    return scales["c_maj"];
}
