#include "ChordGame.hpp"
#include "Logger.hpp"
#include <chrono>
#include <algorithm>
#include <map>

using namespace std::chrono;

ChordGame::ChordGame(ITransport& transport, IMidiInput& midi, 
                     const GameConfig& config, bool withInversions)
    : transport_(transport), midi_(midi), config_(config),
      withInversions_(withInversions), rng_(std::random_device{}()), 
      challengeId_(0) {
    Logger::log("[ChordGame] Instance créée (inversions=" + 
                std::string(withInversions ? "oui" : "non") + ")");
}

void ChordGame::start() {
    Logger::log("[ChordGame] Démarrage du jeu d'accords");
    challengeId_ = 0;
}

GameResult ChordGame::play() {
    GameResult result = {0, 0, 0, 0};
    auto startTime = high_resolution_clock::now();
    
    int perfectCount = 0;
    int partialCount = 0;
    const int maxChallenges = config_.maxChallenges;

    for (int i = 0; i < maxChallenges; ++i) {
        // Générer un accord aléatoire
        Chord targetChord = generateRandomChord();
        challengeId_++;

        // Envoyer le challenge
        Message challenge("chord");
        challenge.addField("name", targetChord.name);
        
        std::string notesStr;
        for (size_t j = 0; j < targetChord.notes.size(); ++j) {
            if (j > 0) notesStr += " ";
            notesStr += targetChord.notes[j].toString();
        }
        challenge.addField("notes", notesStr);
        challenge.addField("id", std::to_string(challengeId_));
        
        transport_.send(challenge);

        Logger::log("[ChordGame] Challenge " + std::to_string(challengeId_) + 
                    ": " + targetChord.name);

        // Attendre les notes jouées
        auto challengeStart = high_resolution_clock::now();
        std::vector<Note> playedNotes = midi_.readNotes();
        auto challengeEnd = high_resolution_clock::now();
        
        auto duration = duration_cast<milliseconds>(challengeEnd - challengeStart).count();

        // Valider l'accord
        int correctCount = validateChord(playedNotes, targetChord.notes);
        
        Message resultMsg("result");
        resultMsg.addField("id", std::to_string(challengeId_));
        resultMsg.addField("duration", std::to_string(duration));

        // Construire les listes de notes correctes et incorrectes
        std::string correctNotes;
        std::string incorrectNotes;

        for (const auto& played : playedNotes) {
            bool found = false;
            for (const auto& expected : targetChord.notes) {
                if (played == expected) {
                    if (!correctNotes.empty()) correctNotes += " ";
                    correctNotes += played.toString();
                    found = true;
                    break;
                }
            }
            if (!found) {
                if (!incorrectNotes.empty()) incorrectNotes += " ";
                incorrectNotes += played.toString();
            }
        }

        if (!correctNotes.empty()) {
            resultMsg.addField("correct", correctNotes);
        }
        if (!incorrectNotes.empty()) {
            resultMsg.addField("incorrect", incorrectNotes);
        }

        // Déterminer si parfait ou partiel
        if (correctCount == static_cast<int>(targetChord.notes.size()) && 
            incorrectNotes.empty()) {
            perfectCount++;
            Logger::log("[ChordGame] Résultat: parfait");
        } else if (correctCount > 0) {
            partialCount++;
            Logger::log("[ChordGame] Résultat: partiel");
        } else {
            Logger::log("[ChordGame] Résultat: incorrect");
        }

        transport_.send(resultMsg);

        // Attendre le message "ready" pour le prochain challenge
        if (i < maxChallenges - 1) {
            Message readyMsg = transport_.receive();
            if (readyMsg.type != "ready") {
                Logger::log("[ChordGame] Erreur: ready attendu", true);
                break;
            }
        }
    }

    auto endTime = high_resolution_clock::now();
    auto totalDuration = duration_cast<milliseconds>(endTime - startTime).count();

    result.duration = totalDuration;
    result.perfect = perfectCount;
    result.partial = partialCount;
    result.total = maxChallenges;

    Logger::log("[ChordGame] Partie terminée: parfait=" + 
                std::to_string(perfectCount) + " partiel=" + 
                std::to_string(partialCount) + "/" + std::to_string(maxChallenges));

    return result;
}

