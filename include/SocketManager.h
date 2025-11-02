#ifndef SOCKETMANAGER_H
#define SOCKETMANAGER_H

#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QTcpServer>
#include <QTcpSocket>
#include <map>
#include <string>

/**
 * @brief Classe SocketManager
 *
 * Cette classe gere les interactions reseau via des sockets TCP.
 * Elle permet de creer un serveur TCP, de gerer les connexions client,
 * et d'envoyer ou recevoir des messages JSON.
 */
class SocketManager : public QObject {
    Q_OBJECT

  public:
    /**
     * @brief Constructeur de la classe SocketManager
     *
     * @param parent Pointeur vers l'objet parent (par defaut nullptr).
     */
    SocketManager(QObject* parent = nullptr);

    /**
     * @brief Destructeur de la classe SocketManager
     *
     * Libere les ressources allouees pour le serveur et la socket client.
     */
    ~SocketManager();

    /**
     * @brief Initialise le serveur TCP
     *
     * Configure le serveur pour ecouter les connexions sur un port specifique.
     *
     * @param port Port sur lequel le serveur ecoute.
     * @return true si le serveur demarre correctement, false sinon.
     */
    bool initialiserServeur(int port);

    /**
     * @brief Attend la connexion d'un client
     *
     * Bloque jusqu'a ce qu'une connexion client soit etablie.
     */
    void attendreConnexion();

    /**
     * @brief Envoie un message JSON au client
     *
     * @param message Message au format QJsonObject a envoyer au client
     * connecte.
     */
    void envoyerMessage(const QJsonObject& message);

    /**
     * @brief Recoit un message JSON depuis le client
     *
     * Attend la reception d'un message depuis la socket client.
     *
     * @return Le message recu sous forme de QString, ou une chaine vide en cas
     * d'erreur.
     */
    QString recevoirMessage();

    /**
     * @brief Traite un message JSON recu
     *
     * Convertit un message JSON au format QString en une map C++.
     *
     * @param message Message JSON recu sous forme de QString.
     * @return Une map contenant les cles et valeurs du message JSON.
     */
    std::map<std::string, std::string> traiterMessage(const QString& message);

  private:
    QTcpServer* serveur;      ///< Pointeur vers le serveur TCP
    QTcpSocket* clientSocket; ///< Pointeur vers la socket client connectee
};

#endif // SOCKETMANAGER_H
