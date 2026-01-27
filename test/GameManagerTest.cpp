#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "GameManager.hpp"
#include <doctest/doctest.h>

TEST_CASE("GameManager instantiation") {
    GameManager gm;
    CHECK(true);
}
