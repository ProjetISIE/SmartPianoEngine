#ifndef RTMIDIINPUT_HPP
#define RTMIDIINPUT_HPP

#include "IMidiInput.hpp"
#include "Logger.hpp"
#include <atomic>
#include <memory>
#include <mutex>
#include <stop_token>
#include <thread>
#include <vector>

// Forward declarations to avoid including RtMidi.h if possible, or include it
// if needed for types. RtMidi uses std::string and vector.
#include <string>

// Interfaces for wrapping RtMidi
class IRtMidiIn {
  public:
    virtual ~IRtMidiIn() = default;
    virtual void openVirtualPort(const std::string& portName) = 0;
    virtual void ignoreTypes(bool midiSysex, bool midiTime, bool midiSense) = 0;
    virtual double getMessage(std::vector<unsigned char>* message) = 0;
};

class IRtMidiOut {
  public:
    virtual ~IRtMidiOut() = default;
    virtual void openVirtualPort(const std::string& portName) = 0;
};

/**
 * @brief Implémentation de l'entrée MIDI via RTMidi
 *
 * Gère la réception des notes MIDI depuis un périphérique
 */
class RtMidiInput : public IMidiInput {
  private:
    std::unique_ptr<IRtMidiIn> midiIn;       ///< Instance RTMidi (Wrapper)
    std::unique_ptr<IRtMidiOut> midiOut;     ///< Sortie MIDI (Wrapper)
    std::vector<Note> lastNotes;             ///< Dernières notes jouées
    std::atomic<bool> notesAvailable{false}; ///< Notes disponibles?
    std::mutex notesMutex;                   ///< Mutex pour accès aux notes

    std::jthread inputThread; ///< Thread de traitement

  private:
    /**
     * @brief Traite les messages MIDI reçus (thread séparé)
     * @param stopToken Jeton d'arrêt pour le thread
     */
    void processMidiMessages(std::stop_token stopToken);

    /**
     * @brief Convertit un numéro MIDI en Note
     * @param midiNote Numéro MIDI (0-127)
     * @return Note correspondante
     */
    Note convertMidiToNote(int midiNote) const;

  public:
    RtMidiInput() { Logger::log("[RtMidiInput] Instance créée"); }

    ~RtMidiInput() { Logger::log("[RtMidiInput] Instance détruite"); }

    /**
     * @brief Initialise l'entrée MIDI
     * @return true si initialisation réussie
     */
    bool initialize() override;

    /**
     * @brief Lit les notes jouées (bloquant)
     * @param stopToken Jeton d'arrêt pour annuler l'attente
     * @return Vecteur de notes jouées
     */
    std::vector<Note> readNotes(std::stop_token stopToken) override;

    /**
     * @brief Vérifie si MIDI est prêt
     * @return true si MIDI est prêt
     */
    bool isReady() const override;

  protected:
    /**
     * @brief Crée l'instance IRtMidiIn
     * @return Pointeur vers IRtMidiIn
     */
    virtual std::unique_ptr<IRtMidiIn> createMidiIn();

    /**
     * @brief Crée l'instance IRtMidiOut
     * @return Pointeur vers IRtMidiOut
     */
    virtual std::unique_ptr<IRtMidiOut> createMidiOut();
};

#endif // RTMIDIINPUT_HPP
