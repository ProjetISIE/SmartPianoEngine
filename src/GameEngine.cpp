#include "GameEngine.hpp"
#include "ChordGame.hpp"
#include "Logger.hpp"
#include "NoteGame.hpp"

void GameEngine::run() {
    this->running = true;
    Logger::log("[GameEngine] Démarrage du moteur");
    while (this->running) try {
            handleClientConnection();
        } catch (const std::exception& e) {
            Logger::err("[GameEngine] Exception: {}", e.what());
        }
    Logger::log("[GameEngine] Moteur arrêté");
}

void GameEngine::stop() {
    this->running = false;
    this->currentGame.reset();
}

void GameEngine::handleClientConnection() {
    Logger::log("[GameEngine] En attente de connexion client");
    this->transport.waitForClient();
    while (this->transport.isClientConnected()) {
        // Attendre un message de configuration
        Message msg = this->transport.receive();
        if (msg.getType() == "config") {
            GameConfig config = parseConfig(msg);
            if (config.gameType.empty()) {
                sendAck(false, "game", "Type de jeu manquant");
                continue; // Recommencer si config invalide
            }
            // Initialiser MIDI si nécessaire (erreur non fatale)
            if (!this->midi.isReady() && !this->midi.initialize()) {
                Logger::err("[GameEngine] MIDI non disponible, en attente...");
                // Envoyer une erreur mais continuer
                Message error(
                    "error", {{"code", "midi"},
                              {"message", "Périphérique MIDI non disponible"}});
                this->transport.send(error);
                continue; // Ne pas acquiter si MIDI pas prêt, recommencer
            }
            sendAck(true);
            processGameSession(config);
        } else if (msg.getType() == "quit") {
            Logger::log(
                "[GameEngine] Client demande retour à l'état non configuré");
            continue; // Ne déconnecte pas, réinitialise juste la configuration
        } else {
            Logger::err("[GameEngine] Message inattendu: {}", msg.getType());
            Message error("error",
                          {{"code", "state"},
                           {"message", "Message inattendu: " + msg.getType()}});
            this->transport.send(error);
        }
    }
    Logger::log("[GameEngine] Client déconnecté");
}

void GameEngine::processGameSession(const GameConfig& config) {
    Logger::log("[GameEngine] Démarrage session: {}", config.gameType);
    // Créer le mode de jeu approprié
    this->currentGame = createGameMode(config);
    if (!this->currentGame) {
        Message error("error", {{"code", "internal"},
                                {"message", "Mode de jeu non supporté"}});
        this->transport.send(error);
        return;
    }
    bool sessionActive = true;
    while (sessionActive) {
        // Attendre que le client soit prêt (ou quit)
        Message readyMsg = this->transport.receive();
        if (readyMsg.getType() == "quit") {
            Logger::log("[GameEngine] Client demande arrêt de session");
            this->currentGame->stop();
            return; // Retour à l'état CONFIGURED
        }
        if (readyMsg.getType() != "ready") {
            Logger::err("[GameEngine] Erreur: ready attendu, reçu {}",
                        readyMsg.getType());
            Message error("error",
                          {{"code", "state"},
                           {"message", "Message 'ready' ou 'quit' attendu"}});
            this->transport.send(error);
            continue;
        }
        // Lancer la partie
        this->currentGame->start();
        GameResult result = this->currentGame->play();
        // Envoyer le résultat final (sans score)
        std::map<std::string, std::string> overFields{
            {"duration", std::to_string(result.duration)},
            {"perfect", std::to_string(result.perfect)},
            {"total", std::to_string(result.total)}};
        if (result.partial > 0)
            overFields["partial"] = std::to_string(result.partial);
        Message overMsg("over", overFields);
        this->transport.send(overMsg);
        Logger::log("[GameEngine] Session terminée");
        sessionActive = false; // Une seule partie puis retour à CONFIGURED
    }
}

std::unique_ptr<IGameMode>
GameEngine::createGameMode(const GameConfig& config) {
    if (config.gameType == "note")
        return std::make_unique<NoteGame>(this->transport, this->midi, config);
    else if (config.gameType == "chord")
        return std::make_unique<ChordGame>(this->transport, this->midi, config,
                                           false);
    else if (config.gameType == "inversed")
        return std::make_unique<ChordGame>(this->transport, this->midi, config,
                                           true);
    Logger::err("[GameEngine] Mode de jeu inconnu: {}", config.gameType);
    return nullptr;
}

GameConfig GameEngine::parseConfig(const Message& msg) {
    GameConfig config;
    config.gameType = msg.getField("game");
    config.scale = msg.getField("scale");
    config.mode = msg.getField("mode");
    Logger::log("[GameEngine] Config: game={} scale={} mode={}",
                config.gameType, config.scale, config.mode);
    return config;
}

void GameEngine::sendAck(bool ok, const std::string& errorCode,
                         const std::string& errorMessage) {
    std::map<std::string, std::string> ackFields{
        {"status", ok ? "ok" : "error"}};
    if (!ok) {
        if (!errorCode.empty()) ackFields["code"] = errorCode;
        if (!errorMessage.empty()) ackFields["message"] = errorMessage;
    }
    Message ack("ack", ackFields);
    this->transport.send(ack);
}
