#include "GameManager.hpp"
#include "Logger.hpp"
#include <alsa/asoundlib.h>
#include <iostream>

int main(int argc, char* argv[]) {
    // Initialisation des chemins de logs
    Logger::init("log_basique_MDJ.txt", "log_erreurs_MDJ.txt");
    Logger::log("[MAIN] Ligne 14 : Initialisation des logs");

    // Creation de l'objet GameManager
    GameManager gameManager;
    Logger::log("[MAIN] Ligne 18 : Instance de GameManager creee");

    // Initialisation du serveur sur un socket Unix
    std::string socketPath = "/tmp/smartpiano.sock";
    if (!gameManager.initialiserServeur(socketPath)) {
        Logger::log(
            "[MAIN] Ligne 24 : Erreur : Impossible de demarrer le serveur",
            true);
        return -1; // Retourne une erreur si l'initialisation echoue
    }
    Logger::log("[MAIN] Ligne 28 : Serveur demarre sur le socket " + socketPath);

    std::cout << "Serveur demarre. En attente de connexion..." << std::endl;
    Logger::log("[MAIN] Ligne 31 : Serveur en attente de connexion");

    // Attente de la connexion d'un client
    gameManager.attendreConnexion();
    Logger::log("[MAIN] Ligne 35 : Connexion etablie avec un client");

    std::cout << "Connexion etablie. Lancement du jeu." << std::endl;
    Logger::log("[MAIN] Ligne 38 : Lancement du jeu");

    // Lancement du jeu
    gameManager.lancerJeu();
    Logger::log("[MAIN] Ligne 42 : Jeu termine");

    std::cout << "Jeu termine. Arret du serveur." << std::endl;
    Logger::log("[MAIN] Ligne 45 : Fin du programme, arret du serveur");

    return 0;
}
