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
    CHECK(transport.getSocketPath() == socketPath);
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
    std::string socketPath = "test_edge.sock";
    UdsTransport transport(socketPath);

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

    SUBCASE("Start failure (permission denied)") {
        // Try to bind to root directory or similar
        UdsTransport failTransport("/root/test.sock");
        CHECK_FALSE(failTransport.start());
    }

    SUBCASE("Client disconnects gracefully") {
        std::thread client([&]() {
            int sock = socket(AF_UNIX, SOCK_STREAM, 0);
            struct sockaddr_un addr;
            memset(&addr, 0, sizeof(addr));
            addr.sun_family = AF_UNIX;
            strncpy(addr.sun_path, socketPath.c_str(),
                    sizeof(addr.sun_path) - 1);
            if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
                // Send nothing or partial
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                close(sock);
            }
        });
        transport.waitForClient();
        Message msg = transport.receive(); // Should detect disconnect
        CHECK(msg.getType() == "error");
        if (client.joinable()) client.join();
    }
}

TEST_CASE("UdsTransport Malformed Data") {
    std::string socketPath = "test_malformed.sock";
    UdsTransport transport(socketPath);

    if (transport.start()) {
        struct sockaddr_un addr;
        memset(&addr, 0, sizeof(addr));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, socketPath.c_str(), sizeof(addr.sun_path) - 1);

        auto sendRaw = [&](const std::string& raw) {
            int sock = socket(AF_UNIX, SOCK_STREAM, 0);
            if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
                ::send(sock, raw.c_str(), raw.length(), 0);
                // Give server time to process
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            close(sock);
        };

        std::thread serverThread([&]() {
            transport.waitForClient();
            // 1. Missing type (empty line first)
            Message m1 = transport.receive();
            CHECK(m1.getType() == "error");

            transport.waitForClient();
            // 2. Malformed key=value (no =)
            Message m2 = transport.receive();
            // parseMessage skips invalid lines, so if valid type is there, it
            // returns it with empty fields (or what it found)
            CHECK(m2.getType() == "TEST_MALFORMED");
            CHECK(m2.getFields().empty());

            transport.waitForClient();
            // 3. Just newline (empty message?)
            Message m3 = transport.receive();
            CHECK(m3.getType() == "error");
        });

        // 1. Missing type
        sendRaw("\nkey=val\n\n");

        // 2. Malformed key=value
        sendRaw("TEST_MALFORMED\nnotakeyvalue\n\n");

        // 3. Empty
        sendRaw("\n\n");

        if (serverThread.joinable()) serverThread.join();
        transport.stop();
    }
}

TEST_CASE("UdsTransport CRLF Support") {
    std::string socketPath = "test_crlf.sock";
    UdsTransport transport(socketPath);
    if (transport.start()) {
        std::thread client([&]() {
            int sock = socket(AF_UNIX, SOCK_STREAM, 0);
            struct sockaddr_un addr;
            memset(&addr, 0, sizeof(addr));
            addr.sun_family = AF_UNIX;
            strncpy(addr.sun_path, socketPath.c_str(),
                    sizeof(addr.sun_path) - 1);
            if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
                std::string msg = "TYPE\r\nkey=val\r\n\r\n";
                ::send(sock, msg.c_str(), msg.length(), 0);
                close(sock);
            }
        });

        transport.waitForClient();
        Message msg = transport.receive();
        CHECK(msg.getType() == "TYPE");
        CHECK(msg.getField("key") == "val");

        if (client.joinable()) client.join();
        transport.stop();
    }
}

TEST_CASE("UdsTransport Send Failure") {
    std::string socketPath = "test_send_fail.sock";
    UdsTransport transport(socketPath);
    if (transport.start()) {
        // To simulate send failure, we can connect a client, close it, and then
        // try to send. But send() checks if clientSock < 0. If client closed
        // the connection, send() might receive SIGPIPE or return -1.
        // MSG_NOSIGNAL can be used if supported, or we handle signal.
        // But here we just want to hit the path where ::send returns < 0.

        // Start thread to connect and close immediately
        std::thread client([&]() {
            int sock = socket(AF_UNIX, SOCK_STREAM, 0);
            struct sockaddr_un addr;
            memset(&addr, 0, sizeof(addr));
            addr.sun_family = AF_UNIX;
            strncpy(addr.sun_path, socketPath.c_str(),
                    sizeof(addr.sun_path) - 1);
            connect(sock, (struct sockaddr*)&addr, sizeof(addr));
            close(sock);
        });

        transport.waitForClient();
        client.join();

        // Client closed. Writing now might fail.
        // Note: UdsTransport::send uses ::send. On Linux writing to closed
        // socket sends SIGPIPE. We should use MSG_NOSIGNAL in implementation or
        // ignore SIGPIPE. The implementation uses `::send(..., 0)`. If it
        // crashes, we know we need to fix implementation too!

        // Let's assume implementation doesn't crash (or we fix it).
        // Actually, let's fix implementation to use MSG_NOSIGNAL to be safe, or
        // just expect it. For now, let's try.

        // Wait a bit for close to propagate
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        transport.send(Message("SHOULD_FAIL"));

        transport.stop();
    }
}

TEST_CASE("UdsTransport waitForClient error handling") {
    std::string socketPath = "test_wait_fail.sock";
    UdsTransport transport(socketPath);
    
    // Call waitForClient without starting server (serverSock < 0)
    transport.waitForClient(); // Should log error and return
    CHECK_FALSE(transport.isClientConnected());
}

TEST_CASE("UdsTransport listen failure") {
    // This test attempts to cover the listen() failure path
    // This is hard to trigger reliably, but we can try binding to invalid path
    UdsTransport transport("/proc/test.sock"); // /proc is read-only
    CHECK_FALSE(transport.start()); // Should fail on bind or listen
}

TEST_CASE("UdsTransport recv error") {
    std::string socketPath = "test_recv_err.sock";
    UdsTransport transport(socketPath);
    
    if (transport.start()) {
        std::thread client([&]() {
            int sock = socket(AF_UNIX, SOCK_STREAM, 0);
            struct sockaddr_un addr;
            memset(&addr, 0, sizeof(addr));
            addr.sun_family = AF_UNIX;
            strncpy(addr.sun_path, socketPath.c_str(), sizeof(addr.sun_path) - 1);
            if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
                // Send incomplete message then close
                ::send(sock, "TYPE", 4, 0);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
                shutdown(sock, SHUT_RDWR);
                close(sock);
            }
        });
        
        transport.waitForClient();
        Message msg = transport.receive();
        CHECK(msg.getType() == "error");
        
        if (client.joinable()) client.join();
        transport.stop();
    }
}

TEST_CASE("UdsTransport message with newline detection") {
    std::string socketPath = "test_newline.sock";
    UdsTransport transport(socketPath);
    
    if (transport.start()) {
        std::thread client([&]() {
            int sock = socket(AF_UNIX, SOCK_STREAM, 0);
            struct sockaddr_un addr;
            memset(&addr, 0, sizeof(addr));
            addr.sun_family = AF_UNIX;
            strncpy(addr.sun_path, socketPath.c_str(), sizeof(addr.sun_path) - 1);
            if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
                // Send message with newline to trigger logging on line 96-97
                std::string msg = "TYPE\nkey=value\n\n";
                ::send(sock, msg.c_str(), msg.length(), 0);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                close(sock);
            }
        });
        
        transport.waitForClient();
        Message msg = transport.receive();
        CHECK(msg.getType() == "TYPE");
        
        if (client.joinable()) client.join();
        transport.stop();
    }
}
