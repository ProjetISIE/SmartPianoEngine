#ifndef NOTE_HPP
#define NOTE_HPP

#include <string>
#include <cctype>
#include <stdexcept>

/**
 * @brief Représente une note musicale
 *
 * Format: lettre (a-g) + altération optionnelle (#/b) + octave (0-8)
 * Exemple: c4, d#5, gb3
 */
class Note {
  public:
    /**
     * @brief Constructeur par défaut
     */
    Note() : name_("c"), octave_(4) {}

    /**
     * @brief Constructeur depuis une chaîne
     * @param noteStr Chaîne représentant la note (ex: "c4", "d#5")
     */
    explicit Note(const std::string& noteStr) {
        if (!parse(noteStr)) {
            throw std::invalid_argument("Invalid note format: " + noteStr);
        }
    }

    /**
     * @brief Constructeur avec paramètres
     * @param name Nom de la note (ex: "c", "d#", "gb")
     * @param octave Octave (0-8)
     */
    Note(std::string name, int octave) : name_(std::move(name)), octave_(octave) {
        if (octave < 0 || octave > 8) {
            throw std::invalid_argument("Octave must be between 0 and 8");
        }
    }

    /**
     * @brief Retourne la représentation en chaîne de la note
     * @return Chaîne représentant la note (ex: "c4")
     */
    std::string toString() const {
        return name_ + std::to_string(octave_);
    }

    /**
     * @brief Retourne le nom de la note (sans l'octave)
     * @return Nom de la note (ex: "c", "d#")
     */
    const std::string& getName() const { return name_; }

    /**
     * @brief Retourne l'octave de la note
     * @return Octave (0-8)
     */
    int getOctave() const { return octave_; }

    /**
     * @brief Compare deux notes pour l'égalité
     * @param other Autre note
     * @return true si les notes sont identiques
     */
    bool operator==(const Note& other) const {
        return name_ == other.name_ && octave_ == other.octave_;
    }

    /**
     * @brief Compare deux notes pour l'inégalité
     * @param other Autre note
     * @return true si les notes sont différentes
     */
    bool operator!=(const Note& other) const {
        return !(*this == other);
    }

  private:
    std::string name_;  ///< Nom de la note (ex: "c", "d#", "gb")
    int octave_;        ///< Octave (0-8)

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
        name_ = noteStr[pos++];

        // Altération optionnelle
        if (pos < noteStr.length() && (noteStr[pos] == '#' || noteStr[pos] == 'b')) {
            name_ += noteStr[pos++];
        }

        // Octave
        if (pos >= noteStr.length() || !std::isdigit(noteStr[pos])) return false;
        octave_ = noteStr[pos++] - '0';

        // Vérifier qu'il n'y a pas de caractères supplémentaires
        if (pos != noteStr.length()) return false;

        return octave_ >= 0 && octave_ <= 8;
    }
};

#endif // NOTE_HPP
