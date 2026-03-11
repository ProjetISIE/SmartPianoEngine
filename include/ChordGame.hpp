#ifndef CHORDGAME_HPP
#define CHORDGAME_HPP

#include "AnswerValidator.hpp"
#include "ChallengeFactory.hpp"
#include "IGameMode.hpp"
#include "IMidiInput.hpp"
#include "ITransport.hpp"
#include <string>
#include <vector>

/**
 * @brief Jeu d'accords simples ou renversés
 * Le joueur doit jouer l'accord affiché
 */
class ChordGame : public IGameMode {
  private:
    ITransport& transport;     ///< Référence au transport
    IMidiInput& midi;          ///< Référence à l'entrée MIDI
    ChallengeFactory& factory; ///< Référence à la factory
    GameConfig config;         ///< Configuration du jeu
    AnswerValidator validator; ///< Validateur de réponses
    bool withInversions;       ///< Mode avec renversements?
    int challengeId;           ///< ID du challenge actuel

  public:
    /**
     * @brief Constructeur
     * @param transport Transport pour communication
     * @param midi Entrée MIDI
     * @param factory Factory pour générer les challenges
     * @param config Configuration du jeu
     * @param withInversions true pour mode avec renversements
     */
    ChordGame(ITransport& transport, IMidiInput& midi,
              ChallengeFactory& factory, const GameConfig& config,
              bool withInversions = false);

    /**
     * @brief Démarre le jeu
     */
    void start() override;

    /**
     * @brief Exécute une partie
     * @return Résultat de la partie
     */
    GameResult play() override;

    /**
     * @brief Arrête le jeu
     */
    void stop() override;
};

#endif // CHORDGAME_HPP
