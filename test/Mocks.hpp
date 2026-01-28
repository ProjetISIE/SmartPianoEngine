#ifndef MOCKS_HPP
#define MOCKS_HPP

#include "IMidiInput.hpp"
#include "ITransport.hpp"
#include "Message.hpp"
#include "Note.hpp"
#include <chrono>
#include <condition_variable>
#include <deque>
#include <mutex>
#include <vector>

class MockMidiInput : public IMidiInput {
  public:
    bool initialized = false;
    bool closed = false;
    bool initResult = true; // Default success
    std::deque<std::vector<Note>> notesQueue;
    std::mutex mtx;
    std::condition_variable cv;

    bool initialize() override {
        if (initResult) {
            initialized = true;
        }
        return initResult;
    }

    void setInitializeResult(bool res) { initResult = res; }
    void setReady(bool ready) { initialized = ready; }


    std::vector<Note> readNotes() override {
        std::unique_lock<std::mutex> lock(mtx);
        // Wait for notes to be available
        cv.wait(lock, [this] { return !notesQueue.empty() || closed; });

        if (notesQueue.empty() && closed) {
            return {};
        }

        auto notes = notesQueue.front();
        notesQueue.pop_front();
        return notes;
    }

    void close() override {
        {
            std::lock_guard<std::mutex> lock(mtx);
            closed = true;
        }
        cv.notify_all();
    }

    bool isReady() const override { return initialized && !closed; }

    // Helper to push input
    void pushNotes(const std::vector<Note>& notes) {
        std::lock_guard<std::mutex> lock(mtx);
        notesQueue.push_back(notes);
        cv.notify_one();
    }

    // Helper to push string notes
    void pushNotes(const std::vector<std::string>& notesStr) {
        std::vector<Note> notes;
        for (const auto& s : notesStr) notes.emplace_back(s);
        pushNotes(notes);
    }
};

class MockTransport : public ITransport {
  public:
    bool started = false;
    bool connected = false;
    const std::string sockPath{"/tmp/smartpiano.sock"}; ///< Chemin socket Unix
    std::deque<Message> sentMessages;
    std::deque<Message> incomingMessages;
    std::mutex mtx;
    std::condition_variable cv;

    bool start() override {
        started = true;
        return true;
    }

    void waitForClient() override { connected = true; }

    void send(const Message& msg) override {
        std::lock_guard<std::mutex> lock(mtx);
        sentMessages.push_back(msg);
        cv.notify_all(); // Notify that a message was sent
    }

    Message receive() override {
        std::unique_lock<std::mutex> lock(mtx);
        // Block until message available
        if (incomingMessages.empty()) {
            cv.wait(lock,
                    [this] { return !incomingMessages.empty() || !connected; });
        }
        if (incomingMessages.empty() && !connected) {
            return Message("error");
        }
        auto msg = incomingMessages.front();
        incomingMessages.pop_front();
        return msg;
    }

    void stop() override {
        {
            std::lock_guard<std::mutex> lock(mtx);
            started = false;
            connected = false;
        }
        cv.notify_all();
    }

    bool isClientConnected() const override { return connected; }

    // Helper
    void pushIncoming(const Message& msg) {
        std::lock_guard<std::mutex> lock(mtx);
        incomingMessages.push_back(msg);
        cv.notify_one();
    }

    // Helper to wait for a message sent by the game
    Message waitForSentMessage() {
        std::unique_lock<std::mutex> lock(mtx);
        if (sentMessages.empty()) {
            cv.wait_for(lock, std::chrono::seconds(10),
                        [this] { return !sentMessages.empty(); });
        }
        if (sentMessages.empty()) return Message("TIMEOUT");
        auto msg = sentMessages.front();
        sentMessages.pop_front();
        return msg;
    }

    /**
     * @brief Obtient le chemin de la socket Unix
     * @return Chemin de la socket (string)
     */
    std::string getSocketPath() const override { return this->sockPath; }
};

#endif // MOCKS_HPP
