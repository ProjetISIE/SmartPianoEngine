#include "GameEngine.hpp"
#include "UdsTransport.hpp"
#include "RtMidiInput.hpp"
#include "Logger.hpp"
#include <print>
#include <csignal>

// Variables globales pour gestion des signaux
static GameEngine* g_engine = nullptr;
static UdsTransport* g_transport = nullptr;

/**
 * @brief Gestionnaire de signaux pour arrêt propre
 */
void signalHandler(int signum) {
    Logger::log("[MAIN] Signal reçu: " + std::to_string(signum));
    
    if (g_engine) {
        g_engine->stop();
    }
    
    if (g_transport) {
        g_transport->stop();
    }
}

int main() {
    // Initialisation des fichiers de logs
    Logger::init("smartpiano.log", "smartpiano.err.log");
    Logger::log("[MAIN] === Démarrage Smart Piano Engine ===");
    Logger::log("[MAIN] Architecture: C++23 pure, UDS, Protocol v1.0.0");

    // Configuration des gestionnaires de signaux
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    try {
        // Création des composants d'infrastructure
        UdsTransport transport;
        g_transport = &transport;
        
        RtMidiInput midi;

        // Création du moteur de jeu
        GameEngine engine(transport, midi);
        g_engine = &engine;

        // Démarrage du transport
        std::string socketPath = "/tmp/smartpiano.sock";
        if (!transport.start(socketPath)) {
            Logger::log("[MAIN] ERREUR FATALE: Impossible de démarrer le transport", true);
            std::println("Erreur: Impossible de créer le socket {}", socketPath);
            return 1;
        }

        Logger::log("[MAIN] Transport démarré sur " + socketPath);
        std::println("Smart Piano Engine démarré");
        std::println("Socket: {}", socketPath);
        std::println("En attente de connexion client...");
        std::println("");
        std::println("Appuyez sur Ctrl+C pour arrêter");

        // Lancement du moteur (boucle principale)
        engine.run();

    } catch (const std::exception& e) {
        Logger::log("[MAIN] EXCEPTION NON GÉRÉE: " + std::string(e.what()), true);
        std::println("Erreur fatale: {}", e.what());
        return 1;
    }

    // Nettoyage
    g_engine = nullptr;
    g_transport = nullptr;

    Logger::log("[MAIN] === Arrêt Smart Piano Engine ===");
    std::println("");
    std::println("Smart Piano Engine arrêté");

    return 0;
}
