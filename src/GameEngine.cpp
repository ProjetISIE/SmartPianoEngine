#include "GameEngine.hpp"
#include "ChordGame.hpp"
#include "Logger.hpp"
#include "NoteGame.hpp"

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
            Logger::log("[GameEngine] Exception: " + std::string(e.what()),
                        true);
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

            // Initialiser MIDI si nécessaire (erreur non fatale)
            if (!midi_.isReady()) {
                if (!midi_.initialize()) {
                    Logger::log(
                        "[GameEngine] MIDI non disponible, en attente...",
                        true);
                    // Envoyer une erreur mais continuer
                    Message error("error");
                    error.addField("code", "midi");
                    error.addField("message",
                                   "Périphérique MIDI non disponible");
                    transport_.send(error);
                    // Ne pas envoyer ack - attendre que MIDI soit prêt
                    continue;
                }
            }

            sendAck(true);
            processGameSession(config);

        } else if (msg.type == "quit") {
            Logger::log(
                "[GameEngine] Client demande retour à l'état non configuré");
            // quit ne déconnecte pas, il reset juste la config
            // Continue la boucle pour attendre une nouvelle config
            continue;

        } else {
            Logger::log("[GameEngine] Message inattendu: " + msg.type, true);
            Message error("error");
            error.addField("code", "state");
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
        error.addField("code", "internal");
        error.addField("message", "Mode de jeu non supporté");
        transport_.send(error);
        return;
    }

    bool sessionActive = true;
    while (sessionActive) {
        // Attendre que le client soit prêt (ou quit)
        Message readyMsg = transport_.receive();

        if (readyMsg.type == "quit") {
            Logger::log("[GameEngine] Client demande arrêt de session");
            currentGame_->stop();
            return; // Retour à l'état CONFIGURED
        }

        if (readyMsg.type != "ready") {
            Logger::log("[GameEngine] Erreur: ready attendu, reçu " +
                            readyMsg.type,
                        true);
            Message error("error");
            error.addField("code", "state");
            error.addField("message", "Message 'ready' ou 'quit' attendu");
            transport_.send(error);
            continue;
        }

        // Lancer la partie
        currentGame_->start();
        GameResult result = currentGame_->play();

        // Envoyer le résultat final (sans score)
        Message overMsg("over");
        overMsg.addField("duration", std::to_string(result.duration));
        overMsg.addField("perfect", std::to_string(result.perfect));
        if (result.partial > 0) {
            overMsg.addField("partial", std::to_string(result.partial));
        }
        overMsg.addField("total", std::to_string(result.total));
        transport_.send(overMsg);

        Logger::log("[GameEngine] Session terminée");
        sessionActive = false; // Une seule partie puis retour à CONFIGURED
    }
}

std::unique_ptr<IGameMode>
GameEngine::createGameMode(const GameConfig& config) {
    if (config.gameType == "note") {
        return std::make_unique<NoteGame>(transport_, midi_, config);
    } else if (config.gameType == "chord") {
        return std::make_unique<ChordGame>(transport_, midi_, config, false);
    } else if (config.gameType == "inversed") {
        return std::make_unique<ChordGame>(transport_, midi_, config, true);
    }

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
