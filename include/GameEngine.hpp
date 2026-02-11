#ifndef GAMEENGINE_HPP
#define GAMEENGINE_HPP

#include "IGameMode.hpp"
#include "IMidiInput.hpp"
#include "ITransport.hpp"
#include "Logger.hpp"
#include <memory>
#include <stop_token>

/**
 * @brief Moteur de jeu principal
 * Orchestrateur qui coordonne les composants et gère le cycle de vie
 */
class GameEngine {
  private:
    ITransport& transport;                  ///< Référence au transport
    IMidiInput& midi;                       ///< Référence à l'entrée MIDI
    std::unique_ptr<IGameMode> currentGame; ///< Mode de jeu actuel
    bool running{false};                    ///< État du moteur
    std::stop_source gameStopSource;        ///< Source d'arrêt pour le jeu

  private:
    /**
     * @brief Gère la connexion d'un client
     */
    void handleClientConnection();

    /**
     * @brief Traite une session de jeu
     * @param config Configuration du jeu
     */
    void processGameSession(const GameConfig& config);

    /**
     * @brief Crée un mode de jeu selon la configuration
     * @param config Configuration du jeu
     * @return Mode de jeu créé
     */
    std::unique_ptr<IGameMode> createGameMode(const GameConfig& config);

    /**
     * @brief Parse une configuration depuis un message
     * @param msg Message reçu
     * @return Configuration parsée
     */
    GameConfig parseConfig(const Message& msg);

    /**
     * @brief Envoie un accusé de réception
     * @param ok true si configuration OK, false sinon
     * @param errorCode Code d'erreur éventuel
     * @param errorMessage Message d'erreur éventuel
     */
    void sendAck(bool ok, const std::string& errorCode = "",
                 const std::string& errorMessage = "");

  public:
    GameEngine(ITransport& transport, IMidiInput& midi)
        : transport(transport), midi(midi) {
        Logger::log("[GameEngine] Instance créée");
    }

    ~GameEngine() {
        stop();
        Logger::log("[GameEngine] Instance détruite");
    }

    /**
     * @brief Lance le moteur de jeu
     */
    void run();

    /**
     * @brief Arrête le moteur de jeu
     */
    void stop();
};

#endif // GAMEENGINE_HPP
