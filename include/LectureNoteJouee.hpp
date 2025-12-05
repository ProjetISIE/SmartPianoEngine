#ifndef LECTURENOTEJOUEE_H
#define LECTURENOTEJOUEE_H

#include <atomic>
#include <mutex>
#include <rtmidi/RtMidi.h>
#include <string>
#include <vector>

typedef struct tsf tsf;
#include <miniaudio/miniaudio.h>

/**
 * @brief Classe LectureNoteJouee
 * Lit les événements MIDI depuis un clavier physique
 * et synthétise le son (piano) en interne via TinySoundFont et Miniaudio
 */
class LectureNoteJouee {
  private:
    RtMidiIn* midiIn;         ///< Entrée MIDI (Clavier physique)
    tsf* g_tsf;               ///< L'instance du synthétiseur TinySoundFont
    ma_device device;         ///< Le périphérique audio Miniaudio
    bool audioInitialized;    ///< Flag pour savoir si l'audio est actif
    std::string derniereNote; ///< Dernière note jouée
    std::vector<std::string> dernierAccord; ///< Dernier accord joué
    std::atomic<bool> noteDisponible;       ///< Jeu peut lire un résultat ?
    std::mutex noteMutex; ///< Mutex pour protéger l'accès aux données du jeu

  public:
    /**
     * @brief Constructeur de la classe LectureNoteJouee
     */
    LectureNoteJouee();

    /**
     * @brief Destructeur de la classe LectureNoteJouee
     */
    ~LectureNoteJouee();

    /**
     * @brief Initialise le moteur audio et l'entrée MIDI
     *
     * Charge le SoundFont, démarre le driver audio (Miniaudio) et ouvre
     * le port d'entrée MIDI.
     * @return true si tout s'est bien passé, false sinon.
     */
    bool initialiser();

    /**
     * @brief Lit une ou plusieurs notes jouées (Interface Jeu)
     *
     * Bloque jusqu'à ce qu'une note ou un accord soit disponible.
     * Cette fonction est utilisée par le GameManager.
     * @return Un vecteur de chaînes représentant les notes jouées.
     */
    std::vector<std::string> lireNote();

    /**
     * @brief Libère les ressources (Audio et MIDI)
     */
    void fermer();

    /**
     * @brief Teste la conversion (Debug)
     */
    std::string testerConvertirNote(int noteMidi) {
        return convertirNote(noteMidi);
    }

    /**
     * @brief Accesseur pour les tests unitaires
     */
    std::vector<std::string> getDernierAccord() { return dernierAccord; }

  private:
    /**
     * @brief Callback audio statique appelé par Miniaudio
     * C'est ici que les échantillons sonores sont générés.
     */
    static void data_callback(ma_device* pDevice, void* pOutput,
                              const void* pInput, ma_uint32 frameCount);

    /**
     * @brief Thread de traitement MIDI
     *
     * Lit les messages MIDI entrants, déclenche le son dans le synthé,
     * et met à jour l'état du jeu (notes jouées).
     */
    void traiterMessagesMIDI();

  protected:
    /**
     * @brief Convertis une note MIDI en notation musicale (ex: 60 -> "C4")
     */
    std::string convertirNote(int noteMidi);
};

#endif // LECTURENOTEJOUEE_H
