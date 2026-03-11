#ifndef NOTEGAME_HPP
#define NOTEGAME_HPP

#include "AnswerValidator.hpp"
#include "ChallengeFactory.hpp"
#include "IGameMode.hpp"
#include "IMidiInput.hpp"
#include "ITransport.hpp"
#include <random>
#include <string>
#include <vector>

/**
 * @brief Jeu de reconnaissance de notes individuelles
 * Le joueur doit jouer la note affichée
 */
class NoteGame : public IGameMode {
  private:
    ITransport& transport;     ///< Référence au transport
    IMidiInput& midi;          ///< Référence à l'entrée MIDI
    ChallengeFactory& factory; ///< Référence à la factory
    GameConfig config;         ///< Configuration du jeu
    AnswerValidator validator; ///< Validateur de réponses
    int challengeId;           ///< ID du challenge actuel

  public:
    /**
     * @brief Constructeur
     * @param transport Transport pour communication
     * @param midi Entrée MIDI
     * @param factory Factory pour générer les challenges
     * @param config Configuration du jeu
     */
    NoteGame(ITransport& transport, IMidiInput& midi, ChallengeFactory& factory,
             const GameConfig& config);

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

#endif // NOTEGAME_HPP
