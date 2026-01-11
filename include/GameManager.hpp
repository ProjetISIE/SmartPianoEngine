#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include "GenererNoteAleatoire.hpp"
#include "LectureNoteJouee.hpp"
#include "Logger.hpp"
#include "SocketManager.hpp"
#include "ValidationNote.hpp"

// Classe représentant l’essentiel de l’état de Smart Piano
class GameManager {
  public:
    // Constructeur par défaut, ajout log
    GameManager() {
        Logger::log("[GameManager] Initialisation de GameManager");
    }

    // Initialiser le serveur sur un socket Unix
    bool initialiserServeur(const std::string& socketPath) {
        Logger::log("[GameManager] Initialisation serveur sur " + socketPath);
        return socketManager.initialiserServeur(socketPath);
    }

    // Attendre une connexion client
    void attendreConnexion() {
        Logger::log("[GameManager] En attente de connexion client");
        socketManager.attendreConnexion();
    }

    // Lancer le jeu
    void lancerJeu();

    // Relancer le jeu, avec des parametres definis
    void rejouer(const std::string& gamme, const std::string& mode);

    // Remettre le jeu a l'état initial "retourner à l'accueil"
    void retourAccueil();

    // Redémarrer le programme
    void restartProgram();

    // Accéder a socketManager (pour les tests unitaires)
    // SocketManager& getSocketManager() { return socketManager; }

  protected:
    // Lancer le "Jeu de note"
    void lancerJeuDeNote(const std::string& gamme, const std::string& mode);

    // Lancer le "Jeu d'accords simples"
    void lancerJeuDaccordSR(const std::string& gamme, const std::string& mode);

    // Lancer le "Jeu d'accords renversés"
    void lancerJeuDaccordRenversement(const std::string& gamme,
                                      const std::string& mode);

  private:
    // Communications réseau avec le client
    SocketManager socketManager;

    // Génération de notes et d'accords aléatoires
    GenererNoteAleatoire generateur;

    // Validation des notes et accords joués
    ValidationNote validateur;

    // Lecture des notes jouées par le client
    LectureNoteJouee lectureNote;

    // Paramètres du jeu actuel
    std::string jeuActuel;     // Type de jeu en cours (ex: "Jeu de note")
    std::string gammeActuelle; // Gamme musicale utilisée (ex: "Do")
    std::string modeActuel;    // Mode utilisé (ex: "Majeur" ou "Mineur")
};

#endif // GAME_MANAGER_H
