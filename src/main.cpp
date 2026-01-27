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
    // Initialisation journaux
    Logger::init("smartpiano.log", "smartpiano.err.log");
    Logger::log("[MAIN] === Démarrage Smart Piano Engine ===");
    // Configuration gestionnaires de signaux de terminaison
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    try {
        UdsTransport transport;
        g_transport = &transport; // Garder référence pour le signal handler
        RtMidiInput midi;
        // Création du moteur de jeu
        GameEngine engine(transport, midi);
        g_engine = &engine; // Garder référence pour le signal handler
        // Démarrage du transport
        if (!transport.start()) {
            Logger::err(
                "[MAIN] ERREUR FATALE: Impossible de démarrer le transport");
            std::println("Erreur: Impossible de créer le socket {}",
                         transport.getSocketPath());
            return 1;
        }
        Logger::log("[MAIN] Transport démarré sur {}",
                    transport.getSocketPath());
        std::println("Smart Piano Engine démarré");
        std::println("Socket: {}", transport.getSocketPath());
        std::println("En attente de connexion client...");
        std::println("");
        std::println("Appuyez sur Ctrl+C pour arrêter");
        // Lancement moteur (boucle d’évènements principale)
        engine.run();
    } catch (const std::exception& e) {
        Logger::err("[MAIN] EXCEPTION NON GÉRÉE: {}", e.what());
        std::println("Erreur fatale: {}", e.what());
        return 1;
    }

    g_engine = nullptr;    // Nettoyage
    g_transport = nullptr; // Nettoyage
    Logger::log("[MAIN] === Arrêt Smart Piano Engine ===");
    std::println("");
    std::println("Smart Piano Engine arrêté");
    return 0;
}
