#ifndef LECTURENOTEJOUEE_H
#define LECTURENOTEJOUEE_H

#include <atomic>
#include <mutex>
#include <rtmidi/RtMidi.h>
#include <string>
#include <vector>

/**
 * @brief Classe LectureNoteJouee
 *
 * Cette classe permet de lire les notes ou accords joues sur un peripherique
 * MIDI. Elle gère la connexion au peripherique, la lecture des messages MIDI et
 * leur conversion en notation musicale standard.
 */
class LectureNoteJouee {
  public:
    /**
     * @brief Constructeur de la classe LectureNoteJouee
     *
     * Initialise les membres de la classe et prepare l'environnement MIDI.
     */
    LectureNoteJouee();

    /**
     * @brief Destructeur de la classe LectureNoteJouee
     *
     * Lib�re les ressources MIDI utilis�es.
     */
    ~LectureNoteJouee();

    /**
     * @brief Initialise le peripherique MIDI
     *
     * Configure le peripherique MIDI pour la lecture des messages.
     * @param jouerSonON Paramètre pour jouer ou non le son .wav des notes.
     * @return true si l'initialisation reussit, false sinon.
     */
    bool initialiser(bool jouerSonON);

    /**
     * @brief Lit une ou plusieurs notes jouees
     *
     * Bloque jusqu'a ce qu'une note ou un accord soit disponible.
     * @return Un vecteur de chaines representant les notes jouees.
     */
    std::vector<std::string> lireNote();

    /**
     * @brief Libere les ressources MIDI
     *
     * Ferme la connexion au peripherique MIDI et nettoie les ressources.
     */
    void fermer();

    /**
     * @brief Teste les APIs MIDI disponibles
     *
     * Affiche les APIs MIDI compilees et les ports disponibles pour debug.
     */
    void test();

    /**
     * @brief Fonction pour les tests unitaires pour tester la convertion des
     * notes
     *
     */
    std::string testerConvertirNote(int noteMidi) {
        return convertirNote(noteMidi);
    }

    /**
     * @brief Fonction pour les tests unitaires pour tester la lecture du
     * dernier accord.
     *
     */
    std::vector<std::string> getDernierAccord() { return dernierAccord; }

  private:
    RtMidiIn*
        midiIn; ///< Pointeur sur l'objet RtMidiIn pour gerer la connexion MIDI
    std::string derniereNote; ///< Derniere note jouee
    bool jouerSonON; ///< Booléen indiquant si il faut jouer le son des notes
                     ///< depuis le programme.

    /**
     * @brief Traite les messages MIDI recus
     *
     * Fonction executee dans un thread separe pour gerer les messages MIDI.
     */
    void traiterMessagesMIDI();

  protected:
    /**
     * @brief Convertit une note MIDI en notation musicale
     *
     * Prend une valeur MIDI et la convertit en une chaine representant la note
     * (ex: "C4" pour le Do de la 4eme octave).
     *
     * @param noteMidi Valeur MIDI de la note
     * @return La note en notation musicale.
     */
    std::string convertirNote(int noteMidi);

    std::vector<std::string> dernierAccord; ///< Dernier accord joue

    std::atomic<bool>
        noteDisponible; ///< Indique si une note ou un accord est disponible

    std::mutex noteMutex; ///< Mutex pour proteger l'acces aux notes
};

#endif // LECTURENOTEJOUEE_H
