#include "GameManager.hpp"
#include "Logger.hpp"
#include <QCoreApplication>
#include <QLoggingCategory>
#include <alsa/asoundlib.h>
#include <iostream>

int main(int argc, char* argv[]) {
    // Initialisation des chemins de logs
    Logger::init("log_basique_MDJ.txt", "log_erreurs_MDJ.txt");
    Logger::log("[MAIN] Ligne 14 : Initialisation des logs");

    // Configuration des logs pour Qt Multimedia
    QLoggingCategory::setFilterRules(QStringLiteral("qt.multimedia.*=true"));
    Logger::log("[MAIN] Ligne 18 : Configuration des logs Qt Multimedia");

    // Initialisation de l'application Qt
    QCoreApplication app(argc, argv);
    Logger::log("[MAIN] Ligne 22 : Application Qt initialisee");

    // Creation de l'objet GameManager
    GameManager gameManager;
    Logger::log("[MAIN] Ligne 26 : Instance de GameManager creee");

    // Initialisation du serveur sur le port 8080
    if (!gameManager.initialiserServeur(8080)) {
        Logger::log(
            "[MAIN] Ligne 31 : Erreur : Impossible de demarrer le serveur",
            true);
        return -1; // Retourne une erreur si l'initialisation echoue
    }
    Logger::log("[MAIN] Ligne 34 : Serveur demarre sur le port 8080");

    std::cout << "Serveur demarre. En attente de connexion..." << std::endl;
    Logger::log("[MAIN] Ligne 37 : Serveur en attente de connexion");

    // Attente de la connexion d'un client
    gameManager.attendreConnexion();
    Logger::log("[MAIN] Ligne 41 : Connexion etablie avec un client");

    std::cout << "Connexion etablie. Lancement du jeu." << std::endl;
    Logger::log("[MAIN] Ligne 44 : Lancement du jeu");

    // Lancement du jeu
    gameManager.lancerJeu();
    Logger::log("[MAIN] Ligne 48 : Jeu termine");

    std::cout << "Jeu termine. Arret du serveur." << std::endl;
    Logger::log("[MAIN] Ligne 51 : Fin du programme, arret du serveur");

    return 0;
}
