#ifndef GENERER_NOTE_ALEATOIRE_H
#define GENERER_NOTE_ALEATOIRE_H

#include <vector>
#include <string>

// Classe pour generer des notes et des accords aleatoires
class GenererNoteAleatoire {
public:
    // Constructeur par defaut
    GenererNoteAleatoire();

    /**
     * @brief Genere une note aleatoire dans une gamme et un mode specifiques.
     * 
     * @param gamme La gamme musicale (ex: "Do", "Re", etc.).
     * @param mode Le mode musical (ex: "Majeur", "Mineur").
     * @return Une chaine de caracteres representant la note generee (ex: "C4").
     */
    std::string generer(const std::string &gamme, const std::string &mode);

    /**
     * @brief Genere un accord aleatoire dans une gamme et un mode specifiques.
     * 
     * @param gamme La gamme musicale (ex: "Do", "Re", etc.).
     * @param mode Le mode musical (ex: "Majeur", "Mineur").
     * @return Une paire contenant le nom de l'accord et un vecteur de notes (ex: {"Do majeur", {"C4", "E4", "G4"}}).
     */
    std::pair<std::string, std::vector<std::string>> genererAccord(const std::string &gamme, const std::string &mode);

    /**
     * @brief Genere un accord avec un renversement aleatoire.
     * 
     * @param gamme La gamme musicale (ex: "Do", "Re", etc.).
     * @param mode Le mode musical (ex: "Majeur", "Mineur").
     * @return Un tuple contenant le nom de l'accord, un vecteur de notes et le numero de (renversement-1) (ex: {"Do majeur", {"E4", "G4", "C5"}, 2 = 3 eme renversement}).
     */
    std::tuple<std::string, std::vector<std::string>, int> genererAccordRenversement(const std::string &gamme, const std::string &mode);
};

#endif // GENERER_NOTE_ALEATOIRE_H
