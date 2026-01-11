#include "GameManager.hpp"
#include "Logger.hpp"
#include <chrono>
#include <cstdlib>
#include <unistd.h>

using namespace std::chrono;

// Fonction principale: lancement du jeu
void GameManager::lancerJeu() {
    Logger::log("[GameManager] En attente des paramètres du jeu...");
    std::string message = socketManager.recevoirMessage();
    Logger::log("[GameManager] Message brut recu: " + message);

    auto params = socketManager.traiterMessage(message);
    if (params.empty()) {
        Logger::log("[GameManager] Erreur: Message invalide ou vide", true);
        return;
    }

    std::string jeu = params["jeu"];
    std::string gamme = params["gamme"];
    std::string mode = params["mode"];

    Logger::log("[GameManager] Paramètres recus: Jeu=" + jeu +
                "  Gamme=" + gamme + "  Mode=" + mode);

    if (jeu == "Jeu de note") {
        lancerJeuDeNote(gamme, mode);
    } else if (jeu == "Jeu d'accord SR") {
        lancerJeuDaccordSR(gamme, mode);
    } else if (jeu == "Jeu d'accord AR") {
        lancerJeuDaccordRenversement(gamme, mode);
    } else {
        Logger::log("[GameManager] Type de jeu non supporté: " + jeu, true);
    }
}

// Jeu d'accords renversés
void GameManager::lancerJeuDaccordRenversement(const std::string& gamme,
                                               const std::string& mode) {
    Logger::log("[GameManager] Lancement du jeu d'accords renversés");
    if (!lectureNote.initialiser()) {
        Logger::log("[GameManager] Erreur: Lecture MIDI impossible", true);
        return;
    }

    auto start = high_resolution_clock::now();
    int accordsCorrects = 0;
    const int maxAccords = 8;

    while (accordsCorrects < maxAccords) {
        auto [nomAccord, notes, renversement] =
            generateur.genererAccordRenversement(gamme, mode);

        std::string renverStr = std::to_string(renversement - 1);
        std::map<std::string, std::string> accordMessage = {
            {"type", "accord_a_jouer"},
            {"nom_accord", nomAccord + " " + renverStr}};

        socketManager.envoyerMessage(accordMessage);
        Logger::log("[GameManager] Accord: " + nomAccord + " " + renverStr);

        bool accordCorrect = false;
        while (!accordCorrect) {
            std::vector<std::string> accordJoue = lectureNote.lireNote();
            if (accordJoue.size() == 3) {
                if (validateur.validerAccordRenversement(accordJoue, notes,
                                                         renversement)) {
                    Logger::log("[GameManager] Accord correct joué");
                    accordsCorrects++;
                    accordCorrect = true;
                } else {
                    Logger::log("[GameManager] Accord incorrect joué", true);
                    socketManager.envoyerMessage(accordMessage);
                }
            } else {
                Logger::log("[GameManager] Nombre incorrect de notes", true);
                socketManager.envoyerMessage(accordMessage);
            }
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    std::map<std::string, std::string> finMessage = {
        {"type", "fin_du_jeu"}, {"score", std::to_string(duration)}};
    socketManager.envoyerMessage(finMessage);
    Logger::log("[GameManager] Jeu terminé, temps total: " +
                std::to_string(duration) + "s");

    restartProgram();
}

// Jeu d'accords standards
void GameManager::lancerJeuDaccordSR(const std::string& gamme,
                                     const std::string& mode) {
    Logger::log("[GameManager] Lancement du jeu d'accords standards");
    if (!lectureNote.initialiser()) {
        Logger::log("[GameManager] Erreur: Lecture MIDI impossible", true);
        return;
    }

    auto start = high_resolution_clock::now();
    int accordsCorrects = 0;
    const int maxAccords = 8;

    while (accordsCorrects < maxAccords) {
        auto [nomAccord, notes] = generateur.genererAccord(gamme, mode);

        std::map<std::string, std::string> accordMessage = {
            {"type", "accord_a_jouer"}, {"nom_accord", nomAccord}};

        socketManager.envoyerMessage(accordMessage);
        Logger::log("[GameManager] Accord envoyé: " + nomAccord);

        bool accordCorrect = false;
        while (!accordCorrect) {
            std::vector<std::string> accordJoue = lectureNote.lireNote();
            if (accordJoue.size() == 3) {
                if (validateur.validerAccordSR(
                        accordJoue, {notes[0], notes[1], notes[2]})) {
                    Logger::log("[GameManager] Accord correct joué");
                    accordsCorrects++;
                    accordCorrect = true;
                } else {
                    Logger::log("[GameManager] Accord incorrect joué", true);
                    socketManager.envoyerMessage(accordMessage);
                }
            } else {
                Logger::log("[GameManager] Nombre incorrect de notes", true);
                socketManager.envoyerMessage(accordMessage);
            }
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    std::map<std::string, std::string> finMessage = {
        {"type", "fin_du_jeu"}, {"score", std::to_string(duration)}};
    socketManager.envoyerMessage(finMessage);
    Logger::log("[GameManager] Jeu terminé, temps total: " +
                std::to_string(duration) + "s");

    restartProgram();
}

// Jeu de notes
void GameManager::lancerJeuDeNote(const std::string& gamme,
                                  const std::string& mode) {
    Logger::log("[GameManager] Lancement du jeu de notes");
    if (!lectureNote.initialiser()) {
        Logger::log("[GameManager] Erreur: Lecture MIDI impossible", true);
        return;
    }

    auto start = high_resolution_clock::now();
    int notesCorrectes = 0;
    const int maxNotes = 10;

    while (notesCorrectes < maxNotes) {
        std::string note = generateur.generer(gamme, mode);
        std::map<std::string, std::string> noteMessage = {
            {"type", "note_a_jouer"}, {"note", note}};
        socketManager.envoyerMessage(noteMessage);
        Logger::log("[GameManager] Note envoyée: " + note);
        bool noteCorrecte = false;
        while (!noteCorrecte) {
            std::vector<std::string> notesJouees = lectureNote.lireNote();
            if (notesJouees.size() == 1) {
                if (validateur.valider(notesJouees[0], note)) {
                    Logger::log("[GameManager] Notes OK: " + notesJouees[0]);
                    notesCorrectes++;
                    noteCorrecte = true;
                } else {
                    Logger::log("[GameManager] Note incorrecte", true);
                    socketManager.envoyerMessage(noteMessage);
                }
            } else {
                Logger::log("[GameManager] Nombre incorrect de notes", true);
                socketManager.envoyerMessage(noteMessage);
            }
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    std::map<std::string, std::string> finMessage = {
        {"type", "fin_du_jeu"}, {"score", std::to_string(duration)}};

    socketManager.envoyerMessage(finMessage);
    Logger::log("[GameManager] Jeu terminé, temps total: " +
                std::to_string(duration) + "s");

    restartProgram();
}

// Redémarrage du programme
// void GameManager::restartProgram() {
//     Logger::log("[GameManager] Redémarrage de Smart Piano");
//     const char* program = "TODO auto detect starting path";
//     const char* args[] = {program, nullptr};
//     execvp(program, const_cast<char* const*>(args));
//     Logger::log("[GameManager] Échec du redémarrage", true);
//     std::exit(EXIT_FAILURE);
// }