void ChordGame::stop() {
    Logger::log("[ChordGame] Arrêt du jeu");
}

ChordGame::Chord ChordGame::generateRandomChord() {
    std::vector<int> degrees = getChordDegrees();
    std::uniform_int_distribution<> degreeDist(0, degrees.size() - 1);
    std::uniform_int_distribution<> octaveDist(3, 4); // Octaves 3-4

    int degree = degrees[degreeDist(rng_)];
    int octave = octaveDist(rng_);

    Chord chord = buildChord(degree, octave);

    // Appliquer un renversement si demandé
    if (withInversions_) {
        std::uniform_int_distribution<> inversionDist(0, 2);
        int inversion = inversionDist(rng_);
        if (inversion > 0) {
            chord = applyInversion(chord, inversion);
        }
    }

    return chord;
}

std::vector<int> ChordGame::getChordDegrees() {
    // Pour simplifier, utiliser les degrés I, IV, V (les plus courants)
    return {1, 4, 5};
}

ChordGame::Chord ChordGame::buildChord(int degree, int octave) {
    // Notes de la gamme selon la configuration
    std::map<std::string, std::vector<std::string>> scaleNotes;
    scaleNotes["c_maj"] = {"c", "d", "e", "f", "g", "a", "b"};
    scaleNotes["d_maj"] = {"d", "e", "f#", "g", "a", "b", "c#"};
    scaleNotes["e_maj"] = {"e", "f#", "g#", "a", "b", "c#", "d#"};
    scaleNotes["f_maj"] = {"f", "g", "a", "a#", "c", "d", "e"};
    scaleNotes["g_maj"] = {"g", "a", "b", "c", "d", "e", "f#"};
    scaleNotes["a_maj"] = {"a", "b", "c#", "d", "e", "f#", "g#"};
    scaleNotes["b_maj"] = {"b", "c#", "d#", "e", "f#", "g#", "a#"};

    std::string key = config_.scale + "_" + config_.mode;
    auto it = scaleNotes.find(key);
    std::vector<std::string> notes = (it != scaleNotes.end()) ? it->second : scaleNotes["c_maj"];

    // Construire l'accord à partir du degré (fondamentale, tierce, quinte)
    int root = degree - 1;  // Indexé à 0
    int third = (root + 2) % 7;
    int fifth = (root + 4) % 7;

    Chord chord;
    
    // Nom de l'accord
    std::string rootName = notes[root];
    rootName[0] = std::toupper(rootName[0]); // Majuscule
    chord.name = rootName;
    if (config_.mode == "maj") {
        chord.name += " majeur";
    } else {
        chord.name += " mineur";
    }

    // Notes de l'accord
    chord.notes.push_back(Note(notes[root], octave));
    chord.notes.push_back(Note(notes[third], octave));
    chord.notes.push_back(Note(notes[fifth], octave));
    chord.inversion = 0;

    return chord;
}

ChordGame::Chord ChordGame::applyInversion(const Chord& chord, int inversion) {
    Chord inverted = chord;
    inverted.inversion = inversion;
    inverted.name = chord.name + " " + std::to_string(inversion);

    // Premier renversement: déplacer la première note une octave plus haut
    if (inversion == 1) {
        Note first = inverted.notes[0];
        inverted.notes.erase(inverted.notes.begin());
        inverted.notes.push_back(Note(first.getName(), first.getOctave() + 1));
    }
    // Deuxième renversement: déplacer les deux premières notes une octave plus haut
    else if (inversion == 2) {
        Note first = inverted.notes[0];
        Note second = inverted.notes[1];
        inverted.notes.erase(inverted.notes.begin(), inverted.notes.begin() + 2);
        inverted.notes.push_back(Note(first.getName(), first.getOctave() + 1));
        inverted.notes.push_back(Note(second.getName(), second.getOctave() + 1));
    }

    return inverted;
}

int ChordGame::validateChord(const std::vector<Note>& played, 
                             const std::vector<Note>& expected) {
    int correctCount = 0;
    
    for (const auto& playedNote : played) {
        for (const auto& expectedNote : expected) {
            if (playedNote == expectedNote) {
                correctCount++;
                break;
            }
        }
    }

    return correctCount;
}
