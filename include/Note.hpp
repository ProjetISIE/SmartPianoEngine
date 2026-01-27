#ifndef NOTE_HPP
#define NOTE_HPP

#include <cctype>
#include <stdexcept>
#include <string>

/**
 * @brief Représente une note musicale
 *
 * Format: lettre (a-g) + altération optionnelle (#/b) + octave (0-8)
 * Exemple: c4, d#5, gb3
 */
class Note {
  private:
    std::string name; ///< Nom de la note (ex: "c", "d#", "gb")
    int octave;       ///< Octave (0-8)

  private:
    /**
     * @brief Parse une chaîne en note
     * @param noteStr Chaîne à parser
     * @return true si parsing réussi
     */
    bool parse(const std::string& noteStr) {
        if (noteStr.empty()) return false;
        size_t pos = 0;
        // Lettre de base (a-g)
        if (noteStr[pos] < 'a' || noteStr[pos] > 'g') return false;
        this->name = noteStr[pos++];
        // Altération optionnelle
        if (pos < noteStr.length() &&
            (noteStr[pos] == '#' || noteStr[pos] == 'b')) {
            this->name += noteStr[pos++];
        }
        // Octave
        if (pos >= noteStr.length() || !std::isdigit(noteStr[pos]))
            return false;
        this->octave = noteStr[pos++] - '0';
        // Vérifier qu'il n'y a pas de caractères supplémentaires
        if (pos != noteStr.length()) return false;
        return this->octave >= 0 && this->octave <= 8;
    }

  public:
    /**
     * @brief Constructeur par défaut
     */
    Note() : name("c"), octave(4) {}

    /**
     * @brief Constructeur depuis une chaîne
     * @param noteStr Chaîne représentant la note (ex: "c4", "d#5")
     */
    explicit Note(const std::string& noteStr) {
        if (!parse(noteStr))
            throw std::invalid_argument("Invalid note format: " + noteStr);
    }

    /**
     * @brief Constructeur avec paramètres
     * @param name Nom de la note (ex: "c", "d#", "gb")
     * @param octave Octave (0-8)
     */
    Note(std::string name, int octave) : name(std::move(name)), octave(octave) {
        if (octave < 0 || octave > 8)
            throw std::invalid_argument("Octave must be between 0 and 8");
    }

    /**
     * @brief Retourne la représentation en chaîne de la note
     * @return Chaîne représentant la note (ex: "c4")
     */
    std::string toString() const {
        return this->name + std::to_string(this->octave);
    }

    /**
     * @brief Retourne le nom de la note (sans l'octave)
     * @return Nom de la note (ex: "c", "d#")
     */
    const std::string& getName() const { return this->name; }

    /**
     * @brief Retourne l'octave de la note
     * @return Octave (0-8)
     */
    int getOctave() const { return this->octave; }

    /**
     * @brief Compare deux notes pour l'égalité
     * @param other Autre note
     * @return true si les notes sont identiques
     */
    bool operator==(const Note& other) const {
        return this->name == other.name && this->octave == other.octave;
    }

    /**
     * @brief Compare deux notes pour l'inégalité
     * @param other Autre note
     * @return true si les notes sont différentes
     */
    bool operator!=(const Note& other) const { return !(*this == other); }
};

#endif // NOTE_HPP
