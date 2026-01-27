#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "SocketManager.hpp"
#include <chrono>
#include <doctest/doctest.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

void simpleClient(const std::string& path, const std::string& msgToSend) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) return;
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0)
        send(sock, msgToSend.c_str(), msgToSend.length(), 0);
    close(sock);
}

TEST_CASE("SocketManager communication") {
    SocketManager sm;
    std::string socketPath = "test_socket_manager.sock";
    if (sm.initialiserServeur()) {
        std::string msg = "key1=value1\nkey2=value2\n\n";
        std::thread clientThread(simpleClient, socketPath, msg);
        sm.attendreConnexion();
        std::string received = sm.recevoirMessage();
        CHECK(received.find("key1=value1") != std::string::npos);
        auto map = sm.traiterMessage(received);
        CHECK(map["key1"] == "value1");
        CHECK(map["key2"] == "value2");
        if (clientThread.joinable()) clientThread.join();
    } else WARN("Could not create server socket. Skipping test.");
}
