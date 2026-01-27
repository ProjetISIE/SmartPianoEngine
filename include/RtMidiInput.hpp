#ifndef RTMIDIINPUT_HPP
#define RTMIDIINPUT_HPP

#include "IMidiInput.hpp"
#include "Logger.hpp"
#include <atomic>
#include <mutex>
#include <rtmidi/RtMidi.h>
#include <vector>

/**
 * @brief Implémentation de l'entrée MIDI via RTMidi
 *
 * Gère la réception des notes MIDI depuis un périphérique
 */
class RtMidiInput : public IMidiInput {
  private:
    RtMidiIn* midiIn{nullptr};               ///< Instance RTMidi
    RtMidiOut* midiOut{nullptr};             ///< Sortie MIDI (pour écho)
    std::vector<Note> lastNotes;             ///< Dernières notes jouées
    std::atomic<bool> notesAvailable{false}; ///< Notes disponibles?
    std::mutex notesMutex;                   ///< Mutex pour accès aux notes

  private:
    /**
     * @brief Traite les messages MIDI reçus (thread séparé)
     */
    void processMidiMessages();

    /**
     * @brief Convertit un numéro MIDI en Note
     * @param midiNote Numéro MIDI (0-127)
     * @return Note correspondante
     */
    Note convertMidiToNote(int midiNote) const;

  public:
    RtMidiInput() { Logger::log("[RtMidiInput] Instance créée"); }

    ~RtMidiInput() {
        close();
        Logger::log("[RtMidiInput] Instance détruite");
    }

    /**
     * @brief Initialise l'entrée MIDI
     * @return true si initialisation réussie
     */
    bool initialize() override;

    /**
     * @brief Lit les notes jouées (bloquant)
     * @return Vecteur de notes jouées
     */
    std::vector<Note> readNotes() override;

    /**
     * @brief Ferme l'entrée MIDI
     */
    void close() override;

    /**
     * @brief Vérifie si MIDI est prêt
     * @return true si MIDI est prêt
     */
    bool isReady() const override;
};

#endif // RTMIDIINPUT_HPP
