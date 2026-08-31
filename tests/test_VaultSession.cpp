/*
 * @brief Unit tests for the VaultSession class.
 *
 * Some tests were written with the assistance of AI
 *
 * @author Nolan Evard
 * @date 31.08.2026
 */
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "VaultSession.h"
#include "entities/CreditCard.h"
#include "entities/Entry.h"
#include "entities/Website.h"
#include "entities/Wifi.h"

#include <chrono>
#include <memory>
#include <string>
#include <utility>

class VaultSessionTest : public ::testing::Test {
  protected:
    EncKey makeEncKey() {
        EncKey key{};
        return key;
    }

    AuthKey makeAuthKey() {
        AuthKey key{};
        return key;
    }

    std::unique_ptr<Persona> makePersona(std::string firstName, std::string lastName) {
        return std::make_unique<Persona>(std::move(firstName), std::move(lastName),
                                         std::chrono::system_clock::time_point{}, "Address",
                                         "000000000");
    }

    std::unique_ptr<Category> makeCategory(std::string name) {
        return std::make_unique<Category>(std::move(name));
    }

    class TestEntry : public Entry {
      public:
        explicit TestEntry(std::string notes = {}) : Entry(std::move(notes)) {}

        std::string getType() const override { return "TestEntry"; }
    };

    std::unique_ptr<Entry> makeEntry(std::string notes) {
        return std::make_unique<TestEntry>(std::move(notes));
    }

    json makeVaultJson() {
        json website = {{"type", "Website"},
                        {"id", 3000},
                        {"creationAt", 1704067200000LL},
                        {"updatedAt", 1704067201000LL},
                        {"notes", "Website notes"},
                        {"title", "Example"},
                        {"comments", "Example comments"},
                        {"username", "alice"},
                        {"password", "website-password"},
                        {"url", "https://example.com"},
                        {"alias", "Example alias"},
                        {"personaId", 4000}};

        json wifi = {{"type", "Wifi"},
                     {"id", 3001},
                     {"creationAt", 1704067200000LL},
                     {"updatedAt", 1704067201000LL},
                     {"notes", "Wi-Fi notes"},
                     {"networkName", "Home Wi-Fi"},
                     {"password", "wifi-password"}};

        json creditCard = {{"type", "CreditCard"},
                           {"id", 3002},
                           {"creationAt", 1704067200000LL},
                           {"updatedAt", 1704067201000LL},
                           {"notes", "Card notes"},
                           {"cardHolderName", "Alice Example"},
                           {"cardNumber", "4111111111111111"},
                           {"expiration", "12/30"},
                           {"securityCode", "123"}};

        return {{"id", 1000},
                {"creationAt", 1704067200000LL},
                {"updatedAt", 1704153600000LL},
                {"name", "Parsed Vault"},
                {"categories",
                 {{{"id", 2000}, {"name", "Passwords"}, {"entries", {website, wifi, creditCard}}}}},
                {"personas",
                 {{{"id", 4000},
                   {"creationAt", 1704067200000LL},
                   {"updatedAt", 1704067201000LL},
                   {"firstName", "Alice"},
                   {"lastName", "Example"},
                   {"dateOfBirth", 946684800000LL},
                   {"address", "Example Street 1"},
                   {"phone", "+41 79 123 45 67"}}}}};
    }
};

// ------------- VaultSession tests --------------------

/*
 * Tests that the VaultSession constructor initializes the name and empty collections correctly.
 */
TEST_F(VaultSessionTest, ConstructorInitializesNameAndEmptyCollections) {
    const auto before = std::chrono::system_clock::now();

    VaultSession session("My Vault", makeEncKey(), makeAuthKey());

    const auto after = std::chrono::system_clock::now();

    EXPECT_EQ(session.getName(), "My Vault");
    EXPECT_TRUE(session.getCategories().empty());
    EXPECT_TRUE(session.getPersonas().empty());

    EXPECT_GE(session.getCreationDate(), before);
    EXPECT_LE(session.getCreationDate(), after);

    EXPECT_GE(session.getLastModifiedDate(), before);
    EXPECT_LE(session.getLastModifiedDate(), after);
}

/*
 * Tests that adding a category appends it to the collection and updates the last modified date.
 */
TEST_F(VaultSessionTest, AddCategoryAppendsCategoryAndUpdatesLastModifiedDate) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
    const auto previousLastModified = session.getLastModifiedDate();

    session.addCategory(makeCategory("Passwords"));

    ASSERT_EQ(session.getCategories().size(), 1U);
    EXPECT_EQ(session.getCategories().front()->getName(), "Passwords");
    EXPECT_GE(session.getLastModifiedDate(), previousLastModified);
}

