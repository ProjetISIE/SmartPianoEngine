#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "RtMidiInput.hpp"
#include <doctest/doctest.h>

TEST_CASE("RtMidiInput initialization") {
    RtMidiInput input;
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
