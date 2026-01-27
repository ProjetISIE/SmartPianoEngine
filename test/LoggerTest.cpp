#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "Logger.hpp"
#include <doctest/doctest.h>
#include <filesystem>
#include <fstream>

TEST_CASE("Logger functionality") {
    std::string basicLog = "test_basic.log";
    std::string errorLog = "test_error.log";

    // Clean up before test
    if (std::filesystem::exists(basicLog)) std::filesystem::remove(basicLog);
    if (std::filesystem::exists(errorLog)) std::filesystem::remove(errorLog);

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
        Logger::log("Test error message", true);

        std::ifstream f(errorLog);
        CHECK(f.is_open());
        std::string line;
        std::getline(f, line);
        CHECK(line.find("Test error message") != std::string::npos);
    }

    // Clean up after test
    if (std::filesystem::exists(basicLog)) std::filesystem::remove(basicLog);
    if (std::filesystem::exists(errorLog)) std::filesystem::remove(errorLog);
}