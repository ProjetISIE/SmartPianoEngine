#ifndef RTMIDIINPUT_HPP
#define RTMIDIINPUT_HPP

#include "IMidiInput.hpp"
#include <rtmidi/RtMidi.h>
#include <atomic>
#include <mutex>
#include <vector>

/**
 * @brief Implémentation de l'entrée MIDI via RTMidi
 *
 * Gère la réception des notes MIDI depuis un périphérique
 */
class RtMidiInput : public IMidiInput {
  public:
    /**
     * @brief Constructeur
     */
    RtMidiInput();

    /**
     * @brief Destructeur
     */
    ~RtMidiInput() override;

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

  private:
    RtMidiIn* midiIn_;                    ///< Instance RTMidi
    RtMidiOut* midiOut_;                  ///< Sortie MIDI (pour écho)
    std::vector<Note> lastNotes_;         ///< Dernières notes jouées
    std::atomic<bool> notesAvailable_;    ///< Notes disponibles?
    std::mutex notesMutex_;               ///< Mutex pour accès aux notes

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
};

#endif // RTMIDIINPUT_HPP
