#ifndef UDSTRANSPORT_HPP
#define UDSTRANSPORT_HPP

#include "ITransport.hpp"
#include <string>

/**
 * @brief Implémentation du transport via Unix Domain Socket
 *
 * Gère la communication via socket Unix en respectant le protocole défini
 */
class UdsTransport : public ITransport {
  public:
    /**
     * @brief Constructeur
     */
    UdsTransport();

    /**
     * @brief Destructeur - nettoie les ressources
     */
    ~UdsTransport() override;

    /**
     * @brief Démarre le serveur UDS
     * @param endpoint Chemin du socket Unix (ex: "/tmp/smartpiano.sock")
     * @return true si démarrage réussi
     */
    bool start(const std::string& endpoint) override;

    /**
     * @brief Attend la connexion d'un client (bloquant)
     */
    void waitForClient() override;

    /**
     * @brief Envoie un message au client
     * @param msg Message à envoyer
     */
    void send(const Message& msg) override;

    /**
     * @brief Reçoit un message du client (bloquant)
     * @return Message reçu
     */
    Message receive() override;

    /**
     * @brief Arrête le serveur
     */
    void stop() override;

    /**
     * @brief Vérifie si un client est connecté
     * @return true si un client est connecté
     */
    bool isClientConnected() const override;

  private:
    int serverSocket_;       ///< Descripteur du socket serveur
    int clientSocket_;       ///< Descripteur du socket client
    std::string socketPath_; ///< Chemin du socket Unix

    /**
     * @brief Sérialise un message en chaîne selon le protocole
     * @param msg Message à sérialiser
     * @return Chaîne sérialisée
     */
    std::string serializeMessage(const Message& msg) const;

    /**
     * @brief Parse une chaîne en message selon le protocole
     * @param data Données à parser
     * @return Message parsé
     */
    Message parseMessage(const std::string& data) const;
};

#endif // UDSTRANSPORT_HPP
