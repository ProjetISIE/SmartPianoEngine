#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include "GenererNoteAleatoire.hpp"
#include "LectureNoteJouee.hpp"
#include "Logger.hpp"
#include "SocketManager.hpp"
#include "ValidationNote.hpp"

// Classe représentant l’essentiel de l’état de Smart Piano
class GameManager {
  private:
    SocketManager socketManager;     ///< Communication avec le client
    GenererNoteAleatoire generateur; ///< Génération notes et accords aléatoires
    ValidationNote validateur;       ///< Validation des notes et accords joués
    LectureNoteJouee lectureNote;    ///< Lecture des notes jouées par le client
    std::string jeuActuel;     ///< Type de jeu en cours (ex: "Jeu de note")
    std::string gammeActuelle; ///< Gamme musicale utilisée (ex: "Do")
    std::string modeActuel;    ///< Mode utilisé (ex: "Majeur" ou "Mineur")

  protected:
    /** Lancer le "Jeu de note" */
    void lancerJeuDeNote(const std::string& gamme, const std::string& mode);

    /** Lancer le "Jeu d'accords simples" */
    void lancerJeuDaccordSR(const std::string& gamme, const std::string& mode);

    /** Lancer le "Jeu d'accords renversés" */
    void lancerJeuDaccordRenversement(const std::string& gamme,
                                      const std::string& mode);

  public:
    /** Constructeur par défaut, ajout log */
    GameManager() {
        Logger::log("[GameManager] Initialisation de GameManager");
    }

    /** Initialiser le serveur sur un socket Unix */
    bool initialiserServeur(const std::string& socketPath) {
        Logger::log("[GameManager] Initialisation serveur sur {}", socketPath);
        return socketManager.initialiserServeur();
    }

    /** Attendre une connexion client */
    void attendreConnexion() {
        Logger::log("[GameManager] En attente de connexion client");
        socketManager.attendreConnexion();
    }

    /** Lancer le jeu */
    void lancerJeu();

    /** Relancer le jeu avec des paramètres définis */
    void rejouer(const std::string& gamme, const std::string& mode);

    /** Remettre le jeu à l'état initial "retourner à l'accueil" */
    void retourAccueil();

    /** Redémarrer le programme */
    void restartProgram();
};

#endif // GAME_MANAGER_H
