#ifndef CHORDREPOSITORY_HPP
#define CHORDREPOSITORY_HPP

#include <map>
#include <string>
#include <vector>

/** Référentiel contenant tous les accords musicaux mappés par tonalité et degré */
class ChordRepository {
  private:
    void initialiserAccords(); ///< Initialise accords + notes correspondantes

  public:
    ChordRepository(); ///< Constructeur, initialise les accords disponibles

    /**
     * @brief Obtenir l'accord associé à une tonalité et un degré
     *
     * @param tonalite Tonalité musicale (ex: "Do", "Re")
     * @param degre Degré de l'accord dans la tonalité (ex: "I", "II", "III")
     * @return Un vecteur d'entiers représentant les notes MIDI de l'accord
     */
    std::vector<int> obtenirAccord(const std::string& tonalite,
                                   const std::string& degre) const;

    // Accords pour chaque tonalité/degré: accords["Tonalite"]["Degre"]={Notes}
    std::map<std::string, std::map<std::string, std::vector<int>>> accords;
};

#endif // CHORDREPOSITORY_HPP
