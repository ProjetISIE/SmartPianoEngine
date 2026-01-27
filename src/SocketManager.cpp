#include "SocketManager.hpp"
#include "Logger.hpp"
#include <cstring>
#include <sstream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

SocketManager::~SocketManager() {
    if (clientSocket >= 0) close(clientSocket);
    if (serverSocket >= 0) {
        close(serverSocket);
        if (!socketPath.empty()) unlink(socketPath.c_str());
    }
    Logger::log("[SocketManager] Destructeur appelé, serveur supprimé");
}

bool SocketManager::initialiserServeur() {
    unlink(socketPath.c_str()); // Supprimer socket existant

    // Créer socket Unix
    serverSocket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        Logger::err("[SocketManager] Impossible de créer le socket");
        return false;
    }

    // Configurer adresse du socket
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socketPath.c_str(), sizeof(addr.sun_path) - 1);

    // Lier socket
    if (bind(serverSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        Logger::err("[SocketManager] Impossible de lier le socket");
        close(serverSocket);
        serverSocket = -1;
        return false;
    }

    // Écouter connexions
    if (listen(serverSocket, 1) < 0) {
        Logger::err("[SocketManager] Impossible de mettre le socket en écoute");
        close(serverSocket);
        serverSocket = -1;
        return false;
    }

    Logger::log("[SocketManager] Serveur démarré sur le socket {}", socketPath);
    return true;
}

void SocketManager::attendreConnexion() {
    if (serverSocket < 0) {
        Logger::err("[SocketManager] Serveur non initialisé");
        return;
    }

    clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket < 0) {
        Logger::err("[SocketManager] Échec de l'acceptation de connexion");
        return;
    }

    Logger::log("[SocketManager] Connexion client établie");
}

void SocketManager::envoyerMessage(
    const std::map<std::string, std::string>& message) {
    if (clientSocket < 0) {
        Logger::err("[SocketManager] Aucun client connecté pour envoi");
        return;
    }

    // Convertir map en texte format "cle=valeur\n"
    std::string data;
    for (const auto& [key, value] : message) data += key + "=" + value + "\n";
    data += "\n"; // Double newline pour fin message

    ssize_t sent = send(clientSocket, data.c_str(), data.length(), 0);
    if (sent < 0) {
        Logger::err("[SocketManager] Échec de l'envoi du message");
        return;
    }

    Logger::log("[SocketManager] Message envoyé: {}", data);
}

std::string SocketManager::recevoirMessage() {
    if (clientSocket < 0) {
        Logger::err("[SocketManager] Pas de client connecté");
        return "";
    }
    char buffer[4096];
    std::string message;
    while (true) {
        ssize_t received = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (received < 0) {
            Logger::err("[SocketManager] Échec de la réception du message");
            return "";
        }
        if (received == 0) {
            Logger::err("[SocketManager] Client déconnecté");
            return "";
        }
        buffer[received] = '\0';
        message += buffer;
        // Vérifier si message complet (double newline)
        if (message.find("\n\n") != std::string::npos) break;
    }
    Logger::log("[SocketManager] Message reçu: {}", message);
    return message;
}

std::map<std::string, std::string>
SocketManager::traiterMessage(const std::string& message) {
    std::map<std::string, std::string> result;
    if (message.empty()) {
        Logger::err("[SocketManager] Message vide");
        return result;
    }
    std::istringstream stream(message);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.empty()) break; // Double newline marque fin
        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            result[key] = value;
        }
    }
    Logger::log("[SocketManager] Message texte traité avec succès");
    return result;
}
