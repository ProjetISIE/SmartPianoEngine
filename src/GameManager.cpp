#include "GameManager.hpp"
#include "Logger.hpp"
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <chrono>
#include <cstdlib>
#include <unistd.h>

using namespace std::chrono;

// Constructeur par defaut
GameManager::GameManager() {
    Logger::log("[GameManager] ligne 17 : Initialisation de GameManager");
}

// Initialiser le serveur sur un port specifique
bool GameManager::initialiserServeur(int port) {
    Logger::log(
        "[GameManager] ligne 23 : Initialisation du serveur sur le port " +
        std::to_string(port));
    return socketManager.initialiserServeur(port);
}

// Attendre une connexion client
void GameManager::attendreConnexion() {
    Logger::log("[GameManager] ligne 30 : En attente de connexion client");
    socketManager.attendreConnexion();
}

// Fonction principale pour lancer le jeu
void GameManager::lancerJeu() {
    Logger::log("[GameManager] ligne 37 : En attente des parametres du jeu...");
    QString message = socketManager.recevoirMessage();
    Logger::log("[GameManager] ligne 39 : Message brut recu : " +
                message.toStdString());

    QJsonDocument jsonDoc = QJsonDocument::fromJson(message.toUtf8());
    if (jsonDoc.isNull()) {
        Logger::log(
            "[GameManager] ligne 44 : Erreur - Document JSON invalide ou vide",
            true);
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();
    Logger::log(
        "[GameManager] ligne 49 : JSON deserialise : " +
        QJsonDocument(jsonObj).toJson(QJsonDocument::Compact).toStdString());

    std::string jeu = jsonObj.value("jeu").toString().toStdString();
    std::string gamme = jsonObj.value("gamme").toString().toStdString();
    std::string mode = jsonObj.value("mode").toString().toStdString();
    // bool jouerSon = jsonObj.value("jouerSon").toBool();

    Logger::log("[GameManager] ligne 57 : Parametres recus - Jeu : " + jeu +
                ", Gamme : " + gamme + ", Mode : " + mode);

    if (jeu == "Jeu de note") {
        lancerJeuDeNote(gamme, mode);
    } else if (jeu == "Jeu d'accord SR") {
        lancerJeuDaccordSR(gamme, mode);
    } else if (jeu == "Jeu d'accord AR") {
        lancerJeuDaccordRenversement(gamme, mode);
    } else {
        Logger::log(
            "[GameManager] ligne 75 : Type de jeu non supporte : " + jeu, true);
    }
}

// Jeu d'accords avec renversements
void GameManager::lancerJeuDaccordRenversement(const std::string& gamme,
                                               const std::string& mode) {
    Logger::log("[GameManager] ligne 83 : Lancement du jeu d'accords avec "
                "renversements");
    if (!lectureNote.initialiser()) {
        Logger::log("[GameManager] ligne 86 : Erreur - Impossible "
                    "d'initialiser la lecture MIDI",
                    true);
        return;
    }

    auto start = high_resolution_clock::now();

    int accordsCorrects = 0;
    const int maxAccords = 8;

    while (accordsCorrects < maxAccords) {
        auto [nomAccord, notes, renversement] =
            generateur.genererAccordRenversement(gamme, mode);

        std::string renversementStr = std::to_string(renversement - 1);
        QJsonObject accordMessage = {
            {"type", "accord_a_jouer"},
            {"nom_accord",
             QString::fromStdString(nomAccord + " " + renversementStr)}};

        socketManager.envoyerMessage(accordMessage);
        Logger::log("[GameManager] ligne 105 : Accord envoye : " + nomAccord +
                    " avec renversement " + renversementStr);

        bool accordCorrect = false;
        while (!accordCorrect) {
            std::vector<std::string> accordJoue = lectureNote.lireNote();

            if (accordJoue.size() == 3) {
                if (validateur.validerAccordRenversement(accordJoue, notes,
                                                         renversement)) {
                    Logger::log(
                        "[GameManager] ligne 116 : Accord correct joue");
                    accordsCorrects++;
                    accordCorrect = true;
                } else {
                    Logger::log("[GameManager] ligne 122 : Accord incorrect",
                                true);
                    socketManager.envoyerMessage(accordMessage);
                }
            } else {
                Logger::log("[GameManager] ligne 128 : Nombre incorrect de "
                            "notes detectees",
                            true);
                socketManager.envoyerMessage(accordMessage);
            }
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    QJsonObject finMessage = {{"type", "fin_du_jeu"},
                              {"score", QString::number(duration)}};

    socketManager.envoyerMessage(finMessage);
    Logger::log("[GameManager] ligne 142 : Jeu termine. Temps total : " +
                std::to_string(duration) + " secondes");

    restartProgram();
}

// Jeu d'accords standards
void GameManager::lancerJeuDaccordSR(const std::string& gamme,
                                     const std::string& mode) {
    Logger::log(
        "[GameManager] ligne 151 : Lancement du jeu d'accords standards");
    if (!lectureNote.initialiser()) {
        Logger::log("[GameManager] ligne 154 : Erreur - Impossible "
                    "d'initialiser la lecture MIDI",
                    true);
        return;
    }

    auto start = high_resolution_clock::now();

    int accordsCorrects = 0;
    const int maxAccords = 8;

    while (accordsCorrects < maxAccords) {
        auto [nomAccord, notes] = generateur.genererAccord(gamme, mode);

        QJsonObject accordMessage = {
            {"type", "accord_a_jouer"},
            {"nom_accord", QString::fromStdString(nomAccord)}};

        socketManager.envoyerMessage(accordMessage);
        Logger::log("[GameManager] ligne 172 : Accord envoye : " + nomAccord);

        bool accordCorrect = false;
        while (!accordCorrect) {
            std::vector<std::string> accordJoue = lectureNote.lireNote();

            if (accordJoue.size() == 3) {
                if (validateur.validerAccordSR(
                        accordJoue, {notes[0], notes[1], notes[2]})) {
                    Logger::log(
                        "[GameManager] ligne 183 : Accord correct joue");
                    accordsCorrects++;
                    accordCorrect = true;
                } else {
                    Logger::log("[GameManager] ligne 189 : Accord incorrect",
                                true);
                    socketManager.envoyerMessage(accordMessage);
                }
            } else {
                Logger::log("[GameManager] ligne 195 : Nombre incorrect de "
                            "notes detectees",
                            true);
                socketManager.envoyerMessage(accordMessage);
            }
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    QJsonObject finMessage = {{"type", "fin_du_jeu"},
                              {"score", QString::number(duration)}};

    socketManager.envoyerMessage(finMessage);
    Logger::log("[GameManager] ligne 209 : Jeu termine. Temps total : " +
                std::to_string(duration) + " secondes");

    restartProgram();
}

// Jeu de notes
void GameManager::lancerJeuDeNote(const std::string& gamme,
                                  const std::string& mode) {
    Logger::log("[GameManager] ligne 217 : Lancement du jeu de notes");
    if (!lectureNote.initialiser()) {
        Logger::log("[GameManager] ligne 220 : Erreur - Impossible "
                    "d'initialiser la lecture MIDI",
                    true);
        return;
    }

    auto start = high_resolution_clock::now();

    int notesCorrectes = 0;
    const int maxNotes = 10;

    while (notesCorrectes < maxNotes) {
        std::string note = generateur.generer(gamme, mode);

        QJsonObject noteMessage = {{"type", "note_a_jouer"},
                                   {"note", QString::fromStdString(note)}};

        socketManager.envoyerMessage(noteMessage);
        Logger::log("[GameManager] ligne 238 : Note envoyee : " + note);

        bool noteCorrecte = false;
        while (!noteCorrecte) {
            std::vector<std::string> notesJouees = lectureNote.lireNote();

            if (notesJouees.size() == 1) {
                if (validateur.valider(notesJouees[0], note)) {
                    Logger::log(
                        "[GameManager] ligne 249 : Note correcte joue : " +
                        notesJouees[0]);
                    notesCorrectes++;
                    noteCorrecte = true;
                } else {
                    Logger::log("[GameManager] ligne 255 : Note incorrecte",
                                true);
                    socketManager.envoyerMessage(noteMessage);
                }
            } else {
                Logger::log("[GameManager] ligne 261 : Nombre incorrect de "
                            "notes detectees",
                            true);
                socketManager.envoyerMessage(noteMessage);
            }
        }
    }

    auto end = high_resolution_clock::now();
    auto duration = duration_cast<seconds>(end - start).count();

    QJsonObject finMessage = {{"type", "fin_du_jeu"},
                              {"score", QString::number(duration)}};

    socketManager.envoyerMessage(finMessage);
    Logger::log("[GameManager] ligne 275 : Jeu termine. Temps total : " +
                std::to_string(duration) + " secondes");

    restartProgram();
}

// Redemarrage du programme
void GameManager::restartProgram() {
    Logger::log("[GameManager] ligne 283 : Redemarrage du programme");
    const char* program = "/home/vivien/Desktop/PRI/pianotrainer/"
                          "PianoTrainerMDJV1/PianoTrainerMDJV1";
    const char* args[] = {program, nullptr};

    execvp(program, const_cast<char* const*>(args));

    Logger::log("[GameManager] ligne 289 : Echec du redemarrage", true);
    std::exit(EXIT_FAILURE);
}
