#ifndef VALIDATION_NOTE_H
#define VALIDATION_NOTE_H

#include <string>
#include <vector>

/**
 * @brief Classe ValidationNote
 *
 * Cette classe fournit des methodes pour valider des notes et accords musicaux.
 * Elle permet de comparer des notes jouees avec des notes attendues et de
 * verifier la validite d'accords, y compris avec renversements.
 */
class ValidationNote {
  public:
    /**
     * @brief Valide une note jouee par rapport a une note attendue.
     *
     * @param noteJouee La note jouee (ex: "C4").
     * @param noteAttendue La note attendue (ex: "C4").
     * @return true si la note jouee correspond a la note attendue, false sinon.
     */
    bool valider(const std::string& noteJouee, const std::string& noteAttendue);

    /**
     * @brief Valide un accord sans prendre en compte le renversement.
     *
     * Compare les accords joues et attendus, en ignorant l'ordre des notes.
     *
     * @param accordJoue Vecteur de notes jouees (ex: {"C4", "E4", "G4"}).
     * @param accordAttendu Vecteur de notes attendues (ex: {"C4", "E4", "G4"}).
     * @return true si les deux accords contiennent les memes notes, false
     * sinon.
     */
    bool validerAccordSR(const std::vector<std::string>& accordJoue,
                         const std::vector<std::string>& accordAttendu);

    /**
     * @brief Valide un accord avec prise en compte d'un renversement
     * specifique.
     *
     * Compare les accords joues et attendus en recalibrant les notes selon le
     * renversement specifie.
     *
     * @param accordJoue Vecteur de notes jouees (ex: {"E4", "G4", "C5"}).
     * @param accordAttendu Vecteur de notes attendues (ex: {"C4", "E4", "G4"}).
     * @param renversement Numero du renversement+1 attendu (2 = premier
     * renversement, etc.).ATTENTION renversement = 1 => Pas de renversement,
     * renversement = 2 => 1er renversement et renversement = 3 => 2eme
     * renversement
     * @return true si l'accord joue correspond a l'accord attendu pour le
     * renversement donne, false sinon.
     */
    bool
    validerAccordRenversement(const std::vector<std::string>& accordJoue,
                              const std::vector<std::string>& accordAttendu,
                              int renversement);
};

#endif // VALIDATION_NOTE_H
