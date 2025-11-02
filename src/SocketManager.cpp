#include "SocketManager.h"
#include "Logger.h"
#include <QCoreApplication>
#include <QJsonDocument>

// Constructeur de la classe SocketManager
SocketManager::SocketManager(QObject* parent)
    : QObject(parent), serveur(new QTcpServer(this)), clientSocket(nullptr) {
    Logger::log("[SocketManager] Ligne 10 : Constructeur initialise.");
}

// Destructeur de la classe SocketManager
SocketManager::~SocketManager() {
    delete serveur;
    Logger::log(
        "[SocketManager] Ligne 17 : Destructeur appele, serveur supprime.");
}

// Initialisation du serveur sur un port specifique
bool SocketManager::initialiserServeur(int port) {
    connect(serveur, &QTcpServer::newConnection, this, [&]() {
        clientSocket = serveur->nextPendingConnection();
        Logger::log("[SocketManager] Ligne 26 : Nouvelle connexion detectee.");
    });

    if (!serveur->listen(QHostAddress::Any, port)) {
        Logger::log("[SocketManager] Ligne 31 : Erreur : Impossible de "
                    "demarrer le serveur.",
                    true);
        return false;
    }

    Logger::log("[SocketManager] Ligne 35 : Serveur demarre sur le port " +
                std::to_string(port) + ".");
    return true;
}

// Attente de la connexion d'un client
void SocketManager::attendreConnexion() {
    while (!clientSocket) {
        QCoreApplication::processEvents(); // Traiter les evenements Qt pendant
                                           // l'attente
    }
    Logger::log("[SocketManager] Ligne 46 : Connexion client etablie.");
}

// Envoi d'un message JSON au client
void SocketManager::envoyerMessage(const QJsonObject& message) {
    if (!clientSocket || !clientSocket->isOpen()) {
        Logger::log("[SocketManager] Ligne 54 : Erreur : Aucun client connecte "
                    "pour envoyer le message.",
                    true);
        return;
    }

    QJsonDocument doc(message);
    QByteArray data = doc.toJson(QJsonDocument::Compact);
    clientSocket->write(data);
    clientSocket->flush();
    clientSocket->waitForBytesWritten();
    Logger::log("[SocketManager] Ligne 63 : Message envoye : " +
                data.toStdString());
}

// Reception d'un message JSON depuis le client
QString SocketManager::recevoirMessage() {
    if (!clientSocket) {
        Logger::log(
            "[SocketManager] Ligne 71 : Erreur : Pas de client connecte.",
            true);
        return QString();
    }

    if (!clientSocket->isOpen()) {
        Logger::log("[SocketManager] Ligne 77 : Erreur : Socket fermee.", true);
        return QString();
    }

    if (!clientSocket->waitForReadyRead(3000)) {
        Logger::log("[SocketManager] Ligne 83 : Aucun message recu apres delai "
                    "de 3 secondes.");
        return QString();
    }

    if (clientSocket->state() == QAbstractSocket::UnconnectedState) {
        Logger::log("[SocketManager] Ligne 89 : Le client est deconnecte.",
                    true);
        return QString();
    }

    QByteArray data = clientSocket->readAll();
    if (data.isEmpty()) {
        Logger::log(
            "[SocketManager] Ligne 96 : Avertissement : Message vide recu.");
        return QString();
    }

    Logger::log("[SocketManager] Ligne 100 : Message recu : " +
                data.toStdString());
    return QString::fromUtf8(data);
}

// Traitement d'un message JSON recu et conversion en map
std::map<std::string, std::string>
SocketManager::traiterMessage(const QString& message) {
    std::map<std::string, std::string> result;
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());

    if (doc.isNull() || !doc.isObject()) {
        Logger::log(
            "[SocketManager] Ligne 113 : Erreur : Message JSON invalide.",
            true);
        return result;
    }

    QJsonObject jsonObj = doc.object();
    for (auto it = jsonObj.begin(); it != jsonObj.end(); ++it) {
        result[it.key().toStdString()] = it.value().toString().toStdString();
    }

    Logger::log("[SocketManager] Ligne 123 : Message JSON traite avec succes.");
    return result;
}