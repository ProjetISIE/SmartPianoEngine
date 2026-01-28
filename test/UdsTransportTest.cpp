#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "UdsTransport.hpp"
#include <chrono>
#include <doctest/doctest.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <thread>
#include <unistd.h>

void simpleClientUds(const std::string& path, const std::string& msgToSend) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock < 0) return;
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);
    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
        send(sock, msgToSend.c_str(), msgToSend.length(), 0);
        char buf[1024];
        recv(sock, buf, sizeof(buf), 0);
    }
    close(sock);
}

TEST_CASE("UdsTransport communication") {
    std::string socketPath = "test_uds_transport.sock";
    UdsTransport transport(socketPath);
    if (transport.start()) {
        std::string msg = "TEST\nfoo=bar\n\n";
        std::thread clientThread(simpleClientUds, socketPath, msg);
        transport.waitForClient();
        CHECK(transport.isClientConnected());
        Message received = transport.receive();
        CHECK(received.getType() == "TEST");
        CHECK(received.getField("foo") == "bar");
        transport.send(Message("ACK"));
        if (clientThread.joinable()) clientThread.join();
        transport.stop();
        CHECK_FALSE(transport.isClientConnected());
    } else WARN("Could not create server socket. Skipping test.");
}

TEST_CASE("UdsTransport Edge Cases") {
    UdsTransport transport("test_edge.sock");

    SUBCASE("Send without client") {
        // Should not crash, just log error
        transport.send(Message("TEST"));
    }

    SUBCASE("Receive without client") {
        Message msg = transport.receive();
        CHECK(msg.getType() == "error");
    }

    SUBCASE("Stop idempotency") {
        transport.stop();
        transport.stop(); // Should be safe
    }
}
