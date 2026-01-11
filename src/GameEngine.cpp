#include "GameEngine.hpp"
#include "NoteGame.hpp"
#include "Logger.hpp"

GameEngine::GameEngine(ITransport& transport, IMidiInput& midi)
    : transport_(transport), midi_(midi), running_(false) {
    Logger::log("[GameEngine] Instance créée");
}

GameEngine::~GameEngine() {
    stop();
    Logger::log("[GameEngine] Instance détruite");
}

void GameEngine::run() {
    running_ = true;
    Logger::log("[GameEngine] Démarrage du moteur");

    while (running_) {
        try {
            handleClientConnection();
        } catch (const std::exception& e) {
            Logger::log("[GameEngine] Exception: " + std::string(e.what()), true);
        }
    }

    Logger::log("[GameEngine] Moteur arrêté");
}

void GameEngine::stop() {
    running_ = false;
    currentGame_.reset();
}

void GameEngine::handleClientConnection() {
    Logger::log("[GameEngine] En attente de connexion client");
    transport_.waitForClient();
    Logger::log("[GameEngine] Client connecté");

    while (transport_.isClientConnected()) {
        // Attendre un message de configuration
        Message msg = transport_.receive();

        if (msg.type == "config") {
            GameConfig config = parseConfig(msg);
            
            // Valider la configuration
            if (config.gameType.empty()) {
                sendAck(false, "game", "Type de jeu manquant");
                continue;
            }

            // Initialiser MIDI si nécessaire
            if (!midi_.isReady()) {
                if (!midi_.initialize()) {
                    sendAck(false, "midi", "Périphérique MIDI non disponible");
                    continue;
                }
            }

            sendAck(true);
            processGameSession(config);

        } else if (msg.type == "quit") {
            Logger::log("[GameEngine] Client demande déconnexion");
            break;

        } else {
            Logger::log("[GameEngine] Message inattendu: " + msg.type, true);
            Message error("error");
            error.addField("code", "protocol");
            error.addField("message", "Message inattendu: " + msg.type);
            transport_.send(error);
        }
    }

    Logger::log("[GameEngine] Client déconnecté");
}

void GameEngine::processGameSession(const GameConfig& config) {
    Logger::log("[GameEngine] Démarrage session: " + config.gameType);

    // Créer le mode de jeu approprié
    currentGame_ = createGameMode(config);
    if (!currentGame_) {
        Message error("error");
        error.addField("code", "game");
        error.addField("message", "Mode de jeu non supporté");
        transport_.send(error);
        return;
    }

    // Attendre que le client soit prêt
    Message readyMsg = transport_.receive();
    if (readyMsg.type != "ready") {
        Logger::log("[GameEngine] Erreur: ready attendu, reçu " + readyMsg.type, true);
        return;
    }

    // Lancer la partie
    currentGame_->start();
    GameResult result = currentGame_->play();

    // Envoyer le résultat final
    Message overMsg("over");
    overMsg.addField("score", std::to_string(result.score));
    overMsg.addField("duration", std::to_string(result.duration));
    overMsg.addField("perfect", std::to_string(result.perfect));
    overMsg.addField("total", std::to_string(result.total));
    transport_.send(overMsg);

    Logger::log("[GameEngine] Session terminée");
}

std::unique_ptr<IGameMode> GameEngine::createGameMode(const GameConfig& config) {
    if (config.gameType == "note") {
        return std::make_unique<NoteGame>(transport_, midi_, config);
    }
    // TODO: Ajouter chord et inversed
    
    Logger::log("[GameEngine] Mode de jeu inconnu: " + config.gameType, true);
    return nullptr;
}

GameConfig GameEngine::parseConfig(const Message& msg) {
    GameConfig config;
    config.gameType = msg.getField("game");
    config.scale = msg.getField("scale");
    config.mode = msg.getField("mode");
    
    // Valeurs par défaut
    if (config.scale.empty()) config.scale = "c";
    if (config.mode.empty()) config.mode = "maj";
    config.maxChallenges = 10;

    Logger::log("[GameEngine] Config: game=" + config.gameType + 
                " scale=" + config.scale + " mode=" + config.mode);
    
    return config;
}

void GameEngine::sendAck(bool ok, const std::string& errorCode, 
                         const std::string& errorMessage) {
    Message ack("ack");
    ack.addField("status", ok ? "ok" : "error");
    
    if (!ok) {
        if (!errorCode.empty()) {
            ack.addField("code", errorCode);
        }
        if (!errorMessage.empty()) {
            ack.addField("message", errorMessage);
        }
    }
    
    transport_.send(ack);
}
