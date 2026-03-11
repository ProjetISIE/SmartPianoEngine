#include "AnswerValidator.hpp"
#include "Logger.hpp"
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <map>
#include <sys/types.h>
#include <vector>

// Validation d'une seule note jouee par rapport a une note attendue
bool AnswerValidator::valider(const std::string& noteJouee,
                              const std::string& noteAttendue) {
    Logger::log("[AnswerValidator] Validation note {} = {}", noteJouee,
                noteAttendue);
    return noteJouee == noteAttendue;
}

int AnswerValidator::getNoteValue(std::string noteName) const {
    static const std::map<std::string, int> notesMidi = {
        {"C", 0},  {"C#", 1}, {"DB", 1},  {"D", 2},   {"D#", 3}, {"EB", 3},
        {"E", 4},  {"F", 5},  {"F#", 6},  {"GB", 6},  {"G", 7},  {"G#", 8},
        {"AB", 8}, {"A", 9},  {"A#", 10}, {"BB", 10}, {"B", 11}};

    std::transform(noteName.begin(), noteName.end(), noteName.begin(),
                   ::toupper);
    auto it = notesMidi.find(noteName);
    if (it == notesMidi.end()) {
        Logger::err("[AnswerValidator] Note inconnue: {}", noteName);
        return -1;
    }
    return it->second;
}

int AnswerValidator::getAbsoluteMidiValue(const std::string& noteStr) const {
    if (noteStr.empty()) return -1;
    std::string namePart = noteStr.substr(0, noteStr.size() - 1);
    int noteVal = getNoteValue(namePart);
    if (noteVal == -1) return -1;

    try {
        int octave = std::stoi(noteStr.substr(noteStr.size() - 1));
        return noteVal + (octave * 12);
    } catch (...) {
        Logger::err("[AnswerValidator] Octave invalide dans: {}", noteStr);
        return -1;
    }
}

// Validation d'un accord sans tenir compte du renversement
bool AnswerValidator::validerAccordSR(
    const std::vector<std::string>& accordJoue,
    const std::vector<std::string>& accordAttendu) {
    Logger::log("[AnswerValidator] Validation accord sans renversement");

    if (accordJoue.size() != accordAttendu.size()) {
        Logger::err("[AnswerValidator] Taille des accords différente");
        return false;
    }

    std::vector<int> joueModulo, attenduModulo;
    for (const auto& note : accordJoue) {
        int val = getNoteValue(note.substr(0, note.size() - 1));
        if (val != -1) joueModulo.push_back(val);
    }
    for (const auto& note : accordAttendu) {
        int val = getNoteValue(note.substr(0, note.size() - 1));
        if (val != -1) attenduModulo.push_back(val);
    }

    if (joueModulo.size() != accordAttendu.size() ||
        attenduModulo.size() != accordAttendu.size())
        return false;

    std::sort(joueModulo.begin(), joueModulo.end());
    std::sort(attenduModulo.begin(), attenduModulo.end());

    bool resultat = (joueModulo == attenduModulo);
    Logger::log("[AnswerValidator] Validation sans renversement terminée {}",
                resultat ? "Valide" : "Invalide");
    return resultat;
}

// Validation d'un accord avec prise en compte du renversement
bool AnswerValidator::validerAccordRenversement(
    const std::vector<std::string>& accordJoue,
    const std::vector<std::string>& accordAttendu, uint32_t renversement) {
    Logger::log("[AnswerValidator] Validation accord avec renversement {}",
                renversement);

    if (accordJoue.size() != accordAttendu.size()) {
        Logger::err("[AnswerValidator] Taille des accords différente");
        return false;
    }

    std::vector<int> joueMidi;
    for (const auto& note : accordJoue) {
        int val = getAbsoluteMidiValue(note);
        if (val != -1) joueMidi.push_back(val);
    }
    if (joueMidi.size() != accordJoue.size()) return false;
    std::sort(joueMidi.begin(), joueMidi.end());

    std::vector<int> attenduRecalibre;
    // recalibrage selon renversement
    for (uint32_t i = renversement - 1; i < accordAttendu.size(); ++i) {
        int val = getAbsoluteMidiValue(accordAttendu[i]);
        if (val != -1) attenduRecalibre.push_back(val);
    }
    for (uint32_t i = 0; i < renversement - 1; ++i) {
        int val = getAbsoluteMidiValue(accordAttendu[i]);
        if (val != -1) attenduRecalibre.push_back(val);
    }
    if (attenduRecalibre.size() != accordAttendu.size()) return false;

    for (uint32_t i = 1; i < joueMidi.size(); ++i) {
        if (joueMidi[i] - joueMidi[i - 1] > 12) {
            Logger::err(
                "[AnswerValidator] Une des notes est en dehors de l'octave");
            return false;
        }
    }

    for (uint32_t i = 0; i < attenduRecalibre.size(); ++i) {
        if (joueMidi[i] % 12 != attenduRecalibre[i] % 12) {
            Logger::err(
                "[AnswerValidator] Hauteurs des notes non correspondantes");
            return false;
        }
    }

    Logger::log(
        "[AnswerValidator] Validation avec renversement terminée: valide");
    return true;
}
