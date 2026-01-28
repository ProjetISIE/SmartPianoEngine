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

// Validation d'un accord sans tenir compte du renversement
bool AnswerValidator::validerAccordSR(
    const std::vector<std::string>& accordJoue,
    const std::vector<std::string>& accordAttendu) {
    Logger::log("[AnswerValidator] Validation accord sans renversement");

    if (accordJoue.size() != accordAttendu.size()) {
        Logger::err("[AnswerValidator] Taille des accords différente");
        return false;
    }

    auto convertirEnModulo12 = [](const std::string& note) -> int {
        static const std::map<std::string, int> notesMidi = {
            {"C", 0},  {"C#", 1}, {"D", 2},  {"D#", 3}, {"E", 4},   {"F", 5},
            {"F#", 6}, {"G", 7},  {"G#", 8}, {"A", 9},  {"A#", 10}, {"B", 11}};
        std::string nomNote = note.substr(0, note.size() - 1);
        std::transform(nomNote.begin(), nomNote.end(), nomNote.begin(),
                       ::toupper);
        return notesMidi.at(nomNote);
    };

    std::vector<int> joueModulo, attenduModulo;
    for (const auto& note : accordJoue) {
        joueModulo.push_back(convertirEnModulo12(note));
    }
    for (const auto& note : accordAttendu) {
        attenduModulo.push_back(convertirEnModulo12(note));
    }

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

    auto convertirEnMidi = [](const std::string& note) -> int {
        static const std::map<std::string, int> notesMidi = {
            {"C", 0},  {"C#", 1}, {"D", 2},  {"D#", 3}, {"E", 4},   {"F", 5},
            {"F#", 6}, {"G", 7},  {"G#", 8}, {"A", 9},  {"A#", 10}, {"B", 11}};
        std::string nomNote = note.substr(0, note.size() - 1);
        std::transform(nomNote.begin(), nomNote.end(), nomNote.begin(),
                       ::toupper);
        int octave = std::stoi(note.substr(note.size() - 1));
        return notesMidi.at(nomNote) + (octave * 12);
    };

    std::vector<int> joueMidi;
    for (const auto& note : accordJoue) {
        joueMidi.push_back(convertirEnMidi(note));
    }
    std::sort(joueMidi.begin(), joueMidi.end());

    std::vector<int> attenduRecalibre;
    for (uint32_t i = renversement - 1; i < accordAttendu.size(); ++i) {
        attenduRecalibre.push_back(convertirEnMidi(accordAttendu[i]));
    }
    for (uint32_t i = 0; i < renversement - 1; ++i) {
        attenduRecalibre.push_back(convertirEnMidi(accordAttendu[i]));
    }

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
