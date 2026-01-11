#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <map>
#include <string>

/**
 * @brief Représente un message échangé via le protocole UDS
 *
 * Un message est composé d'un type et de champs optionnels key=value
 */
struct Message {
    std::string type;                          ///< Type du message
    std::map<std::string, std::string> fields; ///< Champs du message

    /**
     * @brief Constructeur avec type uniquement
     * @param messageType Type du message
     */
    explicit Message(std::string messageType) : type(std::move(messageType)) {}

    /**
     * @brief Constructeur avec type et champs
     * @param messageType Type du message
     * @param messageFields Champs du message
     */
    Message(std::string messageType,
            std::map<std::string, std::string> messageFields)
        : type(std::move(messageType)), fields(std::move(messageFields)) {}

    /**
     * @brief Ajoute un champ au message
     * @param key Clé du champ
     * @param value Valeur du champ
     */
    void addField(const std::string& key, const std::string& value) {
        fields[key] = value;
    }

    /**
     * @brief Récupère la valeur d'un champ
     * @param key Clé du champ
     * @return Valeur du champ ou chaîne vide si inexistant
     */
    std::string getField(const std::string& key) const {
        auto it = fields.find(key);
        return (it != fields.end()) ? it->second : "";
    }

    /**
     * @brief Vérifie si un champ existe
     * @param key Clé du champ
     * @return true si le champ existe
     */
    bool hasField(const std::string& key) const {
        return fields.find(key) != fields.end();
    }
};

#endif // MESSAGE_HPP
