#ifndef BASEACCORDS_H
#define BASEACCORDS_H

#include <map>
#include <string>
#include <vector>

/** Informations sur les accords pour différentes tonalités */
class BaseAccords {
  private:
    void initialiserAccords(); ///< Initialise accords + notes correspondantes

  public:
    BaseAccords(); ///< Constructeur, initialise les accords disponibles

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

#endif // BASEACCORDS_H
