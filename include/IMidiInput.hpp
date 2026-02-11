#ifndef IMIDIINPUT_HPP
#define IMIDIINPUT_HPP

#include "Note.hpp"
#include <stop_token>
#include <vector>

/**
 * @brief Interface pour l'entrée MIDI
 * Abstraction permettant différentes implémentations MIDI
 */
class IMidiInput {
  public:
    virtual ~IMidiInput() = default;

    /**
     * @brief Initialise l'entrée MIDI
     * @return true si initialisation réussie
     */
    virtual bool initialize() = 0;

    /**
     * @brief Lit les notes jouées (bloquant jusqu'à réception de notes)
     * @param stopToken Jeton d'arrêt pour annuler l'attente
     * @return Vecteur de notes jouées
     */
    virtual std::vector<Note> readNotes(std::stop_token stopToken) = 0;

    /**
     * @brief Vérifie si MIDI est initialisé et prêt
     * @return true si MIDI est prêt
     */
    virtual bool isReady() const = 0;
};

#endif // IMIDIINPUT_HPP
