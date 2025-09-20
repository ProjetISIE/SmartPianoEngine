#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <mutex>
#include <chrono>
#include <iomanip>
#include <sstream>

class Logger {
public:
    // Initialise les chemins des fichiers de log
    static void init(const std::string &basicLogPath, const std::string &errorLogPath) {
        basicLogFilePath = basicLogPath;
        errorLogFilePath = errorLogPath;

        std::lock_guard<std::mutex> lock(logMutex);

        // Vérifie que les fichiers peuvent étre créés
        std::ofstream basicFile(basicLogFilePath, std::ios::app);
        std::ofstream errorFile(errorLogFilePath, std::ios::app);

        if (!basicFile.is_open() || !errorFile.is_open()) {
            std::cerr << "[Logger] Erreur : Impossible de créer les fichiers de log." << std::endl;
        }
    }

    // écrit un message dans le fichier de log approprié
    static void log(const std::string &message, bool isError = false) {
        std::lock_guard<std::mutex> lock(logMutex);

        if (basicLogFilePath.empty() || errorLogFilePath.empty()) {
            std::cerr << "[Logger] Erreur : Les fichiers de log ne sont pas initialisés." << std::endl;
            return;
        }

        const std::string &logFilePath = isError ? errorLogFilePath : basicLogFilePath;

        // Vérifie la taille du fichier de log et applique une rotation si nécessaire
        if (std::filesystem::exists(logFilePath)) {
            auto fileSize = std::filesystem::file_size(logFilePath);
            if (fileSize > MAX_LOG_SIZE) {
                rotateLog(logFilePath);
            }
        }

        // Ajoute un horodatage au message
        std::string timestamp = getCurrentTimestamp();
        std::string logEntry = "[" + timestamp + "] " + message;

        // écrit le message dans le fichier approprié
        std::ofstream file(logFilePath, std::ios::app);
        if (file.is_open()) {
            file << logEntry << std::endl;
        } else {
            std::cerr << "[Logger] Erreur : Impossible d'écrire dans le fichier de log." << std::endl;
        }
    }

private:
    static inline std::string basicLogFilePath; // Chemin du fichier de log basique
    static inline std::string errorLogFilePath; // Chemin du fichier de log des erreurs
    static inline std::mutex logMutex;          // Mutex pour garantir l'accès thread-safe
    static const uintmax_t MAX_LOG_SIZE = 2 * 1024 * 1024; // Taille max en octets (2 Mo)

    // Gère la rotation des fichiers de log
    static void rotateLog(const std::string &filePath) {
        std::string backupPath = filePath + ".backup";

        // Supprime l'ancienne sauvegarde si elle existe
        if (std::filesystem::exists(backupPath)) {
            std::filesystem::remove(backupPath);
        }

        // Renomme le fichier actuel comme sauvegarde
        std::filesystem::rename(filePath, backupPath);

        // Crée un nouveau fichier vide
        std::ofstream newFile(filePath, std::ios::trunc);
        if (!newFile.is_open()) {
            std::cerr << "[Logger] Erreur : Impossible de recréer le fichier de log." << std::endl;
        }
    }

    // Retourne un horodatage formaté : "YYYY-MM-DD HH:MM:SS"
    static std::string getCurrentTimestamp() {
        auto now = std::chrono::system_clock::now();
        auto timeT = std::chrono::system_clock::to_time_t(now);
        auto timeMs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::ostringstream timestamp;
        timestamp << std::put_time(std::localtime(&timeT), "%Y-%m-%d %H:%M:%S")
                  << "." << std::setw(3) << std::setfill('0') << timeMs.count();
        return timestamp.str();
    }
};

#endif // LOGGER_H
