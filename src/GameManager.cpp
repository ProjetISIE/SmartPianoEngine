#include "GameManager.hpp"
#include "Logger.hpp"
#include <chrono>
#include <cstdlib>
#include <unistd.h>

using namespace std::chrono;

void GameManager::lancerJeu() {
    Logger::log("[GameManager] En attente des paramètres du jeu");
    std::string message = socketManager.recevoirMessage();
    Logger::log("[GameManager] Message brut reçu: {}", message);

    auto params = socketManager.traiterMessage(message);
    if (params.empty()) {
        Logger::err("[GameManager] Message invalide ou vide");
        return;
    }

    std::string jeu = params["jeu"];
    std::string gamme = params["gamme"];
    std::string mode = params["mode"];

    Logger::log("[GameManager] Paramètres reçus: Jeu={}  Gamme={}  Mode={}",
                jeu, gamme, mode);

    if (jeu == "Jeu de note") lancerJeuDeNote(gamme, mode);
    else if (jeu == "Jeu d'accord SR") lancerJeuDaccordSR(gamme, mode);
    else if (jeu == "Jeu d'accord AR")
        lancerJeuDaccordRenversement(gamme, mode);
    else Logger::err("[GameManager] Type de jeu non supporté: {}", jeu);
}

void GameManager::lancerJeuDaccordRenversement(const std::string& gamme,
                                               const std::string& mode) {
    Logger::log("[GameManager] Lancement du jeu d'accords renversés");
    if (!lectureNote.initialiser()) {
        Logger::err("[GameManager] Lecture MIDI impossible");
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
        Logger::log("[GameManager] Accord: {} {}", nomAccord, renverStr);

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
                    Logger::err("[GameManager] Accord incorrect joué");
                    socketManager.envoyerMessage(accordMessage);
                }
            } else {
                Logger::err("[GameManager] Nombre incorrect de notes");
                socketManager.envoyerMessage(accordMessage);
            }
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    std::map<std::string, std::string> finMessage = {
        {"type", "fin_du_jeu"}, {"score", std::to_string(duration)}};
    socketManager.envoyerMessage(finMessage);
    Logger::log("[GameManager] Jeu terminé, temps total: {}s", duration);

    restartProgram();
}

void GameManager::lancerJeuDaccordSR(const std::string& gamme,
                                     const std::string& mode) {
    Logger::log("[GameManager] Lancement du jeu d'accords standards");
    if (!lectureNote.initialiser()) {
        Logger::err("[GameManager] Lecture MIDI impossible");
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
        Logger::log("[GameManager] Accord envoyé: {}", nomAccord);

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
                    Logger::err("[GameManager] Accord incorrect joué");
                    socketManager.envoyerMessage(accordMessage);
                }
            } else {
                Logger::err("[GameManager] Nombre incorrect de notes");
                socketManager.envoyerMessage(accordMessage);
            }
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    std::map<std::string, std::string> finMessage = {
        {"type", "fin_du_jeu"}, {"score", std::to_string(duration)}};
    socketManager.envoyerMessage(finMessage);
    Logger::log("[GameManager] Jeu terminé, temps total: {}s", duration);

    restartProgram();
}

void GameManager::lancerJeuDeNote(const std::string& gamme,
                                  const std::string& mode) {
    Logger::log("[GameManager] Lancement du jeu de notes");
    if (!lectureNote.initialiser()) {
        Logger::err("[GameManager] Lecture MIDI impossible");
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
        Logger::log("[GameManager] Note envoyée: {}", note);
        bool noteCorrecte = false;
        while (!noteCorrecte) {
            std::vector<std::string> notesJouees = lectureNote.lireNote();
            if (notesJouees.size() == 1) {
                if (validateur.valider(notesJouees[0], note)) {
                    Logger::log("[GameManager] Notes OK: {}", notesJouees[0]);
                    notesCorrectes++;
                    noteCorrecte = true;
                } else {
                    Logger::err("[GameManager] Note incorrecte");
                    socketManager.envoyerMessage(noteMessage);
                }
            } else {
                Logger::err("[GameManager] Nombre incorrect de notes");
                socketManager.envoyerMessage(noteMessage);
            }
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    std::map<std::string, std::string> finMessage = {
        {"type", "fin_du_jeu"}, {"score", std::to_string(duration)}};

    socketManager.envoyerMessage(finMessage);
    Logger::log("[GameManager] Jeu terminé, temps total: {}s", duration);

    restartProgram();
}
