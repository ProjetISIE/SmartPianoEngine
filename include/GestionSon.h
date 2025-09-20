#ifndef GESTIONSON_H
#define GESTIONSON_H

#include <string>
#include <QObject>

/**
 * @brief Classe GestionSon
 * 
 * Cette classe permet de gerer la lecture des sons associes aux notes musicales.
 * Elle utilise un fichier audio specifique pour chaque note.
 */
class GestionSon : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Constructeur de la classe GestionSon
     * 
     * @param parent Pointeur vers l'objet parent (par defaut nullptr).
     */
    explicit GestionSon(QObject *parent = nullptr);

    /**
     * @brief Destructeur de la classe GestionSon
     * 
     * Utilise le destructeur par defaut.
     */
    ~GestionSon() override = default;

    /**
     * @brief Joue un son associe a une note musicale
     * 
     * Cette methode construit le chemin du fichier audio correspondant a la note
     * et le lit en utilisant une commande externe.
     * 
     * @param note Nom de la note (ex: "C4", "D#5").
     */
    void jouerSon(const std::string &note);
};

#endif // GESTIONSON_H
