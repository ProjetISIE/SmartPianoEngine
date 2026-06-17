#ifndef CHALLENGEFACTORY_HPP
#define CHALLENGEFACTORY_HPP

#include <map>
#include <random>
#include <string>
#include <vector>

/**
 * @brief Classe ChallengeFactory
 * Centralise la génération aléatoire de notes et d'accords
 * harmonisée avec le protocole (c, d, e, f, g, a, b / maj, min)
 */
class ChallengeFactory {
  private:
    std::mt19937 rng; ///< Générateur de nombres aléatoires
    int lastChordIdx{-1};
    int currentChordIdx{-1}; ///< Index de l'accord en cours
    int lastOctave{-1};

    // Matrices pour le modèle de génération (Markov) et Répétition Espacée (SR)
    std::vector<std::vector<double>> markovMatrix;
    std::vector<std::vector<double>> srMultiplier;

    void initMarkovAndSR();

    // Tables de gammes harmonisées avec le protocole
    static const std::map<std::string, std::vector<std::string>> scales;

  public:
    ChallengeFactory();

    /**
     * @brief Retour sur l'exercice précédent pour adapter la répétition espacée
     * @param success true si l'exercice a été réussi, false si échec
     */
    void feedbackLastChallenge(bool success);

    /**
     * @brief Génère une note aléatoire dans une gamme et un mode
     * @param scale Gamme (c, d, e, f, g, a, b)
     * @param mode Mode (maj, min)
     * @return Note au format string (ex: "c4")
     */
    std::string generateNote(const std::string& scale, const std::string& mode);

    /**
     * @brief Génère un accord aléatoire
     * @param scale Gamme (c, d, e, f, g, a, b)
     * @param mode Mode (maj, min)
     * @return Paire {nom de l'accord, vecteur de notes}
     */
    std::pair<std::string, std::vector<std::string>>
    generateChord(const std::string& scale, const std::string& mode);

    /**
     * @brief Génère un accord avec renversement
     * @param scale Gamme (c, d, e, f, g, a, b)
     * @param mode Mode (maj, min)
     * @return Tuple {nom, notes, renversement(1-3)}
     */
    std::tuple<std::string, std::vector<std::string>, int>
    generateInversedChord(const std::string& scale, const std::string& mode);

    /**
     * @brief Récupère les notes d'une gamme
     */
    static std::vector<std::string> getScaleNotes(const std::string& scale,
                                                  const std::string& mode);
};

#endif // CHALLENGEFACTORY_HPP
