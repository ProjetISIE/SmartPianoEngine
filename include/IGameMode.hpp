#ifndef IGAMEMODE_HPP
#define IGAMEMODE_HPP

#include <string>

/**
 * @brief Structure contenant le résultat d'une partie
 */
struct GameResult {
    int duration; ///< Durée en millisecondes
    int perfect;  ///< Nombre de challenges parfaits
    int partial;  ///< Nombre de challenges partiels
    int total;    ///< Nombre total de challenges
};

/**
 * @brief Configuration d'une partie
 */
struct GameConfig {
    std::string gameType; ///< Type de jeu: "note", "chord", "inversed"
    std::string scale;    ///< Gamme: "c", "d", "e", "f", "g", "a", "b"
    std::string mode;     ///< Mode: "maj", "min"
    int maxChallenges;    ///< Nombre maximum de challenges
    GameConfig() : maxChallenges(10) {}
};

/**
 * @brief Interface pour les modes de jeu
 * Définit le contrat que tous les modes de jeu doivent respecter
 */
class IGameMode {
  public:
    virtual ~IGameMode() = default;

    /**
     * @brief Démarre le jeu
     */
    virtual void start() = 0;

    /**
     * @brief Exécute une partie complète
     * @return Résultat de la partie
     */
    virtual GameResult play() = 0;

    /**
     * @brief Arrête le jeu
     */
    virtual void stop() = 0;
};

#endif // IGAMEMODE_HPP
