#include <print>
#include <string>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "Logger.hpp"
#include <doctest/doctest.h>
#include <filesystem>
#include <fstream>

/// Vérifie les fonctionnalités du système de journalisation
/// Test initialisation, écriture logs normaux/erreur, rotation automatique
TEST_CASE("Logger functionality") {
    std::string basicLog = "test_basic.log";
    std::string errorLog = "test_error.log";
    // Nettoyage avant test
    if (std::filesystem::exists(basicLog)) std::filesystem::remove(basicLog);
    if (std::filesystem::exists(errorLog)) std::filesystem::remove(errorLog);

    /// Vérifie l'initialisation par défaut avec fichiers smartpiano.log
    SUBCASE("Default init") {
        std::string defaultLog = "smartpiano.log";
        std::string defaultErr = "smartpiano.err.log";
        if (std::filesystem::exists(defaultLog))
            std::filesystem::remove(defaultLog);
        if (std::filesystem::exists(defaultErr))
            std::filesystem::remove(defaultErr);

        Logger::init();
        Logger::log("Default log test");

        CHECK(std::filesystem::exists(defaultLog));
        CHECK(std::filesystem::exists(defaultErr));

        std::filesystem::remove(defaultLog);
        std::filesystem::remove(defaultErr);
    }

    /// Vérifie l'initialisation explicite et la journalisation basique
    SUBCASE("Explicit init and basic logging") {
        Logger::init(basicLog, errorLog);

        /// Vérifie l'écriture d'un message basique avec timestamp
        SUBCASE("Log basic message") {
            Logger::log("Test basic message");
            std::ifstream f(basicLog);
            CHECK(f.is_open());
            std::string line;
            std::getline(f, line);
            CHECK(line.find("Test basic message") != std::string::npos);
            CHECK(line.find("[") != std::string::npos); // Timestamp check
        }
        /// Vérifie l'écriture d'un message d'erreur dans le fichier dédié
        SUBCASE("Log error message") {
            Logger::err("Test error message");
            std::ifstream f(errorLog);
            CHECK(f.is_open());
            std::string line;
            std::getline(f, line);
            CHECK(line.find("Test error message") != std::string::npos);
        }

        /// Vérifie le formatage des messages d'erreur avec paramètres
        SUBCASE("Log error message with format") {
            Logger::err("Error code: {}", 404);
            std::ifstream f(errorLog);
            CHECK(f.is_open());
            std::string line;
            // Read until end to find our message (append mode)
            while (std::getline(f, line)) {
                if (line.find("Error code: 404") != std::string::npos) break;
            }
            CHECK(line.find("Error code: 404") != std::string::npos);
        }
    }

    /// Vérifie la rotation automatique des logs dépassant 2Mo
    SUBCASE("Log rotation") {
        Logger::init(basicLog, errorLog);

        // Remplir fichier > 2Mo pour déclencher rotation
        {
            std::ofstream f(basicLog);
            std::string largeString(1024 * 1024, 'A'); // 1MB
            f << largeString << largeString << "overflow";
        }

        CHECK(std::filesystem::file_size(basicLog) > 2 * 1024 * 1024);

        Logger::log("Trigger rotation");

        // Vérifier si fichier a été archivé avec date
        // La logique de rotation: rename(filePath, date() + filePath)
        // Il faut obtenir la chaîne de date
        auto now = std::chrono::zoned_time{std::chrono::current_zone(),
                                           std::chrono::system_clock::now()};
        std::string dateStr = std::format("{:%F}", now);
        std::string rotatedFile = dateStr + basicLog;

        CHECK(std::filesystem::exists(rotatedFile));
        CHECK(std::filesystem::file_size(rotatedFile) > 2 * 1024 * 1024);

        // Le nouveau fichier doit être petit (juste le log "Trigger rotation")
        CHECK(std::filesystem::file_size(basicLog) < 1024);

        std::filesystem::remove(rotatedFile);
    }

    /// Vérifie la gestion d'erreur lors d'initialisation avec chemin invalide
    SUBCASE("Init failure (invalid path)") {
        // On ne peut pas facilement tester stderr avec doctest sans redirection
        // mais on s'assure que ça ne plante pas
        // Si on passe un chemin invalide comme un répertoire existant
        std::filesystem::create_directory("invalid_dir");
        Logger::init("invalid_dir", "invalid_dir");

        // Doit afficher sur stderr mais pas planter
        // La couverture devrait atteindre le chemin d'erreur dans init

        Logger::log("Should not crash");
        Logger::err("Should not crash");

        std::filesystem::remove("invalid_dir");
    }

    /// Vérifie l'échec de rotation lorsque répertoire en lecture seule
    SUBCASE("Log rotation failure (readonly directory)") {
        // Test ligne 50-51: échec de recréation fichier durant rotation
        Logger::init(basicLog, errorLog);
        // Créer un gros fichier log
        {
            std::ofstream f(basicLog);
            std::println(f, "{}", std::string(2 * 1024 * 1024, 'A'));
        }
        // Rendre répertoire lecture seule pour causer échec rotation
        std::filesystem::permissions(std::filesystem::current_path(),
                                     std::filesystem::perms::owner_read |
                                         std::filesystem::perms::owner_exec,
                                     std::filesystem::perm_options::replace);
        CHECK_THROWS(Logger::log("Should trigger failed rotation"));
        // Restaurer permissions
        std::filesystem::permissions(std::filesystem::current_path(),
                                     std::filesystem::perms::owner_all,
                                     std::filesystem::perm_options::replace);
    }
    // Nettoyage après tests
    if (std::filesystem::exists(basicLog)) std::filesystem::remove(basicLog);
    if (std::filesystem::exists(errorLog)) std::filesystem::remove(errorLog);
}
