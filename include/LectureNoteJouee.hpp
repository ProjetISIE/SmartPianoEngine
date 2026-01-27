#ifndef LECTURENOTEJOUEE_H
#define LECTURENOTEJOUEE_H

#include "Logger.hpp"
#include <atomic>
#include <mutex>
#include <rtmidi/RtMidi.h>
#include <string>
#include <vector>

/**
 * @brief Classe LectureNoteJouee
 *
 * Cette classe permet de lire les notes ou accords joués sur un périphérique
 * MIDI. Elle gère la connexion au périphérique, la lecture des messages MIDI et
 * leur conversion en notation musicale standard.
 */
class LectureNoteJouee {
  private:
    RtMidiIn* midiIn{nullptr};   ///< Pointeur sur l’entrée MIDI
    RtMidiOut* midiOut{nullptr}; ///< Pointeur sur la sortie MIDI

  private:
    /**
     * @brief Traite les messages MIDI reçus
     *
     * Fonction exécutée dans un thread séparé pour gérer les messages MIDI.
     */
    void traiterMessagesMIDI();

  protected:
    std::string derniereNote;                ///< Dernière note jouée
    std::vector<std::string> dernierAccord;  ///< Dernier accord joué
    std::atomic<bool> noteDisponible{false}; ///< Jeu peut lire un résultat ?
    std::mutex noteMutex; ///< Mutex pour protéger l'accès aux notes

  protected:
    /**
     * @brief Convertit une note MIDI en notation musicale
     *
     * Prend une valeur MIDI et la convertit en une chaîne représentant la note
     * (ex: "C4" pour le Do de la 4ème octave).
     *
     * @param noteMidi Valeur MIDI de la note
     * @return La note en notation musicale.
     */
    std::string convertirNote(int noteMidi);

  public:
    LectureNoteJouee() { Logger::log("[LectureNoteJouee] Instance créée"); }

    ~LectureNoteJouee() {
        Logger::log("[LectureNoteJouee] Instance détruite");
        fermer();
    }

    /**
     * @brief Initialise le périphérique MIDI
     *
     * Configure le périphérique MIDI pour la lecture des messages.
     * @return true si l'initialisation reussit, false sinon.
     */
    bool initialiser();

    /**
     * @brief Lit une ou plusieurs notes jouées
     *
     * Bloque jusqu'à ce qu'une note ou un accord soit disponible.
     * @return Un vecteur de chaînes représentant les notes jouées.
     */
    std::vector<std::string> lireNote();

    /**
     * @brief Libère les ressources MIDI
     */
    void fermer();

    /**
     * @brief Tester la convertion des notes
     */
    std::string testerConvertirNote(int noteMidi) {
        return convertirNote(noteMidi);
    }

    /**
     * @brief Tester la lecture du dernier accord
     */
    std::vector<std::string> getDernierAccord() { return dernierAccord; }
};

#endif // LECTURENOTEJOUEE_H
