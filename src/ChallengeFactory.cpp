#include "ChallengeFactory.hpp"
#include "ChordRepository.hpp"
#include "Logger.hpp"
#include <algorithm>
#include <random>

// Centralisation des gammes harmonisées avec le protocole (clés: c_maj, d_min…)
const std::map<std::string, std::vector<std::string>> ChallengeFactory::scales =
    {{"c_maj", {"c", "d", "e", "f", "g", "a", "b"}},
     {"d_maj", {"d", "e", "f#", "g", "a", "b", "c#"}},
     {"e_maj", {"e", "f#", "g#", "a", "b", "c#", "d#"}},
     {"f_maj", {"f", "g", "a", "a#", "c", "d", "e"}},
     {"g_maj", {"g", "a", "b", "c", "d", "e", "f#"}},
     {"a_maj", {"a", "b", "c#", "d", "e", "f#", "g#"}},
     {"b_maj", {"b", "c#", "d#", "e", "f#", "g#", "a#"}},
     {"c_min", {"c", "d", "d#", "f", "g", "g#", "a#"}},
     {"d_min", {"d", "e", "f", "g", "a", "a#", "c"}},
     {"e_min", {"e", "f#", "g", "a", "b", "c", "d"}},
     {"f_min", {"f", "g", "g#", "a#", "c", "c#", "d#"}},
     {"g_min", {"g", "a", "a#", "c", "d", "d#", "f"}},
     {"a_min", {"a", "b", "c", "d", "e", "f", "g"}},
     {"b_min", {"b", "c#", "d", "e", "f#", "g", "a"}}};

ChallengeFactory::ChallengeFactory() : rng(std::random_device{}()) {
    initMarkovAndSR();
    Logger::log("[ChallengeFactory] Générateur initialisé avec Markov + SR");
}

void ChallengeFactory::initMarkovAndSR() {
    // Matrice de Markov de base pour la pop (7 degrés: I à vii°)
    // Lignes: de quel degré on part. Colonnes: vers quel degré on va.
    // Base: I(0), ii(1), iii(2), IV(3), V(4), vi(5), vii°(6)
    markovMatrix = {
        {0.05, 0.10, 0.05, 0.20, 0.30, 0.25, 0.05}, // I
        {0.05, 0.05, 0.05, 0.25, 0.50, 0.05, 0.05}, // ii
        {0.05, 0.05, 0.05, 0.35, 0.05, 0.40, 0.05}, // iii
        {0.25, 0.20, 0.05, 0.05, 0.35, 0.05, 0.05}, // IV
        {0.50, 0.05, 0.05, 0.05, 0.05, 0.25, 0.05}, // V
        {0.05, 0.20, 0.05, 0.45, 0.15, 0.05, 0.05}, // vi
        {0.70, 0.05, 0.05, 0.05, 0.05, 0.05, 0.05}  // vii°
    };

    // Initialisation du multiplicateur de SR à 1.0 partout
    srMultiplier =
        std::vector<std::vector<double>>(7, std::vector<double>(7, 1.0));
}

void ChallengeFactory::feedbackLastChallenge(bool success) {
    if (lastChordIdx >= 0 && currentChordIdx >= 0) {
        if (!success) {
            // Echec : on augmente fortement le multiplicateur (urgence)
            srMultiplier[lastChordIdx][currentChordIdx] *= 5.0;
            // Plafond pour éviter un overflow
            if (srMultiplier[lastChordIdx][currentChordIdx] > 100.0) {
                srMultiplier[lastChordIdx][currentChordIdx] = 100.0;
            }
        } else {
            // Succès : on divise le multiplicateur par 2 (espacement)
            srMultiplier[lastChordIdx][currentChordIdx] /= 2.0;
            // Plancher à 1.0 (on ne descend pas sous la probabilité naturelle)
            if (srMultiplier[lastChordIdx][currentChordIdx] < 1.0) {
                srMultiplier[lastChordIdx][currentChordIdx] = 1.0;
            }
        }
    }
}

static int noteToValue(const std::string& noteName, int octave) {
    static const std::map<char, int> baseValues = {
        {'c', 0}, {'d', 2}, {'e', 4}, {'f', 5}, {'g', 7}, {'a', 9}, {'b', 11}};
    int val = baseValues.at(noteName[0]);
    if (noteName.size() > 1) {
        if (noteName[1] == '#') val += 1;
        else if (noteName[1] == 'b') val -= 1;
    }
    return val + (octave + 1) * 12;
}

