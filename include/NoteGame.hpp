#ifndef NOTEGAME_HPP
#define NOTEGAME_HPP

#include "IGameMode.hpp"
#include "IMidiInput.hpp"
#include "ITransport.hpp"
#include <random>
#include <string>
#include <vector>

/**
 * @brief Jeu de reconnaissance de notes individuelles
 *
 * Le joueur doit jouer la note affichée
 */
class NoteGame : public IGameMode {
  private:
    ITransport& transport; ///< Référence au transport
    IMidiInput& midi;      ///< Référence à l'entrée MIDI
    GameConfig config;     ///< Configuration du jeu
    std::mt19937 rng;      ///< Générateur aléatoire
    int challengeId;       ///< ID du challenge actuel

  private:
    /**
     * @brief Génère une note aléatoire dans la gamme
     * @return Note générée
     */
    Note generateRandomNote();

    /**
     * @brief Obtient les notes de la gamme configurée
     * @return Vecteur de notes possibles
     */
    std::vector<std::string> getScaleNotes();

  public:
    /**
     * @brief Constructeur
     * @param transport Transport pour communication
     * @param midi Entrée MIDI
     * @param config Configuration du jeu
     */
    NoteGame(ITransport& transport, IMidiInput& midi, const GameConfig& config);

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
