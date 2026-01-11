#include "UdsTransport.hpp"
#include "Logger.hpp"
#include <cstring>
#include <sstream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

UdsTransport::UdsTransport()
    : serverSocket_(-1), clientSocket_(-1), socketPath_("") {
    Logger::log("[UdsTransport] Instance créée");
}

UdsTransport::~UdsTransport() {
    stop();
    Logger::log("[UdsTransport] Instance détruite");
}

bool UdsTransport::start(const std::string& endpoint) {
    socketPath_ = endpoint;

    // Supprimer le socket existant s'il existe
    unlink(socketPath_.c_str());

    // Créer le socket Unix
    serverSocket_ = socket(AF_UNIX, SOCK_STREAM, 0);
    if (serverSocket_ < 0) {
        Logger::log("[UdsTransport] Erreur: Impossible de créer le socket", true);
        return false;
    }

    // Configurer l'adresse du socket
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socketPath_.c_str(), sizeof(addr.sun_path) - 1);

    // Lier le socket
    if (bind(serverSocket_, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        Logger::log("[UdsTransport] Erreur: Impossible de lier le socket", true);
        close(serverSocket_);
        serverSocket_ = -1;
        return false;
    }

    // Écouter les connexions
    if (listen(serverSocket_, 1) < 0) {
        Logger::log("[UdsTransport] Erreur: Impossible de mettre le socket en écoute", true);
        close(serverSocket_);
        serverSocket_ = -1;
        return false;
    }

    Logger::log("[UdsTransport] Serveur démarré sur " + socketPath_);
    return true;
}

void UdsTransport::waitForClient() {
    if (serverSocket_ < 0) {
        Logger::log("[UdsTransport] Erreur: Serveur non initialisé", true);
        return;
    }

    clientSocket_ = accept(serverSocket_, nullptr, nullptr);
    if (clientSocket_ < 0) {
        Logger::log("[UdsTransport] Erreur: Échec de l'acceptation de connexion", true);
        return;
    }

    Logger::log("[UdsTransport] Client connecté");
}

void UdsTransport::send(const Message& msg) {
    if (clientSocket_ < 0) {
        Logger::log("[UdsTransport] Erreur: Aucun client connecté", true);
        return;
    }

    std::string data = serializeMessage(msg);
    ssize_t sent = ::send(clientSocket_, data.c_str(), data.length(), 0);
    
    if (sent < 0) {
        Logger::log("[UdsTransport] Erreur: Échec de l'envoi du message", true);
        return;
    }

    Logger::log("[UdsTransport] Message envoyé: type=" + msg.type);
}

Message UdsTransport::receive() {
    if (clientSocket_ < 0) {
        Logger::log("[UdsTransport] Erreur: Aucun client connecté", true);
        return Message("error");
    }

    char buffer[4096];
    std::string data;

    // Lire jusqu'à trouver double newline
    while (true) {
        ssize_t received = recv(clientSocket_, buffer, sizeof(buffer) - 1, 0);
        
        if (received < 0) {
            Logger::log("[UdsTransport] Erreur: Échec de réception", true);
            return Message("error");
        }

        if (received == 0) {
            Logger::log("[UdsTransport] Client déconnecté", true);
            close(clientSocket_);
            clientSocket_ = -1;
            return Message("error");
        }

        buffer[received] = '\0';
        data += buffer;

        // Vérifier si message complet (double newline)
        if (data.find("\n\n") != std::string::npos) {
            break;
        }
    }

    Logger::log("[UdsTransport] Message reçu");
    return parseMessage(data);
}

void UdsTransport::stop() {
    if (clientSocket_ >= 0) {
        close(clientSocket_);
        clientSocket_ = -1;
    }
    
    if (serverSocket_ >= 0) {
        close(serverSocket_);
        serverSocket_ = -1;
        
        if (!socketPath_.empty()) {
            unlink(socketPath_.c_str());
        }
    }

    Logger::log("[UdsTransport] Serveur arrêté");
}

bool UdsTransport::isClientConnected() const {
    return clientSocket_ >= 0;
}

std::string UdsTransport::serializeMessage(const Message& msg) const {
    std::string result = msg.type + "\n";

    for (const auto& [key, value] : msg.fields) {
        result += key + "=" + value + "\n";
    }

    result += "\n"; // Double newline pour terminer
    return result;
}

Message UdsTransport::parseMessage(const std::string& data) const {
    std::istringstream stream(data);
    std::string line;

    // Première ligne = type du message
    if (!std::getline(stream, line) || line.empty()) {
        Logger::log("[UdsTransport] Erreur: Type de message manquant", true);
        return Message("error");
    }

    // Enlever le \r si présent (pour compatibilité Windows)
    if (!line.empty() && line.back() == '\r') {
        line.pop_back();
    }

    Message msg(line);

    // Lignes suivantes = champs clé=valeur
    while (std::getline(stream, line)) {
        // Enlever le \r si présent
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        // Ligne vide = fin du message
        if (line.empty()) {
            break;
        }

        // Parser clé=valeur
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            msg.addField(key, value);
        }
    }

    return msg;
}
