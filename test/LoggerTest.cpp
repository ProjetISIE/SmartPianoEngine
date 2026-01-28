#include <print>
#include <string>
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "Logger.hpp"
#include <doctest/doctest.h>
#include <filesystem>
#include <fstream>

TEST_CASE("Logger functionality") {
    std::string basicLog = "test_basic.log";
    std::string errorLog = "test_error.log";
    // Cleanup before test
    if (std::filesystem::exists(basicLog)) std::filesystem::remove(basicLog);
    if (std::filesystem::exists(errorLog)) std::filesystem::remove(errorLog);

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

    SUBCASE("Explicit init and basic logging") {
        Logger::init(basicLog, errorLog);

        SUBCASE("Log basic message") {
            Logger::log("Test basic message");
            std::ifstream f(basicLog);
            CHECK(f.is_open());
            std::string line;
            std::getline(f, line);
            CHECK(line.find("Test basic message") != std::string::npos);
            CHECK(line.find("[") != std::string::npos); // Timestamp check
        }
        SUBCASE("Log error message") {
            Logger::err("Test error message");
            std::ifstream f(errorLog);
            CHECK(f.is_open());
            std::string line;
            std::getline(f, line);
            CHECK(line.find("Test error message") != std::string::npos);
        }

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

    SUBCASE("Log rotation") {
        Logger::init(basicLog, errorLog);

        // Fill file to > 2MB
        {
            std::ofstream f(basicLog);
            std::string largeString(1024 * 1024, 'A'); // 1MB
            f << largeString << largeString << "overflow";
        }

        CHECK(std::filesystem::file_size(basicLog) > 2 * 1024 * 1024);

        Logger::log("Trigger rotation");

        // Check if rotated
        // The rotation logic is: rename(filePath, date() + filePath)
        // We need the date string
        auto now = std::chrono::zoned_time{std::chrono::current_zone(),
                                           std::chrono::system_clock::now()};
        std::string dateStr = std::format("{:%F}", now);
        std::string rotatedFile = dateStr + basicLog;

        CHECK(std::filesystem::exists(rotatedFile));
        CHECK(std::filesystem::file_size(rotatedFile) > 2 * 1024 * 1024);

        // New file should be small (just the "Trigger rotation" log)
        CHECK(std::filesystem::file_size(basicLog) < 1024);

        std::filesystem::remove(rotatedFile);
    }

    SUBCASE("Init failure (invalid path)") {
        // We can't easily assert on stderr output with doctest unless we
        // redirect it, but we can ensure it doesn't crash and maybe check if
        // files are NOT created or opened. If we pass an invalid path like a
        // directory that exists
        std::filesystem::create_directory("invalid_dir");
        Logger::init("invalid_dir", "invalid_dir");

        // It should print to stderr but not crash.
        // Coverage should hit the error path in init.

        Logger::log("Should not crash");
        Logger::err("Should not crash");

        std::filesystem::remove("invalid_dir");
    }

    SUBCASE("Log rotation failure (readonly directory)") {
        // Test line 50-51: file recreation failure during rotation
        Logger::init(basicLog, errorLog);
        // Create a large log file
        {
            std::ofstream f(basicLog);
            std::println(f, "{}", std::string(2 * 1024 * 1024, 'A'));
        }
        // Make the directory read-only to cause rotation failure
        std::filesystem::permissions(std::filesystem::current_path(),
                                     std::filesystem::perms::owner_read |
                                         std::filesystem::perms::owner_exec,
                                     std::filesystem::perm_options::replace);
        CHECK_THROWS(Logger::log("Should trigger failed rotation"));
        // Restore permissions
        std::filesystem::permissions(std::filesystem::current_path(),
                                     std::filesystem::perms::owner_all,
                                     std::filesystem::perm_options::replace);
    }
    // Cleanup after test
    if (std::filesystem::exists(basicLog)) std::filesystem::remove(basicLog);
    if (std::filesystem::exists(errorLog)) std::filesystem::remove(errorLog);
}
