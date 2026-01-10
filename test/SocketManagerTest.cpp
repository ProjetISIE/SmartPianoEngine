#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "../include/SocketManager.hpp"
#include <doctest/doctest.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

// Test de l'initialisation du serveur
TEST_CASE("SocketManager - Initialisation du serveur") {
    SocketManager serveur;
    std::string socketPath = "/tmp/test_smartpiano.sock";
    CHECK(serveur.initialiserServeur(socketPath) == true);
    unlink(socketPath.c_str());
}

// Test de la connexion d'un client
TEST_CASE("SocketManager - Connexion client") {
    SocketManager serveur;
    std::string socketPath = "/tmp/test_smartpiano2.sock";
    CHECK(serveur.initialiserServeur(socketPath) == true);

    // Creer un thread pour accepter la connexion
    std::thread serverThread([&serveur]() { serveur.attendreConnexion(); });

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

// Test du traitement d'un message texte
TEST_CASE("SocketManager - Traitement texte") {
    SocketManager serveur;
    std::string message = "key1=value1\nkey2=value2\n\n";
    std::map<std::string, std::string> resultat =
        serveur.traiterMessage(message);

    CHECK(resultat["key1"] == "value1");
    CHECK(resultat["key2"] == "value2");
}
