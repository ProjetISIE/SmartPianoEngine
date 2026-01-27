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
    static inline std::string basicLogFilePath; ///< Chemin fichier log standard
    static inline std::string errorLogFilePath; ///< Chemin fichier log erreurs
    static inline std::mutex logMutex;          ///< Mutex accès thread-safe
    static constexpr uintmax_t MAX_LOG_SIZE{2 * 1024 * 1024}; ///< Maxi (2 Mo)

  private:
    /**
     * @brief Écrit un message dans le fichier de log approprié
     * @param message Message à écrire
     * @param isError true pour log d'erreurs, false pour log standard
     */
    static void writeLog(const std::string& message, std::string& logFilePath) {
        std::lock_guard<std::mutex> lock(logMutex);
        if (logFilePath.empty()) {
            std::println(stderr, "[Logger] Fichier de log non initialisé");
            std::println("[{}] {}", getCurrentTimestamp(), message);
            return;
        }
        // Vérifie taille et rotation si nécessaire
        if (std::filesystem::exists(logFilePath)) {
            auto fileSize = std::filesystem::file_size(logFilePath);
            if (fileSize > MAX_LOG_SIZE) rotateLog(logFilePath);
        }
        // Écrit message dans fichier approprié
        std::ofstream file(logFilePath, std::ios::app);
        if (file.is_open())
            std::println(file, "[{}] {}", getCurrentTimestamp(), message);
        else std::println(stderr, "[Logger] Impossible d'écrire dans fichier");
    }

    /**
     * @brief Gère la rotation des fichiers de log
     * @param filePath Chemin du fichier à faire tourner
     */
    static void rotateLog(const std::string& filePath) {
        std::string backupPath = filePath + ".backup";
        // Supprime ancienne sauvegarde si existe
        if (std::filesystem::exists(backupPath))
            std::filesystem::remove(backupPath);
        // Renomme fichier actuel comme sauvegarde
        std::filesystem::rename(filePath, backupPath);
        // Crée nouveau fichier vide
        std::ofstream newFile(filePath, std::ios::trunc);
        if (!newFile.is_open())
            std::println(stderr, "[Logger] Impossible de recréer fichier");
    }

    /**
     * @brief Retourne horodatage formaté
     * @return Horodatage "YYYY-MM-DD HH:MM:SS.mmm"
     */
    static std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto timeT = std::chrono::system_clock::to_time_t(now);
        auto timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                          now.time_since_epoch()) %
                      1000;
        std::ostringstream timestamp;
        timestamp << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S")
                  << "." << std::setw(3) << std::setfill('0') << timeMs.count();
        return timestamp.str();
    }

  public:
    /**
     * @brief Initialise les chemins des fichiers de log
     * @param logPath Chemin du fichier de log standard
     * @param errPath Chemin du fichier de log d'erreurs
     */
    static void init(const std::string& logPath, const std::string& errPath) {
        basicLogFilePath = logPath;
        errorLogFilePath = errPath;
        std::lock_guard<std::mutex> lock(logMutex);
        // Vérifie que les fichiers peuvent être créés
        std::ofstream basicFile(basicLogFilePath, std::ios::app);
        std::ofstream errorFile(errorLogFilePath, std::ios::app);
        if (!basicFile.is_open() || !errorFile.is_open())
            std::println(stderr,
                         "[Logger] Impossible de créer les fichiers de log");
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
                 basicLogFilePath);
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
                 errorLogFilePath);
    }
};

#endif // LOGGER_H
