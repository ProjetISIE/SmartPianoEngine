#include "GameManager.hpp"
#include "Logger.hpp"
#include <alsa/asoundlib.h>
#include <print>

int main() {
    // Initialisation des fichiers de logs
    Logger::init("smartpiano.log", "smartpiano.err.log");
    Logger::log("[MAIN] Initialisation des logs");

    // Création de l'objet GameManager
    GameManager gameManager;
    Logger::log("[MAIN] Instance de GameManager créée");

    // Initialisation du serveur sur un socket Unix
    std::string socketPath = "/tmp/smartpiano.sock";
    if (!gameManager.initialiserServeur(socketPath)) {
        Logger::log("[MAIN] Erreur: Impossible de démarrer le serveur", true);
        return -1; // Retourne une erreur si l'initialisation échoue
    }
    Logger::log("[MAIN] Serveur démarré sur le socket " + socketPath);
    std::println("Serveur démarré, en attente de connexion...");
    Logger::log("[MAIN] Serveur en attente de connexion");

    // Attente de la connexion d'un client
    gameManager.attendreConnexion();
    Logger::log("[MAIN] Connexion établie avec un client");
    std::println("Connexion établie: lancement du jeu");
    Logger::log("[MAIN] Lancement du jeu");

    // Lancement du jeu
    gameManager.lancerJeu();

    // Arrêt du jeu
    Logger::log("[MAIN] Jeu terminé");
    std::println("Jeu terminé, arrêt du serveur Smart Piano");
    Logger::log("[MAIN] Fin du programme, arrêt du serveur");
    return 0;
}
