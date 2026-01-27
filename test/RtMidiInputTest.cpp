#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "RtMidiInput.hpp"
#include <doctest/doctest.h>

TEST_CASE("RtMidiInput initialization") {
    RtMidiInput input;
    // Just check that we can call methods without crash.
    // Initialize will likely fail or return false if no backend.

    bool init = input.initialize();
    if (!init) {
        WARN(
            "RtMidiInput initialization failed (expected if no audio system).");
        CHECK_FALSE(input.isReady());
    } else {
        CHECK(input.isReady());
        input.close();
        CHECK_FALSE(input.isReady());
    }
}
