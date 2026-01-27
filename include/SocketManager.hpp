#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H

#include "Logger.hpp"
#include <map>
#include <string>

/**
 * @brief Classe SocketManager
 *
 * Cette classe gere les interactions via des Unix Domain Sockets (UDS).
 * Elle permet de creer un serveur UDS, de gerer les connexions client,
 * et d'envoyer ou recevoir des messages en texte brut.
 */
class SocketManager {
  private:
    int serverSocket{-1}; ///< Descripteur du serveur UDS
    int clientSocket{-1}; ///< Descripteur de la socket client connectee
    const std::string socketPath{"/tmp/smartpiano.sock"}; ///< Chemin socket

  public:
    SocketManager() { Logger::log("[SocketManager] Constructeur initialisé"); }
    ~SocketManager();

    /**
     * @brief Initialise le serveur UDS
     * Configure le serveur pour ecouter les connexions sur un socket Unix.
     * @return true si le serveur demarre correctement, false sinon.
     */
    bool initialiserServeur();

    /**
     * @brief Attend la connexion d'un client
     *
     * Bloque jusqu'a ce qu'une connexion client soit etablie.
     */
    void attendreConnexion();

    /**
     * @brief Envoie un message texte au client
     *
     * @param message Message sous forme de map cle-valeur a envoyer au client
     * connecte. Format: "cle1=valeur1\ncle2=valeur2\n"
     */
    void envoyerMessage(const std::map<std::string, std::string>& message);

    /**
     * @brief Recoit un message texte depuis le client
     *
     * Attend la reception d'un message depuis la socket client.
     *
     * @return Le message recu sous forme de string, ou une chaine vide en cas
     * d'erreur.
     */
    std::string recevoirMessage();

    /**
     * @brief Traite un message texte recu
     *
     * Convertit un message texte au format "cle=valeur" en une map C++.
     *
     * @param message Message texte recu sous forme de string.
     * @return Une map contenant les cles et valeurs du message.
     */
    std::map<std::string, std::string>
    traiterMessage(const std::string& message);
};

#endif // SOCKETMANAGER_H
