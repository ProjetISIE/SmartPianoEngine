#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "RtMidiInput.hpp"
#include <deque>
#include <doctest/doctest.h>
#include <mutex>
#include <rtmidi/RtMidi.h> // for RtMidiError
#include <thread>

/// Mock pour IRtMidiIn permettant injection messages MIDI de test
/// Simule port virtuel MIDI et file messages pour tests
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

/// Mock pour IRtMidiOut (sortie MIDI pour tests)
/// Simule port virtuel sortie MIDI
class MockRtMidiOut : public IRtMidiOut {
  public:
    bool openPortCalled = false;

    void openVirtualPort(const std::string& portName) override {
        (void)portName;
        openPortCalled = true;
    }
};

/// RtMidiInput testable permettant injection mocks
/// Surcharge createMidiIn/Out pour retourner mocks au lieu de vrais objets RtMidi
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

/// Vérifie initialisation réussie et ouverture ports virtuels MIDI
/// Valide que initialize ouvre ports entrée/sortie et marque comme prêt
TEST_CASE("RtMidiInput initialization success") {
    TestableRtMidiInput input;
    CHECK(input.initialize());
    CHECK(input.isReady());
    CHECK(input.mockIn->openPortCalled);
    CHECK(input.mockOut->openPortCalled);
    input.close();
    CHECK_FALSE(input.isReady());
}

/// Vérifie gestion exception lors traitement messages MIDI
/// Valide que thread continue après exception et ne plante pas
TEST_CASE("RtMidiInput processing exception") {
    TestableRtMidiInput input;
    input.initialize();

    // Configurer mock pour lancer exception sur getMessage
    // On attend un peu pour s'assurer que thread le récupère
    // Boucle thread attrape exception et continue
    input.mockIn->throwOnGet = true;

    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // Restaurer normal pour fermer proprement
    input.mockIn->throwOnGet = false;

    input.close();
}

/// Vérifie échec initialisation lors création objets RtMidi
/// Valide que erreurs création sont gérées correctement
TEST_CASE("RtMidiInput initialization failure (creation)") {
    TestableRtMidiInput input;
    input.forceCreateError = true;
    CHECK_FALSE(input.initialize());
    CHECK_FALSE(input.isReady());
}

/// Vérifie échec initialisation lors ouverture port virtuel
/// Valide gestion erreur RtMidi openVirtualPort
TEST_CASE("RtMidiInput initialization failure (open port)") {
    TestableRtMidiInput input;
    input.throwOnOpen = true;
    CHECK_FALSE(input.initialize());
    CHECK_FALSE(input.isReady());
}

/// Vérifie traitement et conversion message MIDI Note On vers objet Note
/// Valide conversion MIDI (status 0x90, note 60 = C4)
TEST_CASE("RtMidiInput message processing") {
    TestableRtMidiInput input;
    input.initialize();

    // Envoyer Note On (status 0x90, note 60 (C4), vélocité 100)
    std::vector<unsigned char> noteOn = {0x90, 60, 100};
    input.mockIn->pushMessage(noteOn);

    // Attendre timeout accord (100ms dans code) -> attendre 150ms
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Lire notes
    std::vector<Note> notes = input.readNotes();
    CHECK(notes.size() == 1);
    CHECK(notes[0].toString() == "c4");

    input.close();
}

/// Vérifie regroupement notes simultanées en accord (timeout 100ms)
/// Valide détection accords via timing entre notes MIDI
TEST_CASE("RtMidiInput chord processing") {
    TestableRtMidiInput input;
    input.initialize();

    // Envoyer C4
    input.mockIn->pushMessage({0x90, 60, 100});
    // Envoyer E4 peu après
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    input.mockIn->pushMessage({0x90, 64, 100});

    // Attendre timeout
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    // Lire notes
    std::vector<Note> notes = input.readNotes();
    CHECK(notes.size() == 2);
    // Ordre peut dépendre, mais habituellement préservé
    // Logique dans processMidiMessages: push_back
    // c4 en premier, e4 en second
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

/// Expose méthodes protégées pour tester vraies implémentations RtMidi
/// Permet tester factory methods et classes wrapper réelles partiellement
class ExposeRealRtMidiInput : public RtMidiInput {
  public:
    using RtMidiInput::createMidiIn;
    using RtMidiInput::createMidiOut;
    // On ne surcharge pas, donc on utilise implémentation base de
    // RtMidiInput.cpp
};

/// Vérifie instanciation vraies implémentations RtMidi (si système audio dispo)
/// Test factory methods et wrappers méthodes IRtMidiIn/Out
TEST_CASE("RtMidiInput Real Implementation Instantiation") {
    ExposeRealRtMidiInput input;
    // Tenter créer instances réelles

    try {
        IRtMidiIn* in = input.createMidiIn();
        CHECK(in != nullptr);
        // Couvrir méthodes wrapper
        try {
            in->openVirtualPort("test_input");
            in->ignoreTypes(true, true, true);
            std::vector<unsigned char> msg;
            in->getMessage(&msg);
        } catch (...) {}
        delete in;
    } catch (const RtMidiError&) {
        // Attendu si pas système audio
    } catch (const std::exception&) {}

    try {
        IRtMidiOut* out = input.createMidiOut();
        CHECK(out != nullptr);
        try {
            out->openVirtualPort("test_output");
        } catch (...) {}
        delete out;
    } catch (const RtMidiError&) {
        // Attendu si pas système audio
    } catch (const std::exception&) {}
}
