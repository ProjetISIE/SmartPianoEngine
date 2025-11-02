#ifndef BASEACCORDS_H
#define BASEACCORDS_H

#include <map>
#include <string>
#include <vector>

// Classe BaseAccords : contient les informations sur les accords pour
// differentes tonalites
class BaseAccords {
  public:
    // Constructeur : initialise les accords disponibles
    BaseAccords();

    /**
     * @brief Obtenir l'accord associe a une tonalite et un degre
     *
     * @param tonalite Tonalite musicale (ex: "Do", "Re")
     * @param degre Degre de l'accord dans la tonalite (ex: "I", "II", "III")
     * @return Un vecteur d'entiers representant les notes MIDI de l'accord
     */
    std::vector<int> obtenirAccord(const std::string& tonalite,
                                   const std::string& degre) const;

    // Map contenant les accords pour chaque tonalite et degre
    // Structure : accords["Tonalite"]["Degre"] = {Notes MIDI}
    std::map<std::string, std::map<std::string, std::vector<int>>> accords;

  private:
    // Methode pour initialiser les accords avec leurs notes correspondantes
    void initialiserAccords();
};

#endif // BASEACCORDS_H
