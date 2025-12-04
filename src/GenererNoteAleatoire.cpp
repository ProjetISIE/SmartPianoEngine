#include "GenererNoteAleatoire.hpp"
#include "BaseAccords.hpp"
#include "Logger.hpp"
#include <cstdlib>
#include <ctime>
#include <map>
#include <vector>

// Constructeur pour initialiser le generateur de nombres aleatoires
GenererNoteAleatoire::GenererNoteAleatoire() {
    srand(static_cast<unsigned>(
        time(nullptr))); // Initialisation du generateur aleatoire
    Logger::log("[GenererNoteAleatoire] ligne 13 : Generateur de nombres "
                "aleatoires initialise");
}

// Fonction pour generer une note aleatoire en fonction de la gamme et du mode
std::string GenererNoteAleatoire::generer(const std::string& gamme,
                                          const std::string& mode) {
    Logger::log("[GenererNoteAleatoire] ligne 18 : Generation d'une note pour "
                "la gamme " +
                gamme + " et le mode " + mode);

    const std::map<std::string, std::vector<std::string>> gammesMajeures = {
        {"Do", {"C", "D", "E", "F", "G", "A", "B"}},
        {"Sol", {"G", "A", "B", "C", "D", "E", "F#"}},
        {"Ré", {"D", "E", "F#", "G", "A", "B", "C#"}},
        {"La", {"A", "B", "C#", "D", "E", "F#", "G#"}},
        {"Mi", {"E", "F#", "G#", "A", "B", "C#", "D#"}},
        {"Si", {"B", "C#", "D#", "E", "F#", "G#", "A#"}},
        {"Fa", {"F", "G", "A", "A#", "C", "D", "E"}}};

    const std::map<std::string, std::vector<std::string>> gammesMineures = {
        {"La", {"A", "B", "C", "D", "E", "F", "G"}},
        {"Mi", {"E", "F#", "G", "A", "B", "C", "D"}},
        {"Si", {"B", "C#", "D", "E", "F#", "G", "A"}},
        {"Ré", {"D", "E", "F", "G", "A", "A#", "C"}},
        {"Sol", {"G", "A", "A#", "C", "D", "D#", "F"}},
        {"Do", {"C", "D", "D#", "F", "G", "G#", "A#"}},
        {"Fa", {"F", "G", "G#", "A#", "C", "C#", "D#"}}};

    std::vector<std::string> notes;
    if (mode == "Majeur" && gammesMajeures.count(gamme)) {
        notes = gammesMajeures.at(gamme);
    } else if (mode == "Mineur" && gammesMineures.count(gamme)) {
        notes = gammesMineures.at(gamme);
    } else {
        Logger::log(
            "[GenererNoteAleatoire] ligne 46 : Gamme ou mode invalide : " +
                gamme + " " + mode,
            true);
        return "";
    }

    std::string note;
    int octave;
    int index;
    do {
        octave = (rand() % 5) + 2; // Octave aleatoire entre 2 et 6
        index = rand() % notes.size();
        note = notes[index] + std::to_string(octave);
    } while (octave == 6 &&
             notes[index] != "C"); // Exclusion des notes au-dela de C6

    Logger::log("[GenererNoteAleatoire] ligne 59 : Note generee : " + note);
    return note;
}

// Fonction pour generer un accord aleatoire
std::pair<std::string, std::vector<std::string>>
GenererNoteAleatoire::genererAccord(const std::string& gamme,
                                    const std::string& mode) {
    Logger::log("[GenererNoteAleatoire] ligne 66 : Generation d'un accord pour "
                "la gamme " +
                gamme + " et le mode " + mode);
    BaseAccords baseAccords;

    auto it = baseAccords.accords.find(gamme + " " + mode);
    if (it == baseAccords.accords.end()) {
        Logger::log(
            "[GenererNoteAleatoire] ligne 71 : Gamme ou mode invalide : " +
                gamme + " " + mode,
            true);
        return {"", {}};
    }

    const auto& accords = it->second;
    auto accordIt = accords.begin();
    std::advance(accordIt, rand() % accords.size());

    std::string nomAccord = accordIt->first;
    const auto& notesMidi = accordIt->second;
    std::vector<std::string> notes;
    static const std::map<int, std::string> notesMap = {
        {0, "C"},  {1, "C#"}, {2, "D"},  {3, "D#"}, {4, "E"},   {5, "F"},
        {6, "F#"}, {7, "G"},  {8, "G#"}, {9, "A"},  {10, "A#"}, {11, "B"}};

    for (int noteMidi : notesMidi) {
        int noteIndex = noteMidi % 12;
        int octave = (noteMidi / 12) - 1;
        notes.push_back(notesMap.at(noteIndex) + std::to_string(octave));
    }

    Logger::log(
        "[GenererNoteAleatoire] ligne 93 : Accord genere : " + nomAccord +
        " (" + notes[0] + ", " + notes[1] + ", " + notes[2] + ")");
    return {nomAccord, notes};
}

// Fonction pour generer un accord avec renversement
std::tuple<std::string, std::vector<std::string>, int>
GenererNoteAleatoire::genererAccordRenversement(const std::string& gamme,
                                                const std::string& mode) {
    Logger::log("[GenererNoteAleatoire] ligne 100 : Generation d'un accord "
                "avec renversement pour la gamme " +
                gamme + " et le mode " + mode);
    BaseAccords baseAccords;

    auto it = baseAccords.accords.find(gamme + " " + mode);
    if (it == baseAccords.accords.end()) {
        Logger::log(
            "[GenererNoteAleatoire] ligne 105 : Gamme ou mode invalide : " +
                gamme + " " + mode,
            true);
        return {"", {}, 0};
    }

    const auto& accords = it->second;
    auto accordIt = accords.begin();
    std::advance(accordIt, rand() % accords.size());

    std::string nomAccord = accordIt->first;
    const auto& notesMidi = accordIt->second;
    std::vector<std::string> notes;
    static const std::map<int, std::string> notesMap = {
        {0, "C"},  {1, "C#"}, {2, "D"},  {3, "D#"}, {4, "E"},   {5, "F"},
        {6, "F#"}, {7, "G"},  {8, "G#"}, {9, "A"},  {10, "A#"}, {11, "B"}};

    for (int noteMidi : notesMidi) {
        int noteIndex = noteMidi % 12;
        int octave = (noteMidi / 12) - 1;
        notes.push_back(notesMap.at(noteIndex) + std::to_string(octave));
    }

    int renversement = (rand() % 3) + 1;
    Logger::log(
        "[GenererNoteAleatoire] ligne 128 : Accord genere : " + nomAccord +
        " avec renversement " + std::to_string(renversement) + " (" + notes[0] +
        ", " + notes[1] + ", " + notes[2] + ")");
    return {nomAccord, notes, renversement};
}
