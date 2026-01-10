#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H

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
  public:
    /**
     * @brief Constructeur de la classe SocketManager
     */
    SocketManager();

    /**
     * @brief Destructeur de la classe SocketManager
     *
     * Libere les ressources allouees pour le serveur et la socket client.
     */
    ~SocketManager();

    /**
     * @brief Initialise le serveur UDS
     *
     * Configure le serveur pour ecouter les connexions sur un socket Unix.
     *
     * @param socketPath Chemin du socket Unix (ex: "/tmp/smartpiano.sock")
     * @return true si le serveur demarre correctement, false sinon.
     */
    bool initialiserServeur(const std::string& socketPath);

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
    std::map<std::string, std::string> traiterMessage(const std::string& message);

  private:
    int serverSocket;  ///< Descripteur du serveur UDS
    int clientSocket;  ///< Descripteur de la socket client connectee
    std::string socketPath; ///< Chemin du socket Unix
};

#endif // SOCKETMANAGER_H
