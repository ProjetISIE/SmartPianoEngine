#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "RtMidiInput.hpp"
#include <deque>
#include <doctest/doctest.h>
#include <mutex>
#include <rtmidi/RtMidi.h> // for RtMidiError
#include <thread>

// Mock for IRtMidiIn
class MockRtMidiIn : public IRtMidiIn {
  public:
    std::deque<std::vector<unsigned char>> messageQueue;
    std::mutex queueMutex;
    bool openPortCalled = false;
    bool throwOnOpen = false;
    bool throwOnGet = false;

    void openVirtualPort(const std::string& portName) override {
        (void)portName;
        if (throwOnOpen)
            throw RtMidiError("Mock error", RtMidiError::DRIVER_ERROR);
        openPortCalled = true;
    }

    void ignoreTypes(bool midiSysex, bool midiTime, bool midiSense) override {
        (void)midiSysex;
        (void)midiTime;
        (void)midiSense;
    }

    double getMessage(std::vector<unsigned char>* message) override {
        if (throwOnGet) throw std::runtime_error("Mock processing error");
        std::lock_guard<std::mutex> lock(queueMutex);
        if (messageQueue.empty()) {
            message->clear();
            return 0.0;
        }
        *message = messageQueue.front();
        messageQueue.pop_front();
        return 0.0;
    }

    void pushMessage(const std::vector<unsigned char>& msg) {
        std::lock_guard<std::mutex> lock(queueMutex);
        messageQueue.push_back(msg);
    }
};

// Mock for IRtMidiOut
class MockRtMidiOut : public IRtMidiOut {
  public:
    bool openPortCalled = false;

    void openVirtualPort(const std::string& portName) override {
        (void)portName;
        openPortCalled = true;
    }
};

// Testable RtMidiInput
class TestableRtMidiInput : public RtMidiInput {
  public:
    MockRtMidiIn* mockIn = nullptr;
    MockRtMidiOut* mockOut = nullptr;
    bool forceCreateError = false;
    bool throwOnOpen = false;

  protected:
    IRtMidiIn* createMidiIn() override {
        if (forceCreateError)
            throw RtMidiError("Mock creation error", RtMidiError::DRIVER_ERROR);
        mockIn = new MockRtMidiIn();
        mockIn->throwOnOpen = throwOnOpen;
        return mockIn;
    }

    IRtMidiOut* createMidiOut() override {
        mockOut = new MockRtMidiOut();
        return mockOut;
    }
};

TEST_CASE("RtMidiInput initialization success") {
    TestableRtMidiInput input;
    CHECK(input.initialize());
    CHECK(input.isReady());
    CHECK(input.mockIn->openPortCalled);
    CHECK(input.mockOut->openPortCalled);
    input.close();
    CHECK_FALSE(input.isReady());
}

TEST_CASE("RtMidiInput processing exception") {
    TestableRtMidiInput input;
    input.initialize();

    // Configure mock to throw on getMessage
    // We need to wait a bit to ensure the thread picks it up.
    // The thread loop catches exception and continues.
    input.mockIn->throwOnGet = true;

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Restore to normal to close cleanly
    input.mockIn->throwOnGet = false;

    input.close();
}

TEST_CASE("RtMidiInput initialization failure (creation)") {
    TestableRtMidiInput input;
    input.forceCreateError = true;
    CHECK_FALSE(input.initialize());
    CHECK_FALSE(input.isReady());
}

TEST_CASE("RtMidiInput initialization failure (open port)") {
    TestableRtMidiInput input;
    input.throwOnOpen = true;
    CHECK_FALSE(input.initialize());
    CHECK_FALSE(input.isReady());
}

TEST_CASE("RtMidiInput message processing") {
    TestableRtMidiInput input;
    input.initialize();

    // Send Note On (status 0x90, note 60 (C4), velocity 100)
    std::vector<unsigned char> noteOn = {0x90, 60, 100};
    input.mockIn->pushMessage(noteOn);

    // Wait for chord timeout (100ms in code) -> wait 150ms
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Read notes
    std::vector<Note> notes = input.readNotes();
    CHECK(notes.size() == 1);
    CHECK(notes[0].toString() == "c4");

    input.close();
}

TEST_CASE("RtMidiInput chord processing") {
    TestableRtMidiInput input;
    input.initialize();

    // Send C4
    input.mockIn->pushMessage({0x90, 60, 100});
    // Send E4 shortly after
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    input.mockIn->pushMessage({0x90, 64, 100});

    // Wait for timeout
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Read notes
    std::vector<Note> notes = input.readNotes();
    CHECK(notes.size() == 2);
    // Order might depend, but usually preserved
    // Logic in processMidiMessages: push_back.
    // c4 first, e4 second.
    bool foundC4 = false;
    bool foundE4 = false;
    for (const auto& n : notes) {
        if (n.toString() == "c4") foundC4 = true;
        if (n.toString() == "e4") foundE4 = true;
    }
    CHECK(foundC4);
    CHECK(foundE4);

    input.close();
}

// Expose protected methods to test the real factory methods (and thus the real
// Impl classes partially)
class ExposeRealRtMidiInput : public RtMidiInput {
  public:
    using RtMidiInput::createMidiIn;
    using RtMidiInput::createMidiOut;
    // We don't override them, so we use the base implementation from
    // RtMidiInput.cpp
};

TEST_CASE("RtMidiInput Real Implementation Instantiation") {
    ExposeRealRtMidiInput input;
    // Attempt to create real instances.

    try {
        IRtMidiIn* in = input.createMidiIn();
        CHECK(in != nullptr);
        // Cover wrapper methods
        try {
            in->openVirtualPort("test_input");
            in->ignoreTypes(true, true, true);
            std::vector<unsigned char> msg;
            in->getMessage(&msg);
        } catch (...) {}
        delete in;
    } catch (const RtMidiError&) {
        // Expected if no audio system
    } catch (const std::exception&) {}

    try {
        IRtMidiOut* out = input.createMidiOut();
        CHECK(out != nullptr);
        try {
            out->openVirtualPort("test_output");
        } catch (...) {}
        delete out;
    } catch (const RtMidiError&) {
        // Expected if no audio system
    } catch (const std::exception&) {}
}
