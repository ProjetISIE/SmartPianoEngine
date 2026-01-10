#include "SocketManager.hpp"
#include "Logger.hpp"
#include <cstring>
#include <sstream>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

// Constructeur de la classe SocketManager
SocketManager::SocketManager()
    : serverSocket(-1), clientSocket(-1), socketPath("") {
    Logger::log("[SocketManager] Ligne 10 : Constructeur initialise.");
}

// Destructeur de la classe SocketManager
SocketManager::~SocketManager() {
    if (clientSocket >= 0) {
        close(clientSocket);
    }
    if (serverSocket >= 0) {
        close(serverSocket);
        if (!socketPath.empty()) {
            unlink(socketPath.c_str());
        }
    }
    Logger::log(
        "[SocketManager] Ligne 17 : Destructeur appele, serveur supprime.");
}

// Initialisation du serveur sur un socket Unix
bool SocketManager::initialiserServeur(const std::string& socketPath) {
    this->socketPath = socketPath;

    // Supprimer le socket existant s'il existe
    unlink(socketPath.c_str());

    // Creer le socket Unix
    serverSocket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        Logger::log("[SocketManager] Ligne 31 : Erreur : Impossible de "
                    "creer le socket.",
                    true);
        return false;
    }

    // Configurer l'adresse du socket
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socketPath.c_str(), sizeof(addr.sun_path) - 1);

    // Lier le socket
    if (bind(serverSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        Logger::log("[SocketManager] Ligne 46 : Erreur : Impossible de "
                    "lier le socket.",
                    true);
        close(serverSocket);
        serverSocket = -1;
        return false;
    }

    // Ecouter les connexions
    if (listen(serverSocket, 1) < 0) {
        Logger::log("[SocketManager] Ligne 56 : Erreur : Impossible de "
                    "mettre le socket en ecoute.",
                    true);
        close(serverSocket);
        serverSocket = -1;
        return false;
    }

    Logger::log("[SocketManager] Ligne 63 : Serveur demarre sur le socket " +
                socketPath + ".");
    return true;
}

// Attente de la connexion d'un client
void SocketManager::attendreConnexion() {
    if (serverSocket < 0) {
        Logger::log("[SocketManager] Ligne 71 : Erreur : Serveur non "
                    "initialise.",
                    true);
        return;
    }

    clientSocket = accept(serverSocket, nullptr, nullptr);
    if (clientSocket < 0) {
        Logger::log("[SocketManager] Ligne 79 : Erreur : Echec de "
                    "l'acceptation de la connexion.",
                    true);
        return;
    }

    Logger::log("[SocketManager] Ligne 85 : Connexion client etablie.");
}

// Envoi d'un message texte au client
void SocketManager::envoyerMessage(
    const std::map<std::string, std::string>& message) {
    if (clientSocket < 0) {
        Logger::log("[SocketManager] Ligne 93 : Erreur : Aucun client connecte "
                    "pour envoyer le message.",
                    true);
        return;
    }

    // Convertir la map en texte brut format "cle=valeur\n"
    std::string data;
    for (const auto& [key, value] : message) {
        data += key + "=" + value + "\n";
    }
    data += "\n"; // Double newline pour marquer la fin du message

    ssize_t sent = send(clientSocket, data.c_str(), data.length(), 0);
    if (sent < 0) {
        Logger::log(
            "[SocketManager] Ligne 110 : Erreur : Echec de l'envoi du message.",
            true);
        return;
    }

    Logger::log("[SocketManager] Ligne 115 : Message envoye : " + data);
}

// Reception d'un message texte depuis le client
std::string SocketManager::recevoirMessage() {
    if (clientSocket < 0) {
        Logger::log(
            "[SocketManager] Ligne 122 : Erreur : Pas de client connecte.",
            true);
        return "";
    }

    char buffer[4096];
    std::string message;

    while (true) {
        ssize_t received = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (received < 0) {
            Logger::log("[SocketManager] Ligne 134 : Erreur : Echec de la "
                        "reception du message.",
                        true);
            return "";
        }

        if (received == 0) {
            Logger::log("[SocketManager] Ligne 141 : Le client est deconnecte.",
                        true);
            return "";
        }

        buffer[received] = '\0';
        message += buffer;

        // Verifier si le message est complet (se termine par double newline)
        if (message.find("\n\n") != std::string::npos) {
            break;
        }
    }

    Logger::log("[SocketManager] Ligne 156 : Message recu : " + message);
    return message;
}

// Traitement d'un message texte recu et conversion en map
std::map<std::string, std::string>
SocketManager::traiterMessage(const std::string& message) {
    std::map<std::string, std::string> result;

    if (message.empty()) {
        Logger::log(
            "[SocketManager] Ligne 167 : Erreur : Message vide.", true);
        return result;
    }

    std::istringstream stream(message);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.empty()) {
            break; // Double newline marque la fin du message
        }

        size_t pos = line.find('=');
        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            result[key] = value;
        }
    }

    Logger::log(
        "[SocketManager] Ligne 189 : Message texte traite avec succes.");
    return result;
}
