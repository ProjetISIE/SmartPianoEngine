#include "RtMidiInput.hpp"
#include "Logger.hpp"
#include <chrono>
#include <map>
#include <thread>

RtMidiInput::RtMidiInput()
    : midiIn_(nullptr), midiOut_(nullptr), notesAvailable_(false) {
    Logger::log("[RtMidiInput] Instance créée");
}

RtMidiInput::~RtMidiInput() {
    close();
    Logger::log("[RtMidiInput] Instance détruite");
}

bool RtMidiInput::initialize() {
    Logger::log("[RtMidiInput] Initialisation MIDI (JACK)...");

    try {
        midiIn_ = new RtMidiIn(RtMidi::Api::UNIX_JACK, "SmartPianoEngine");
        midiOut_ = new RtMidiOut(RtMidi::Api::UNIX_JACK, "SmartPianoEngine");
    } catch (RtMidiError& error) {
        Logger::log("[RtMidiInput] Erreur création RtMidi: " +
                        error.getMessage(),
                    true);
        return false;
    }

    try {
        midiIn_->openVirtualPort("input");
        midiOut_->openVirtualPort("output");
        midiIn_->ignoreTypes(false, false, false);

        // Lancer le thread de traitement MIDI
        std::thread(&RtMidiInput::processMidiMessages, this).detach();

        Logger::log("[RtMidiInput] Ports JACK ouverts avec succès");
        return true;

    } catch (RtMidiError& error) {
        Logger::log("[RtMidiInput] Erreur ouverture ports: " +
                        error.getMessage(),
                    true);
        return false;
    }
}

std::vector<Note> RtMidiInput::readNotes() {
    // Attendre que des notes soient disponibles
    while (!notesAvailable_.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::vector<Note> notes;
    {
        std::lock_guard<std::mutex> lock(notesMutex_);
        notes = lastNotes_;
        lastNotes_.clear();
        notesAvailable_ = false;
    }

    return notes;
}

void RtMidiInput::close() {
    Logger::log("[RtMidiInput] Fermeture des ressources...");

    if (midiIn_) {
        delete midiIn_;
        midiIn_ = nullptr;
    }

    if (midiOut_) {
        delete midiOut_;
        midiOut_ = nullptr;
    }
}

bool RtMidiInput::isReady() const {
    return midiIn_ != nullptr && midiOut_ != nullptr;
}

void RtMidiInput::processMidiMessages() {
    Logger::log("[RtMidiInput] Thread MIDI démarré");

    constexpr int CHORD_TIMEOUT_MS = 100; // Timeout pour accumulation d'accord

    std::vector<unsigned char> message;
    std::vector<Note> currentNotes;
    auto lastNoteTime = std::chrono::steady_clock::now();
    bool chordInProgress = false;

    while (midiIn_) {
        try {
            if (!midiIn_) break;

            midiIn_->getMessage(&message);

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
                    Logger::log("[RtMidiInput] Note reçue: " + note.toString());
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
                        std::lock_guard<std::mutex> lock(notesMutex_);
                        lastNotes_ = currentNotes;
                        notesAvailable_ = true;
                    }
                    currentNotes.clear();
                    chordInProgress = false;
                    Logger::log("[RtMidiInput] Accord finalisé (" +
                                std::to_string(lastNotes_.size()) + " notes)");
                }
            }

        } catch (const std::exception& e) {
            Logger::log("[RtMidiInput] Exception: " + std::string(e.what()),
                        true);
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
    if (it == noteNames.end()) {
        return Note("c", 4); // Note par défaut en cas d'erreur
    }

    return Note(it->second, octave);
}
