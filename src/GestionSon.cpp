// Classe inutilisée, gardée pour référence
#include "GestionSon.hpp"
#include "Logger.hpp"
#include <cstdlib>

// Constructeur de la classe GestionSon
GestionSon::GestionSon(QObject* parent) : QObject(parent) {
    Logger::log("[GestionSon] ligne 8 : Instance de GestionSon creee");
}

// Fonction pour jouer un fichier audio en fonction de la note
void GestionSon::jouerSon(const std::string& note) {
    Logger::log(
        "[GestionSon] ligne 13 : Requete pour jouer le son de la note " + note);

    // Construction du chemin complet du fichier audio
    std::string cheminFichier = "./sounds/" + note + ".wav";
    Logger::log("[GestionSon] ligne 17 : Chemin du fichier audio utilise : " +
                cheminFichier);

    // Commande pour jouer le son avec aplay
    std::string commande = "aplay \"" + cheminFichier + "\" > /dev/null 2>&1";

    // Execution de la commande
    int resultat = std::system(commande.c_str());
    if (resultat != 0) {
        Logger::log("[GestionSon] ligne 25 : Erreur : Impossible de lire le "
                    "fichier audio : " +
                        cheminFichier,
                    true);
    } else {
        Logger::log(
            "[GestionSon] ligne 27 : Lecture du son terminee avec succes : " +
            cheminFichier);
    }
}
