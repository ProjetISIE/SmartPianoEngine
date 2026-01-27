#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "GameManager.hpp"
#include <doctest/doctest.h>

TEST_CASE("GameManager instantiation") {
    // Just verify it constructs without error
    GameManager gm;
    CHECK(true);
}