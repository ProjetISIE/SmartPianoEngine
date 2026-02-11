#ifndef CHORDGAME_HPP
#define CHORDGAME_HPP

#include "IGameMode.hpp"
#include "IMidiInput.hpp"
#include "ITransport.hpp"
#include <random>
#include <stop_token>
#include <string>
#include <vector>

/**
 * @brief Jeu d'accords simples (sans renversement)
 * Le joueur doit jouer l'accord affiché dans n'importe quel ordre
 */
class ChordGame : public IGameMode {
  private:
    /**
     * @brief Structure représentant un accord
     */
    struct Chord {
        std::string name;        ///< Nom de l'accord
        std::vector<Note> notes; ///< Notes de l'accord
        int inversion;           ///< Renversement (0, 1, 2)
    };

  private:
    ITransport& transport; ///< Référence au transport
    IMidiInput& midi;      ///< Référence à l'entrée MIDI
    GameConfig config;     ///< Configuration du jeu
    bool withInversions;   ///< Mode avec renversements?
    std::mt19937 rng;      ///< Générateur aléatoire
    int challengeId;       ///< ID du challenge actuel

  private:
    /**
     * @brief Génère un accord aléatoire dans la gamme
     * @return Accord généré
     */
    Chord generateRandomChord();

    /**
     * @brief Obtient les degrés d'accords pour la gamme
     * @return Vecteur de degrés (I, II, III, etc.)
     */
    std::vector<int> getChordDegrees();

    /**
     * @brief Construit un accord à partir d'un degré
     * @param degree Degré de l'accord (1-7)
     * @param octave Octave de base
     * @return Accord construit
     */
    Chord buildChord(int degree, int octave);

    /**
     * @brief Applique un renversement à un accord
     * @param chord Accord de base
     * @param inversion Numéro de renversement (1 ou 2)
     * @return Accord renversé
     */
    Chord applyInversion(const Chord& chord, int inversion);

    /**
     * @brief Vérifie si les notes jouées correspondent à l'accord
     * @param played Notes jouées
     * @param expected Notes attendues
     * @return Nombre de notes correctes
     */
    int validateChord(const std::vector<Note>& played,
                      const std::vector<Note>& expected);

  public:
    /**
     * @brief Constructeur
     * @param transport Transport pour communication
     * @param midi Entrée MIDI
     * @param config Configuration du jeu
     * @param withInversions true pour mode avec renversements
     */
    ChordGame(ITransport& transport, IMidiInput& midi, const GameConfig& config,
              bool withInversions = false);

    /**
     * @brief Démarre le jeu
     */
    void start() override;

    /**
     * @brief Exécute une partie
     * @param stopToken Jeton d'arrêt pour interrompre la partie
     * @return Résultat de la partie
     */
    GameResult play(std::stop_token stopToken) override;
};

#endif // CHORDGAME_HPP