/*
 * Tests that adding a persona appends it to the collection and updates the last modified date.
 */
TEST_F(VaultSessionTest, AddPersonaAppendsPersonaAndUpdatesLastModifiedDate) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
    const auto previousLastModified = session.getLastModifiedDate();

    session.addPersona(makePersona("Ada", "Lovelace"));

    ASSERT_EQ(session.getPersonas().size(), 1U);
    EXPECT_EQ(session.getPersonas().front()->getFirstName(), "Ada");
    EXPECT_EQ(session.getPersonas().front()->getLastName(), "Lovelace");
    EXPECT_GE(session.getLastModifiedDate(), previousLastModified);
}

/*
 * Tests that adding an entry to a category appends it to the matching category and updates the last
 * modified date.
 */
TEST_F(VaultSessionTest, AddEntryToCategoryAppendsEntryToMatchingCategory) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());

    session.addCategory(makeCategory("Passwords"));

    const auto categoryId = session.getCategories().front()->getId();
    auto entry = makeEntry("gmail");
    const auto previousLastModified = session.getLastModifiedDate();

    session.addEntryToCategory(categoryId, std::move(entry));

    ASSERT_EQ(session.getCategories().size(), 1U);
    ASSERT_EQ(session.getCategories().front()->getEntries().size(), 1U);

    EXPECT_EQ(session.getCategories().front()->getEntries().front()->getNotes(), "gmail");

    EXPECT_GE(session.getLastModifiedDate(), previousLastModified);
}

/*
 * Tests that adding an entry to a category only affects the target category and does not modify
 * other categories.
 */
TEST_F(VaultSessionTest, AddEntryToCategoryOnlyChangesTargetCategory) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());

    session.addCategory(makeCategory("Passwords"));
    session.addCategory(makeCategory("Banking"));

    const auto passwordsCategoryId = session.getCategories()[0]->getId();
    const auto bankingCategoryId = session.getCategories()[1]->getId();

    session.addEntryToCategory(passwordsCategoryId, makeEntry("gmail"));

    ASSERT_EQ(session.getCategories()[0]->getEntries().size(), 1U);

    EXPECT_EQ(session.getCategories()[0]->getEntries().front()->getNotes(), "gmail");

    EXPECT_TRUE(session.getCategories()[1]->getEntries().empty());
    EXPECT_NE(passwordsCategoryId, bankingCategoryId);
}

/*
 * Tests that adding an entry to a missing category throws a CategoryNotFoundError and leaves the
 * state unchanged.
 */
TEST_F(VaultSessionTest, AddEntryToMissingCategoryThrowsAndLeavesStateUnchanged) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());

    session.addCategory(makeCategory("Passwords"));

    const auto lastModifiedBefore = session.getLastModifiedDate();
    const auto missingCategoryId = session.getCategories().front()->getId() + 1;

    EXPECT_THROW(session.addEntryToCategory(missingCategoryId, makeEntry("gmail")),
                 CategoryNotFoundError);

    EXPECT_EQ(session.getCategories().size(), 1U);
    EXPECT_EQ(session.getCategories().front()->getName(), "Passwords");
    EXPECT_TRUE(session.getCategories().front()->getEntries().empty());
    EXPECT_EQ(session.getLastModifiedDate(), lastModifiedBefore);
}

/*
 * Tests that removing a persona removes only the matching persona and leaves others unchanged.
 */
TEST_F(VaultSessionTest, RemovePersonaRemovesOnlyMatchingPersona) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());

    session.addPersona(makePersona("Ada", "Lovelace"));
    session.addPersona(makePersona("Grace", "Hopper"));

    session.removePersona(session.getPersonas().front()->getId());

    ASSERT_EQ(session.getPersonas().size(), 1U);
    EXPECT_EQ(session.getPersonas().front()->getFirstName(), "Grace");
    EXPECT_EQ(session.getPersonas().front()->getLastName(), "Hopper");
}

/*
 * Tests that removing a persona that does not exist leaves the collection unchanged.
 */
TEST_F(VaultSessionTest, RemoveMissingPersonaLeavesCollectionUnchanged) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());

    session.addPersona(makePersona("Ada", "Lovelace"));

    const auto previousLastModified = session.getLastModifiedDate();

    session.removePersona(session.getPersonas().front()->getId() + 1);

    ASSERT_EQ(session.getPersonas().size(), 1U);
    EXPECT_GE(session.getLastModifiedDate(), previousLastModified);
}

