#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "Message.hpp"
#include <doctest/doctest.h>

/// Vérifie la construction correcte des messages avec type et champs optionnels
/// Test les deux constructeurs (avec et sans champs)
TEST_CASE("Message construction and fields") {
    /// Vérifie qu'un message peut être créé avec uniquement un type
    SUBCASE("Constructor with type only") {
        Message m("TEST_TYPE");
        CHECK(m.getType() == "TEST_TYPE");
        CHECK(m.getFields().empty());
    }
    /// Vérifie qu'un message peut être créé avec type et champs clé-valeur
    SUBCASE("Constructor with fields") {
        std::map<std::string, std::string> fields;
        fields["key1"] = "value1";
        Message m("TEST_TYPE", fields);
        CHECK(m.getType() == "TEST_TYPE");
        CHECK(m.getField("key1") == "value1");
    }
}

/// Vérifie l'accès et la vérification de présence des champs d'un message
/// Test hasField, getField avec valeurs présentes et absentes
TEST_CASE("Message field manipulation") {
    Message m("DATA", {{"id", "123"}, {"content", "hello"}});
    CHECK(m.hasField("id"));
    CHECK(m.hasField("content"));
    CHECK_FALSE(m.hasField("missing"));
    CHECK(m.getField("id") == "123");
    CHECK(m.getField("content") == "hello");
    CHECK(m.getField("missing") == ""); // Default return
}