std::vector<std::string>
ChallengeFactory::getScaleNotes(const std::string& scale,
                                const std::string& mode) {
    std::string key = scale + "_" + mode;
    auto it = scales.find(key);
    if (it != scales.end()) return it->second;
    return scales.at("c_maj");
}

std::string ChallengeFactory::generateNote(const std::string& scale,
                                           const std::string& mode) {
    auto notes = getScaleNotes(scale, mode);
    std::uniform_int_distribution<> noteDist(0, notes.size() - 1);
    std::uniform_int_distribution<> octaveDist(3, 5);
    return notes[noteDist(rng)] + std::to_string(octaveDist(rng));
}

std::pair<std::string, std::vector<std::string>>
ChallengeFactory::generateChord(const std::string& scale,
                                const std::string& mode) {
    auto notes = getScaleNotes(scale, mode);
    std::uniform_int_distribution<> octaveDist(3, 4);

    int rootIdx, baseOctave;

    // Génération basée sur Markov + Répétition Espacée (SR)
    if (currentChordIdx == -1) {
        // Premier accord : tirage aléatoire uniforme
        std::uniform_int_distribution<> degDist(0, 6);
        rootIdx = degDist(rng);
    } else {
        // Tirage pondéré
        std::vector<double> weights(7, 0.0);
        for (int i = 0; i < 7; ++i) {
            weights[i] = markovMatrix[currentChordIdx][i] *
                         srMultiplier[currentChordIdx][i];
        }
        weights[currentChordIdx] =
            0.0; // Eviter la répétition du même accord en boucle

        std::discrete_distribution<> dist(weights.begin(), weights.end());
        rootIdx = dist(rng);
    }

    baseOctave = octaveDist(rng);

    // Mise à jour de l'état des transitions
    lastChordIdx = currentChordIdx;
    currentChordIdx = rootIdx;
    lastOctave = baseOctave;

    int thirdIdx = (rootIdx + 2) % 7;
    int fifthIdx = (rootIdx + 4) % 7;

    int rootVal = noteToValue(notes[rootIdx], baseOctave);

    int thirdOctave = baseOctave;
    int thirdVal = noteToValue(notes[thirdIdx], thirdOctave);
    while (thirdVal <= rootVal) {
        thirdOctave++;
        thirdVal = noteToValue(notes[thirdIdx], thirdOctave);
    }

    int fifthOctave = thirdOctave;
    int fifthVal = noteToValue(notes[fifthIdx], fifthOctave);
    while (fifthVal <= thirdVal) {
        fifthOctave++;
        fifthVal = noteToValue(notes[fifthIdx], fifthOctave);
    }

    std::string rootName = notes[rootIdx];
    rootName[0] = std::toupper(rootName[0]);

    // Détermination du type d'accord (majeur, mineur, diminué)
    std::string chordType;
    if (mode == "maj") {
        if (rootIdx == 0 || rootIdx == 3 || rootIdx == 4) chordType = " majeur";
        else if (rootIdx == 6) chordType = " diminué";
        else chordType = " mineur";
    } else { // mineur
        if (rootIdx == 2 || rootIdx == 5 || rootIdx == 6) chordType = " majeur";
        else if (rootIdx == 1) chordType = " diminué";
        else chordType = " mineur";
    }
    std::string name = rootName + chordType;

    std::vector<std::string> chordNotes = {
        notes[rootIdx] + std::to_string(baseOctave),
        notes[thirdIdx] + std::to_string(thirdOctave),
        notes[fifthIdx] + std::to_string(fifthOctave)};
    return {name, chordNotes};
}

std::tuple<std::string, std::vector<std::string>, int>
ChallengeFactory::generateInversedChord(const std::string& scale,
                                        const std::string& mode) {
    auto chord = generateChord(scale, mode);
    std::uniform_int_distribution<> invDist(1, 3);
    int inversion = invDist(rng);

    auto moveNoteUp = [](std::vector<std::string>& notes) {
        if (notes.empty()) return;
        std::string first = notes[0];
        notes.erase(notes.begin());
        int oct = first.back() - '0';
        notes.push_back(first.substr(0, first.size() - 1) +
                        std::to_string(oct + 1));
    };

    if (inversion == 2) { // 1er renversement
        moveNoteUp(chord.second);
    } else if (inversion == 3) { // 2ème renversement
        moveNoteUp(chord.second);
        moveNoteUp(chord.second);
    }

    std::string name = chord.first;
    if (inversion > 1) name += " " + std::to_string(inversion - 1);

    return {name, chord.second, inversion};
}