// ------------- json parsing tests --------------------

/*
 * Tests that parsing a valid JSON representation of a VaultSession correctly populates the fields.
 */
TEST_F(VaultSessionTest, ParseReadsVaultMetadata) {
    const json input = makeVaultJson();

    const std::string body = input.dump();
    const Bytes bytes{reinterpret_cast<const uint8_t*>(body.data()), body.size()};

    VaultSession session = VaultSession::parse(bytes);

    EXPECT_EQ(session.getName(), "Parsed Vault");
    EXPECT_EQ(session.getCreationDate(), fromUnixMilliseconds(1704067200000LL));
    EXPECT_EQ(session.getLastModifiedDate(), fromUnixMilliseconds(1704153600000LL));
}

/*
 * Tests that parsing a valid JSON representation of a VaultSession correctly populates the
 * categories and personas.
 */
TEST_F(VaultSessionTest, ParseReadsCategoriesAndPersonas) {
    const json input = makeVaultJson();

    const std::string body = input.dump();
    const Bytes bytes{reinterpret_cast<const uint8_t*>(body.data()), body.size()};

    VaultSession session = VaultSession::parse(bytes);

    ASSERT_EQ(session.getCategories().size(), 1U);
    EXPECT_EQ(session.getCategories()[0]->getId(), 2000);
    EXPECT_EQ(session.getCategories()[0]->getName(), "Passwords");

    ASSERT_EQ(session.getPersonas().size(), 1U);
    EXPECT_EQ(session.getPersonas()[0]->getId(), 4000);
    EXPECT_EQ(session.getPersonas()[0]->getFirstName(), "Alice");
    EXPECT_EQ(session.getPersonas()[0]->getLastName(), "Example");
}

/*
 * Tests that parsing a valid JSON representation of a VaultSession correctly populates the entries
 * of the categories, including all supported entry types (Website, Wifi, CreditCard).
 */
TEST_F(VaultSessionTest, ParseReadsAllSupportedEntryTypes) {
    const json input = makeVaultJson();

    const std::string body = input.dump();
    const Bytes bytes{reinterpret_cast<const uint8_t*>(body.data()), body.size()};

    VaultSession session = VaultSession::parse(bytes);

    const auto& entries = session.getCategories()[0]->getEntries();

    ASSERT_EQ(entries.size(), 3U);

    const auto* website = dynamic_cast<const Website*>(entries[0].get());
    ASSERT_NE(website, nullptr);
    EXPECT_EQ(website->getId(), 3000);
    EXPECT_EQ(website->getTitle(), "Example");
    EXPECT_EQ(website->getUsername(), "alice");
    EXPECT_EQ(website->getUrl(), "https://example.com");
    EXPECT_EQ(website->getPersonaId(), 4000);

    const auto* wifi = dynamic_cast<const Wifi*>(entries[1].get());
    ASSERT_NE(wifi, nullptr);
    EXPECT_EQ(wifi->getId(), 3001);
    EXPECT_EQ(wifi->getNetworkName(), "Home Wi-Fi");
    EXPECT_EQ(wifi->getPassword(), "wifi-password");

    const auto* card = dynamic_cast<const CreditCard*>(entries[2].get());
    ASSERT_NE(card, nullptr);
    EXPECT_EQ(card->getId(), 3002);
    EXPECT_EQ(card->getCardHolderName(), "Alice Example");
    EXPECT_EQ(card->getCardNumber(), "4111111111111111");
}

/*
 * Tests that parsing an invalid JSON string throws a parse_error exception.
 */
TEST_F(VaultSessionTest, ParseInvalidJsonThrows) {
    const std::string body = "{ invalid json }";

    const Bytes bytes{reinterpret_cast<const uint8_t*>(body.data()), body.size()};

    EXPECT_THROW(VaultSession::parse(bytes), ParseError);
}

/*
 * Tests that parsing a JSON representation of a VaultSession missing a required field throws an
 * out_of_range exception.
 */
TEST_F(VaultSessionTest, ParseMissingRequiredFieldThrows) {
    json input = makeVaultJson();
    input.erase("name");

    const std::string body = input.dump();
    const Bytes bytes{reinterpret_cast<const uint8_t*>(body.data()), body.size()};

    EXPECT_THROW(VaultSession::parse(bytes), ParseError);
}
