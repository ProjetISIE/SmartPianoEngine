#ifndef ITRANSPORT_HPP
#define ITRANSPORT_HPP

#include "Message.hpp"
#include <string>

/**
 * @brief Interface de transport pour la communication client-serveur
 *
 * Abstraction permettant différentes implémentations de transport
 * (UDS, TCP, etc.)
 */
class ITransport {
  public:
    virtual ~ITransport() = default;

    /**
     * @brief Démarre le serveur de transport
     * @param endpoint Point de connexion (ex: chemin socket Unix)
     * @return true si démarrage réussi, false sinon
     */
    virtual bool start(const std::string& endpoint) = 0;

    /**
     * @brief Attend la connexion d'un client (bloquant)
     */
    virtual void waitForClient() = 0;

    /**
     * @brief Envoie un message au client connecté
     * @param msg Message à envoyer
     */
    virtual void send(const Message& msg) = 0;

    /**
     * @brief Reçoit un message du client (bloquant)
     * @return Message reçu
     */
    virtual Message receive() = 0;

    /**
     * @brief Arrête le serveur de transport
     */
    virtual void stop() = 0;

    /**
     * @brief Vérifie si un client est connecté
     * @return true si un client est connecté
     */
    virtual bool isClientConnected() const = 0;
};

#endif // ITRANSPORT_HPP
