#include "RtMidiInput.hpp"
#include "Logger.hpp"
#include <chrono>
#include <map>
#include <memory>
#include <rtmidi/RtMidi.h>
#include <thread>

// Wrappers implementation
class RtMidiInImpl : public IRtMidiIn {
    RtMidiIn* midi;

  public:
    RtMidiInImpl() {
        midi = new RtMidiIn(RtMidi::Api::UNSPECIFIED, "SmartPianoEngine");
    }
    ~RtMidiInImpl() override { delete midi; }
    void openVirtualPort(const std::string& portName) override {
        midi->openVirtualPort(portName);
    }
    void ignoreTypes(bool midiSysex, bool midiTime, bool midiSense) override {
        midi->ignoreTypes(midiSysex, midiTime, midiSense);
    }
    double getMessage(std::vector<unsigned char>* message) override {
        return midi->getMessage(message);
    }
};

class RtMidiOutImpl : public IRtMidiOut {
    RtMidiOut* midi;

  public:
    RtMidiOutImpl() {
        midi = new RtMidiOut(RtMidi::Api::UNSPECIFIED, "SmartPianoEngine");
    }
    ~RtMidiOutImpl() override { delete midi; }
    void openVirtualPort(const std::string& portName) override {
        midi->openVirtualPort(portName);
    }
};

bool RtMidiInput::initialize() {
    Logger::log("[RtMidiInput] Initialisation MIDI");
    try {
        midiIn = createMidiIn();
        midiOut = createMidiOut();
    } catch (RtMidiError& error) {
        Logger::err("[RtMidiInput] Erreur création RtMidi: {}",
                    error.getMessage());
        return false;
    }
    try {
        midiIn->openVirtualPort("input");
        midiOut->openVirtualPort("output");
        midiIn->ignoreTypes(false, false, false);

        // Lancer thread de traitement MIDI
        inputThread = std::jthread(
            [this](std::stop_token st) { this->processMidiMessages(st); });

        Logger::log("[RtMidiInput] Ports JACK ouverts avec succès");
        return true;
    } catch (RtMidiError& error) {
        Logger::err("[RtMidiInput] Erreur ouverture ports: {}",
                    error.getMessage());
        return false;
    }
}

std::unique_ptr<IRtMidiIn> RtMidiInput::createMidiIn() {
    return std::make_unique<RtMidiInImpl>();
}

std::unique_ptr<IRtMidiOut> RtMidiInput::createMidiOut() {
    return std::make_unique<RtMidiOutImpl>();
}

std::vector<Note> RtMidiInput::readNotes(std::stop_token stopToken) {
    // Attendre que des notes soient disponibles
    while (!notesAvailable.load()) {
        if (stopToken.stop_requested()) return {}; // Sortir si arrêt demandé
        // COUVERTURE: Nécessite d’attendre dans tests mockés
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::vector<Note> notes;
    {
        std::lock_guard<std::mutex> lock(notesMutex);
        notes = lastNotes;
        lastNotes.clear();
        notesAvailable = false;
    }
    return notes;
}

bool RtMidiInput::isReady() const {
    return midiIn != nullptr && midiOut != nullptr;
}

void RtMidiInput::processMidiMessages(std::stop_token stopToken) {
    Logger::log("[RtMidiInput] Thread MIDI démarré");
    constexpr int CHORD_TIMEOUT_MS = 100; // Timeout pour accumulation d'accord
    std::vector<unsigned char> message;
    std::vector<Note> currentNotes;
    auto lastNoteTime = std::chrono::steady_clock::now();
    bool chordInProgress = false;
    while (!stopToken.stop_requested() && midiIn) {
        try {
            midiIn->getMessage(&message);
            if (!message.empty() && message.size() >= 3) {
                int status = message[0] & 0xF0;
                int midiNote = message[1];
                int velocity = message[2];
                // Note On avec vélocité > 0
                if (status == 0x90 && velocity > 0) {
                    Note note = convertMidiToNote(midiNote);
                    currentNotes.push_back(note);
                    lastNoteTime = std::chrono::steady_clock::now();
                    chordInProgress = true;
                    Logger::log("[RtMidiInput] Note reçue: {}",
                                note.toString());
                }
            }
            // Vérifier le timeout pour finaliser l'accord
            if (chordInProgress) {
                auto now = std::chrono::steady_clock::now();
                auto elapsed =
                    std::chrono::duration_cast<std::chrono::milliseconds>(
                        now - lastNoteTime)
                        .count();
                if (elapsed >= CHORD_TIMEOUT_MS && !currentNotes.empty()) {
                    // Timeout expiré - finaliser l'accord
                    {
                        std::lock_guard<std::mutex> lock(notesMutex);
                        lastNotes = currentNotes;
                        notesAvailable = true;
                    }
                    currentNotes.clear();
                    chordInProgress = false;
                    Logger::log("[RtMidiInput] Accord finalisé ({})",
                                std::to_string(lastNotes.size()));
                }
            }
        } catch (const std::exception& e) {
            Logger::err("[RtMidiInput] Exception: {}", e.what());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

Note RtMidiInput::convertMidiToNote(int midiNote) const {
    static const std::map<int, std::string> noteNames = {
        {0, "c"},  {1, "c#"}, {2, "d"},  {3, "d#"}, {4, "e"},   {5, "f"},
        {6, "f#"}, {7, "g"},  {8, "g#"}, {9, "a"},  {10, "a#"}, {11, "b"}};
    int octave = (midiNote / 12) - 1;
    int noteIndex = midiNote % 12;
    auto it = noteNames.find(noteIndex);
    return Note(it->second, octave);
}
