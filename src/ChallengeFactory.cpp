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
    Logger::log("[ChallengeFactory] Générateur initialisé");
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
    // Degrés I, IV, V
    static const std::vector<int> degrees = {0, 3, 4};
    std::uniform_int_distribution<> degDist(0, degrees.size() - 1);
    std::uniform_int_distribution<> octaveDist(3, 4);

    int root = degrees[degDist(rng)];
    int third = (root + 2) % 7;
    int fifth = (root + 4) % 7;
    int octave = octaveDist(rng);

    std::string rootName = notes[root];
    rootName[0] = std::toupper(rootName[0]);
    std::string name = rootName + (mode == "maj" ? " majeur" : " mineur");

    std::vector<std::string> chordNotes = {notes[root] + std::to_string(octave),
                                           notes[third] + std::to_string(octave),
                                           notes[fifth] + std::to_string(octave)};
    return {name, chordNotes};
}

std::tuple<std::string, std::vector<std::string>, int>
ChallengeFactory::generateInversedChord(const std::string& scale,
                                        const std::string& mode) {
    auto chord = generateChord(scale, mode);
    std::uniform_int_distribution<> invDist(1, 3);
    int inversion = invDist(rng);

    if (inversion == 2) { // 1er renversement
        std::string n1 = chord.second[0];
        chord.second.erase(chord.second.begin());
        int oct = n1.back() - '0';
        chord.second.push_back(n1.substr(0, n1.size() - 1) +
                               std::to_string(oct + 1));
    } else if (inversion == 3) { // 2ème renversement
        std::string n1 = chord.second[0];
        std::string n2 = chord.second[1];
        chord.second.erase(chord.second.begin(), chord.second.begin() + 2);
        int oct1 = n1.back() - '0';
        int oct2 = n2.back() - '0';
        chord.second.push_back(n1.substr(0, n1.size() - 1) +
                               std::to_string(oct1 + 1));
        chord.second.push_back(n2.substr(0, n2.size() - 1) +
                               std::to_string(oct2 + 1));
    }

    std::string name = chord.first;
    if (inversion > 1) name += " " + std::to_string(inversion - 1);

    return {name, chord.second, inversion};
}
