#ifndef NOTE_GENERATOR
#define NOTE_GENERATOR

#include <string>
#include <vector>

// Classe pour générer des notes et des accords aléatoires
class NoteGenerator {
  public:
    NoteGenerator(); ///< Constructeur par défaut

    /**
     * @brief Génère une note aléatoire dans une gamme et un mode spécifiques
     * @param gamme La gamme musicale (ex: "Do", "Re"…)
     * @param mode Le mode musical (ex: "Majeur", "Mineur")
     * @return Une chaîne de caractères représentant la note générée (ex: "C4")
     */
    std::string generer(const std::string& gamme, const std::string& mode);

    /**
     * @brief Génère un accord aléatoire dans une gamme et un mode spécifiques
     * @param gamme La gamme musicale (ex: "Do", "Re"…)
     * @param mode Le mode musical (ex: "Majeur", "Mineur")
     * @return Une paire contenant le nom de l'accord et un vecteur de notes
     * (ex: {"Do majeur", {"C4", "E4", "G4"}})
     */
    std::pair<std::string, std::vector<std::string>>
    genererAccord(const std::string& gamme, const std::string& mode);

    /**
     * @brief Génère un accord avec un renversement aléatoire.
     *
     * @param gamme La gamme musicale (ex: "Do", "Re"…).
     * @param mode Le mode musical (ex: "Majeur", "Mineur").
     * @return Un tuple contenant le nom de l'accord, un vecteur de notes et le
     * numéro de (renversement-1) (ex: {"Do majeur", {"E4", "G4", "C5"}, 2 = 3
     * ème renversement})
     */
    std::tuple<std::string, std::vector<std::string>, int>
    genererAccordRenversement(const std::string& gamme,
                              const std::string& mode);
};

#endif // NOTE_GENERATOR
