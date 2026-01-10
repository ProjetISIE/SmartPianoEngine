#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../include/GameManager.hpp"
#include <doctest/doctest.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

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
    std::string socketPath = "/tmp/test_gamemanager.sock";
    CHECK(manager.initialiserServeur(socketPath) == true);
    unlink(socketPath.c_str());
}

// Test de l'attente de connexion
TEST_CASE("GameManager - Attente de connexion") {
    TestGameManager manager;
    std::string socketPath = "/tmp/test_gamemanager2.sock";
    CHECK(manager.initialiserServeur(socketPath) == true);

    // Creer un thread pour accepter la connexion
    std::thread serverThread([&manager]() { manager.attendreConnexion(); });

    // Attendre un peu pour que le serveur soit pret
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Client se connecte
    int clientSock = socket(AF_UNIX, SOCK_STREAM, 0);
    CHECK(clientSock >= 0);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socketPath.c_str(), sizeof(addr.sun_path) - 1);

    int result = connect(clientSock, (struct sockaddr*)&addr, sizeof(addr));
    CHECK(result == 0);

    close(clientSock);
    serverThread.join();
    unlink(socketPath.c_str());
}

// Note: Tests for game modes (lancerJeuDeNote, lancerJeuDaccordSR, 
// lancerJeuDaccordRenversement) require MIDI hardware and cannot be run
// in automated test environment without mocking
