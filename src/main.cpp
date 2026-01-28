#include "GameEngine.hpp"
#include "Logger.hpp"
#include "RtMidiInput.hpp"
#include "UdsTransport.hpp"
#include <csignal>
#include <print>

// Variables globales pour gestion signaux de terminaison
static GameEngine* g_engine = nullptr;
static UdsTransport* g_transport = nullptr;

/**
 * @brief Gestionnaire de signaux pour arrêts propres
 * @param signum Numéro du signal reçu
 */
void signalHandler(int signum) {
    Logger::log("[MAIN] Signal reçu: {}", signum);
    if (g_engine) g_engine->stop();
    if (g_transport) g_transport->stop();
}

int main() {
    Logger::init();
    Logger::log("[MAIN] === Démarrage Smart Piano Engine ===");
    std::println("[MAIN] Appuyer sur Ctrl+C pour arrêter Smart Piano Engine");
    // Configuration gestionnaires de signaux de terminaison
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    try {
        UdsTransport transport;
        g_transport = &transport; // Garder référence pour le signal handler
        RtMidiInput midi;
        GameEngine engine(transport, midi); // Création du moteur de jeu
        g_engine = &engine;       // Garder référence pour le signal handler
        if (!transport.start()) { // Démarrage du transport
            Logger::err(
                "[MAIN] ERREUR FATALE: Impossible de démarrer le transport");
            std::println("Erreur: Impossible de créer le socket {}",
                         transport.getSocketPath());
            return 1;
        }
        engine.run(); // Lancement moteur (boucle d’évènements principale)
    } catch (const std::exception& e) {
        Logger::err("[MAIN] EXCEPTION NON GÉRÉE: {}", e.what());
        std::println("Erreur fatale: {}", e.what());
        return 1;
    }
    g_engine = nullptr;    // Nettoyage
    g_transport = nullptr; // Nettoyage
    Logger::log("[MAIN] === Arrêt Smart Piano Engine ===");
    std::println("Smart Piano Engine arrêté");
    return 0;
}
