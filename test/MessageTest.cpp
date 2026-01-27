#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "Message.hpp"
#include <doctest/doctest.h>

TEST_CASE("Message construction and fields") {
    SUBCASE("Constructor with type only") {
        Message m("TEST_TYPE");
        CHECK(m.type == "TEST_TYPE");
        CHECK(m.fields.empty());
    }

    SUBCASE("Constructor with fields") {
        std::map<std::string, std::string> fields;
        fields["key1"] = "value1";
        Message m("TEST_TYPE", fields);
        CHECK(m.type == "TEST_TYPE");
        CHECK(m.getField("key1") == "value1");
    }
}

TEST_CASE("Message field manipulation") {
    Message m("DATA");

    m.addField("id", "123");
    m.addField("content", "hello");

    CHECK(m.hasField("id"));
    CHECK(m.hasField("content"));
    CHECK_FALSE(m.hasField("missing"));

    CHECK(m.getField("id") == "123");
    CHECK(m.getField("content") == "hello");
    CHECK(m.getField("missing") == ""); // Default return
}