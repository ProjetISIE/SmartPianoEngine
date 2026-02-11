#ifndef IGAMEMODE_HPP
#define IGAMEMODE_HPP

#include <stop_token>
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
    std::string gameType{"note"}; ///< Type jeu: "note", "chord", "inversed"
    std::string scale{"c"};       ///< Gamme: "c", "d", "e", "f", "g", "a", "b"
    std::string mode{"maj"};      ///< Mode: "maj", "min"
    int maxChallenges{10};        ///< Nombre maximum de challenges
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
     * @param stopToken Jeton d'arrêt pour interrompre la partie
     * @return Résultat de la partie
     */
    virtual GameResult play(std::stop_token stopToken) = 0;
};

#endif // IGAMEMODE_HPP
