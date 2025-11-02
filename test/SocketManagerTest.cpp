#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../include/SocketManager.h"
#include <QByteArray>
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket>
#include <doctest/doctest.h>

// Test de l'initialisation du serveur
TEST_CASE("SocketManager - Initialisation du serveur") {
    SocketManager serveur;
    CHECK(serveur.initialiserServeur(8080) == true);
}

// Test de la connexion d'un client
TEST_CASE("SocketManager - Connexion client") {
    SocketManager serveur;
    CHECK(serveur.initialiserServeur(8080) == true);

    QTcpSocket client;
    client.connectToHost("127.0.0.1", 8080);
    CHECK(client.waitForConnected(3000) == true);
}

// Test de l'envoi d'un message JSON
TEST_CASE("SocketManager - Envoi d'un message") {
    SocketManager serveur;
    CHECK(serveur.initialiserServeur(8080) == true);

    QTcpSocket client;
    client.connectToHost("127.0.0.1", 8080);
    CHECK(client.waitForConnected(3000) == true);

    QJsonObject message = {{"type", "test"}, {"content", "Hello"}};
    serveur.envoyerMessage(message);
}

// Test de la reception d'un message JSON
TEST_CASE("SocketManager - Reception d'un message") {
    SocketManager serveur;
    CHECK(serveur.initialiserServeur(8080) == true);

    QTcpSocket client;
    client.connectToHost("127.0.0.1", 8080);
    CHECK(client.waitForConnected(3000) == true);
}

// Test du traitement d'un message JSON
TEST_CASE("SocketManager - Traitement JSON") {
    SocketManager serveur;
    QString jsonString = R"({"key1":"value1", "key2":"value2"})";
    std::map<std::string, std::string> resultat =
        serveur.traiterMessage(jsonString);

    CHECK(resultat["key1"] == "value1");
    CHECK(resultat["key2"] == "value2");
}
