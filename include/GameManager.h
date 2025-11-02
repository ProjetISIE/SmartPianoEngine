#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include "GenererNoteAleatoire.h"
#include "LectureNoteJouee.h"
#include "SocketManager.h"
#include "ValidationNote.h"

// Classe principale pour gerer le jeu
class GameManager {
  public:
    // Constructeur par defaut
    GameManager();

    // Methode pour initialiser le serveur sur un port specifique
    bool initialiserServeur(int port);

    // Methode pour attendre la connexion d'un client
    void attendreConnexion();

    // Methode principale pour lancer le jeu
    void lancerJeu();

    // Methode pour relancer le jeu avec des parametres definis
    void rejouer(const std::string& gamme, const std::string& mode);

    // Methode pour retourner a l'accueil (remet le jeu a l'etat initial)
    void retourAccueil();

    // Methode pour redemarrer le programme
    void restartProgram();

    // Permet d'acceder a socketManager dans les tests unitaires
    SocketManager& getSocketManager() { return socketManager; }

  protected:
    // Methode pour lancer le mode de jeu "Jeu de note"
    void lancerJeuDeNote(const std::string& gamme, const std::string& mode,
                         bool jouerSon);

    // Methode pour lancer le mode de jeu "Jeu d'accords simples"
    void lancerJeuDaccordSR(const std::string& gamme, const std::string& mode,
                            bool jouerSon);

    // Methode pour lancer le mode de jeu "Jeu d'accords avec renversements"
    void lancerJeuDaccordRenversement(const std::string& gamme,
                                      const std::string& mode, bool jouerSon);

  private:
    // Gestion des communications reseau avec le client
    SocketManager socketManager;

    // Generation de notes et d'accords aleatoires
    GenererNoteAleatoire generateur;

    // Validation des notes et accords joues
    ValidationNote validateur;

    // Lecture des notes jouees par le client
    LectureNoteJouee lectureNote;

    // Variables pour stocker les parametres du jeu actuel
    std::string jeuActuel;     // Type de jeu en cours (ex: "Jeu de note")
    std::string gammeActuelle; // Gamme musicale utilisee (ex: "Do")
    std::string modeActuel;    // Mode utilise (ex: "Majeur" ou "Mineur")
};

#endif // GAME_MANAGER_H
