#ifndef LOGGER_H
#define LOGGER_H

#include <chrono>
#include <cstdio>
#include <filesystem>
#include <format>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <print>
#include <sstream>
#include <string>

class Logger {
  private:
    static inline std::string logFilePath{"smartpiano.log"}; ///< Log standard
    static inline std::string errFilePath{"smartpiano.err.log"}; ///< Erreurs
    static inline std::mutex logMutex; ///< Mutex accès thread-safe
    static constexpr uintmax_t MAX_LOG_SIZE{2 * 1024 * 1024}; ///< Maxi (2 Mo)

  private:
    /**
     * @brief Retourne horodatage formaté
     * @return Horodatage "YYYY-MM-DD_HHhMMmSSsmmm"
     */
    static std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto timeT = std::chrono::system_clock::to_time_t(now);
        auto timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                          now.time_since_epoch()) %
                      1000;
        std::ostringstream timestamp;
        timestamp << std::put_time(std::localtime(&timeT), "%Y-%m-%d_%Hh%Mm%Ss")
                  << std::setw(3) << std::setfill('0') << timeMs.count();
        return timestamp.str();
    }

    /**
     * @brief Gère la rotation des fichiers de log
     * @param filePath Chemin du fichier à faire tourner
     */
    static void rotateLog(const std::string& filePath) {
        // Renomme fichier actuel comme sauvegarde
        std::filesystem::rename(filePath, getCurrentTimestamp() + filePath);
        // Crée nouveau fichier vide
        std::ofstream newFile(filePath, std::ios::trunc);
        if (!newFile.is_open())
            std::println(stderr, "[Logger] Impossible de recréer fichier");
    }

    /**
     * @brief Écrit un message dans le fichier de log approprié
     * @param message Message à écrire
     * @param isError true pour log d'erreurs, false pour log standard
     */
    static void writeLog(const std::string& message, std::string& path) {
        std::lock_guard<std::mutex> lock(logMutex);
        if (std::filesystem::exists(path) &&
            std::filesystem::file_size(path) > MAX_LOG_SIZE)
            rotateLog(path); // Rotation des logs si nécessaire
        // Écrit message dans fichier approprié
        std::ofstream file(path, std::ios::app);
        if (file.is_open())
            std::println(file, "[{}] {}", getCurrentTimestamp(), message);
        else {
            std::println(stderr, "[Logger] Impossible d'écrire dans fichier");
            std::println(stderr, "[{}] {}", getCurrentTimestamp(), message);
        }
    }

  public:
    /**
     * @brief Initialise le mutex et vérifie que les fichiers peuvent être créés
     */
    static void init() {
        std::lock_guard<std::mutex> lock(logMutex);
        // Vérifie que les fichiers peuvent être créés
        std::ofstream basicFile(Logger::logFilePath, std::ios::app);
        std::ofstream errorFile(Logger::errFilePath, std::ios::app);
        if (!basicFile.is_open() || !errorFile.is_open())
            std::println(stderr,
                         "[Logger] Impossible de créer les fichiers de log");
    }

    /**
     * @brief Initialise avec des chemins des fichiers de log spécifiques
     * @param logPath Chemin du fichier de log standard
     * @param errPath Chemin du fichier de log d'erreurs
     */
    static void init(const std::string& logPath, const std::string& errPath) {
        Logger::logFilePath = logPath;
        Logger::errFilePath = errPath;
        Logger::init();
    }

    /**
     * @brief Écrit un message dans le fichier de log standard
     * @tparam Args Types des arguments de formatage
     * @param fmt Chaîne de formatage
     * @param args Arguments de formatage
     */
    template <typename... Args>
    static void log(std::format_string<Args...> fmt, Args&&... args) {
        writeLog(std::format(fmt, std::forward<Args>(args)...),
                 Logger::logFilePath);
    }

    /**
     * @brief Écrit un message dans le fichier de log d'erreurs
     * @tparam Args Types des arguments de formatage
     * @param fmt Chaîne de formatage
     * @param args Arguments de formatage
     */
    template <typename... Args>
    static void err(std::format_string<Args...> fmt, Args&&... args) {
        writeLog(std::format(fmt, std::forward<Args>(args)...),
                 Logger::errFilePath);
    }
};

#endif // LOGGER_H
