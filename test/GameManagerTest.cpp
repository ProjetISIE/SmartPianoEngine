#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../include/GameManager.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QTcpSocket>
#include <QTest>
#include <doctest/doctest.h>

// Classe derivee pour acceder aux methodes protegees
class TestGameManager : public GameManager {
  public:
    using GameManager::lancerJeuDaccordRenversement;
    using GameManager::lancerJeuDaccordSR;
    using GameManager::lancerJeuDeNote;
};

// Test de l'initialisation du serveur
TEST_CASE("GameManager - Initialisation du serveur") {
    TestGameManager manager;
    CHECK(manager.initialiserServeur(8080) == true);
}

// Test de l'attente de connexion
TEST_CASE("GameManager - Attente de connexion") {
    TestGameManager manager;
    CHECK(manager.initialiserServeur(8080) == true);

    QTcpSocket client;
    client.connectToHost("127.0.0.1", 8080);

    QTest::qWait(500); // Attente pour la connexion

    CHECK(client.waitForConnected(3000) == true);
}

// Test du lancement du jeu avec des parametres valides
TEST_CASE("GameManager - Lancement du jeu") {
    TestGameManager manager;
    CHECK(manager.initialiserServeur(8080) == true);

    QTcpSocket client;
    client.connectToHost("127.0.0.1", 8080);
    CHECK(client.waitForConnected(3000) == true);
}

// Test du traitement d'un jeu de note
TEST_CASE("GameManager - Jeu de note") {
    TestGameManager manager;
    manager.lancerJeuDeNote("Do", "Majeur");
}

// Test du traitement d'un jeu d'accords sans renversement
TEST_CASE("GameManager - Jeu d'accord sans renversement") {
    TestGameManager manager;
    manager.lancerJeuDaccordSR("Do", "Majeur");
}

// Test du traitement d'un jeu d'accords avec renversement
TEST_CASE("GameManager - Jeu d'accord avec renversement") {
    TestGameManager manager;
    manager.lancerJeuDaccordRenversement("Do", "Majeur");
}
