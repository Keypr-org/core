/*
 * @brief Unit tests for the VaultSession class.
 *
 * Some tests were written with the assistance of AI
 *
 * @author Nolan Evard
 * @author Maikol Correia Da Silva
 *
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
        json website = { { "type", "Website" },
                        { "id", 3000 },
                        { "creationAt", 1704067200000LL },
                        { "updatedAt", 1704067201000LL },
                        { "notes", "Website notes" },
                        { "title", "Example" },
                        { "comments", "Example comments" },
                        { "username", "alice" },
                        { "password", "website-password" },
                        { "url", "https://example.com" },
                        { "alias", "Example alias" },
                        { "personaId", 4000 } };

        json wifi = { { "type", "Wifi" },
                     { "id", 3001 },
                     { "creationAt", 1704067200000LL },
                     { "updatedAt", 1704067201000LL },
                     { "notes", "Wi-Fi notes" },
                     { "networkName", "Home Wi-Fi" },
                     { "password", "wifi-password" } };

        json creditCard = { { "type", "CreditCard" },
                           { "id", 3002 },
                           { "creationAt", 1704067200000LL },
                           { "updatedAt", 1704067201000LL },
                           { "notes", "Card notes" },
                           { "cardHolderName", "Alice Example" },
                           { "cardNumber", "4111111111111111" },
                           { "expiration", "12/30" },
                           { "securityCode", "123" } };

        return { { "id", 1000 },
                { "creationAt", 1704067200000LL },
                { "updatedAt", 1704153600000LL },
                { "name", "Parsed Vault" },
                { "categories",
                 { { { "id", 2000 },{ "name", "Passwords" },{ "entries",{ website, wifi, creditCard } } } } },
                { "personas",
                 { { { "id", 4000 },
                   { "creationAt", 1704067200000LL },
                   { "updatedAt", 1704067201000LL },
                   { "firstName", "Alice" },
                   { "lastName", "Example" },
                   { "dateOfBirth", 946684800000LL },
                   { "address", "Example Street 1" },
                   { "phone", "+41 79 123 45 67" } } } } };
    }

    std::unique_ptr<Website> makeWebsite(std::string notes, std::string title, std::string username,
        std::string password, std::string url) {
        return std::make_unique<Website>(std::move(notes), std::move(title), std::move(username),
            std::move(password), std::move(url));
    }

    std::unique_ptr<Website> makeWebsite(std::string url, std::string notes = "notes",
        std::string title = "Title") {
        return std::make_unique<Website>(std::move(notes), std::move(title), "username", "password",
            std::move(url));
    }
};

// ------------- VaultSession tests --------------------

/*
 * Tests that the VaultSession constructor initializes the name and empty collections correctly.
 */
TEST_F(VaultSessionTest, ConstructorInitializesNameAndEmptyCollections) {
    const auto before = std::chrono::system_clock::now();

    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);

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
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);
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
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);
    const auto previousLastModified = session.getLastModifiedDate();

    session.addPersona(makePersona("Ada", "Lovelace"));

    ASSERT_EQ(session.getPersonas().size(), 1U);
    EXPECT_EQ(session.getPersonas().front()->getFirstName(), "Ada");
    EXPECT_EQ(session.getPersonas().front()->getLastName(), "Lovelace");
    EXPECT_GE(session.getLastModifiedDate(), previousLastModified);
}

/*
 * Tests that adding an entry to a category appends it to the matching category and updates the
 * last modified date.
 */
TEST_F(VaultSessionTest, AddEntryToCategoryAppendsEntryToMatchingCategory) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);

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
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);

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
 * Tests that adding an entry to a missing category throws a CategoryNotFoundError and leaves
 * the state unchanged.
 */
TEST_F(VaultSessionTest, AddEntryToMissingCategoryThrowsAndLeavesStateUnchanged) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);

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
 * Tests that linking a persona to a website updates the website persona ID and leaves other
 * entries untouched.
 */
TEST_F(VaultSessionTest, LinkPersonaToEntryLinksMatchingWebsiteOnly) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);

    session.addPersona(makePersona("Ada", "Lovelace"));
    const auto personaId = session.getPersonas().front()->getId();

    session.addCategory(makeCategory("Passwords"));
    const auto categoryId = session.getCategories().front()->getId();

    auto targetWebsite = makeWebsite("target notes", "Target", "alice", "secret",
        "https://target.example.com");
    auto otherWebsite = makeWebsite("other notes", "Other", "bob", "secret",
        "https://other.example.com");
    auto wifi = std::make_unique<Wifi>("wifi notes", "Home Wi-Fi", "wifi-password");

    const auto targetWebsiteId = targetWebsite->getId();
    const auto otherWebsiteId = otherWebsite->getId();
    const auto wifiId = wifi->getId();

    session.addEntryToCategory(categoryId, std::move(targetWebsite));
    session.addEntryToCategory(categoryId, std::move(otherWebsite));
    session.addEntryToCategory(categoryId, std::move(wifi));

    session.linkPersonaToEntry(personaId, categoryId, targetWebsiteId);

    const auto &entries = session.getCategories().front()->getEntries();
    const auto *linkedWebsite = dynamic_cast<const Website *>(entries[0].get());
    const auto *unlinkedWebsite = dynamic_cast<const Website *>(entries[1].get());
    const auto *storedWifi = dynamic_cast<const Wifi *>(entries[2].get());

    ASSERT_NE(linkedWebsite, nullptr);
    ASSERT_NE(unlinkedWebsite, nullptr);
    ASSERT_NE(storedWifi, nullptr);

    EXPECT_EQ(linkedWebsite->getId(), targetWebsiteId);
    EXPECT_EQ(linkedWebsite->getPersonaId(), personaId);
    EXPECT_EQ(unlinkedWebsite->getId(), otherWebsiteId);
    EXPECT_EQ(unlinkedWebsite->getPersonaId(), -1);
    EXPECT_EQ(storedWifi->getId(), wifiId);
}

/*
 * Tests that linking a persona to a website throws when the persona does not exist.
 */
TEST_F(VaultSessionTest, LinkPersonaToEntryThrowsWhenPersonaDoesNotExist) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);

    session.addCategory(makeCategory("Passwords"));
    const auto categoryId = session.getCategories().front()->getId();

    auto website = makeWebsite("notes", "Target", "alice", "secret", "https://example.com");
    const auto websiteId = website->getId();
    session.addEntryToCategory(categoryId, std::move(website));

    EXPECT_THROW(session.linkPersonaToEntry(999999, categoryId, websiteId), PersonaNotFoundError);

    const auto *storedWebsite = session.getWebsiteById(websiteId);
    ASSERT_NE(storedWebsite, nullptr);
    EXPECT_EQ(storedWebsite->getPersonaId(), -1);
}

/*
 * Tests that linking a persona to a website throws when the category does not exist.
 */
TEST_F(VaultSessionTest, LinkPersonaToEntryThrowsWhenCategoryDoesNotExist) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);

    session.addPersona(makePersona("Ada", "Lovelace"));
    const auto personaId = session.getPersonas().front()->getId();
    auto category = makeCategory("Passwords");
    int64_t categoryId = category->getId();
    session.addCategory(std::move(category));
    auto website = makeWebsite("notes", "Target", "alice", "secret", "https://example.com");
    const auto websiteId = website->getId();
    session.addEntryToCategory(session.getCategories().front()->getId(), std::move(website));

    EXPECT_THROW(session.linkPersonaToEntry(personaId, categoryId + 1, websiteId), CategoryNotFoundError);

    const auto *storedWebsite = session.getWebsiteById(websiteId);
    ASSERT_NE(storedWebsite, nullptr);
    EXPECT_EQ(storedWebsite->getPersonaId(), -1);
}

/*
 * Tests that linking a persona to a website throws when the entry does not exist in the category.
 */
TEST_F(VaultSessionTest, LinkPersonaToEntryThrowsWhenEntryDoesNotExist) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);

    session.addPersona(makePersona("Ada", "Lovelace"));
    const auto personaId = session.getPersonas().front()->getId();

    session.addCategory(makeCategory("Passwords"));
    const auto categoryId = session.getCategories().front()->getId();

    auto website = makeWebsite("notes", "Target", "alice", "secret", "https://example.com");
    const auto websiteId = website->getId();
    session.addEntryToCategory(categoryId, std::move(website));

    EXPECT_THROW(session.linkPersonaToEntry(personaId, categoryId, websiteId + 1),
        EntryNotFoundError);

    const auto *storedWebsite = session.getWebsiteById(websiteId);
    ASSERT_NE(storedWebsite, nullptr);
    EXPECT_EQ(storedWebsite->getPersonaId(), -1);
}

/*
 * Tests that linking a persona to a non-website entry throws and leaves the category unchanged.
 */
TEST_F(VaultSessionTest, LinkPersonaToEntryThrowsWhenEntryHasWrongType) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);

    session.addPersona(makePersona("Ada", "Lovelace"));
    const auto personaId = session.getPersonas().front()->getId();

    session.addCategory(makeCategory("Passwords"));
    const auto categoryId = session.getCategories().front()->getId();

    auto website = makeWebsite("notes", "Target", "alice", "secret", "https://example.com");
    auto wifi = std::make_unique<Wifi>("wifi notes", "Home Wi-Fi", "wifi-password");
    const auto websiteId = website->getId();
    const auto wifiId = wifi->getId();
    session.addEntryToCategory(categoryId, std::move(website));
    session.addEntryToCategory(categoryId, std::move(wifi));

    EXPECT_THROW(session.linkPersonaToEntry(personaId, categoryId, wifiId),
        EntryNotGoodTypeError);

    const auto *storedWebsite = session.getWebsiteById(websiteId);
    ASSERT_NE(storedWebsite, nullptr);
    EXPECT_EQ(storedWebsite->getPersonaId(), -1);
}

/*
 * Tests that linking a persona to a credit card entry also throws because only websites are
 * linkable.
 */
TEST_F(VaultSessionTest, LinkPersonaToEntryThrowsWhenEntryIsCreditCard) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);

    session.addPersona(makePersona("Ada", "Lovelace"));
    const auto personaId = session.getPersonas().front()->getId();

    session.addCategory(makeCategory("Cards"));
    const auto categoryId = session.getCategories().front()->getId();

    auto website = makeWebsite("notes", "Target", "alice", "secret", "https://example.com");
    auto card = std::make_unique<CreditCard>("Alice Example", "4111111111111111", "12/30",
        "123", "card notes");
    const auto websiteId = website->getId();
    const auto cardId = card->getId();
    session.addEntryToCategory(categoryId, std::move(website));
    session.addEntryToCategory(categoryId, std::move(card));

    EXPECT_THROW(session.linkPersonaToEntry(personaId, categoryId, cardId),
        EntryNotGoodTypeError);

    const auto *storedWebsite = session.getWebsiteById(websiteId);
    ASSERT_NE(storedWebsite, nullptr);
    EXPECT_EQ(storedWebsite->getPersonaId(), -1);
}

/**
 * Test that getWebsiteById returns the matching website when the ID exists.
 */
TEST_F(VaultSessionTest, GetWebsiteByIdReturnsMatchingWebsite) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);
    session.addCategory(std::move(makeCategory("Passwords")));
    const auto categoryId = session.getCategories().front()->getId();

    auto website = makeWebsite("notes", "GitHub", "alice", "secret", "https://github.com");
    const auto expectedWebsiteId = website->getId();
    const auto expectedWebsite = website.get();
    session.addEntryToCategory(categoryId, std::move(website));

    const auto *foundWebsite = session.getWebsiteById(expectedWebsiteId);

    ASSERT_NE(foundWebsite, nullptr);
    EXPECT_EQ(foundWebsite, expectedWebsite);
    EXPECT_EQ(foundWebsite->getId(), expectedWebsiteId);
    EXPECT_EQ(foundWebsite->getTitle(), "GitHub");
    EXPECT_EQ(foundWebsite->getUrl(), "https://github.com");
}

/**
 * Test that getWebsiteById searches across all categories.
 */
TEST_F(VaultSessionTest, GetWebsiteByIdSearchesAcrossCategories) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);
    session.addCategory(std::move(makeCategory("Personal")));
    session.addCategory(std::move(makeCategory("Work")));

    const auto personalCategoryId = session.getCategories()[0]->getId();
    const auto workCategoryId = session.getCategories()[1]->getId();

    auto personalWebsite =
        makeWebsite("notes", "Personal", "alice", "secret", "https://personal.example.com");
    auto workWebsite = makeWebsite("notes", "Work", "bob", "secret", "https://work.example.com");
    const auto personalWebsiteId = personalWebsite->getId();
    const auto workWebsiteId = workWebsite->getId();

    session.addEntryToCategory(personalCategoryId, std::move(personalWebsite));
    session.addEntryToCategory(workCategoryId, std::move(workWebsite));

    const auto *foundWebsite = session.getWebsiteById(workWebsiteId);

    ASSERT_NE(foundWebsite, nullptr);
    EXPECT_EQ(foundWebsite->getId(), workWebsiteId);
    EXPECT_EQ(foundWebsite->getUrl(), "https://work.example.com");
    EXPECT_NE(foundWebsite->getId(), personalWebsiteId);
}

/**
 * Test that getWebsiteById returns nullptr when no website matches the given ID.
 */
TEST_F(VaultSessionTest, GetWebsiteByIdReturnsNullptrWhenWebsiteDoesNotExist) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);
    session.addCategory(std::move(makeCategory("Passwords")));

    auto website = makeWebsite("notes", "GitHub", "alice", "secret", "https://github.com");
    auto websiteId = website->getId();
    session.addEntryToCategory(session.getCategories().front()->getId(), std::move(website));

    const auto *foundWebsite = session.getWebsiteById(websiteId + 1);

    EXPECT_EQ(foundWebsite, nullptr);
}

/**
 * Test that getWebsiteByUrl returns all websites matching the given URL.
 */
TEST_F(VaultSessionTest, GetWebsiteByUrlReturnsAllMatchingWebsites) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);
    session.addCategory(std::move(makeCategory("Passwords")));
    const auto categoryId = session.getCategories().front()->getId();

    session.addEntryToCategory(categoryId,
        std::move(makeWebsite("https://example.com", "first", "First")));
    session.addEntryToCategory(categoryId,
        std::move(makeWebsite("https://example.com", "second", "Second")));

    const auto websites = session.getWebsiteByUrl("https://example.com");

    ASSERT_EQ(websites.size(), 2U);
    ASSERT_NE(websites[0], nullptr);
    ASSERT_NE(websites[1], nullptr);
    EXPECT_EQ(websites[0]->getUrl(), "https://example.com");
    EXPECT_EQ(websites[1]->getUrl(), "https://example.com");
    EXPECT_EQ(websites[0]->getNotes(), "first");
    EXPECT_EQ(websites[1]->getNotes(), "second");
}

/**
 * Test that getWebsiteByUrl returns an empty vector when no website matches the requested URL.
 */
TEST_F(VaultSessionTest, GetWebsiteByUrlReturnsEmptyVectorWhenNoWebsiteMatches) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);
    session.addCategory(std::move(makeCategory("Passwords")));
    session.addEntryToCategory(session.getCategories().front()->getId(),
        std::move(makeWebsite("https://example.com")));

    const auto websites = session.getWebsiteByUrl("https://missing.example.com");

    EXPECT_TRUE(websites.empty());
}

/**
 * Test that getWebsiteByUrl matches when the requested URL contains the website URL.
 */
TEST_F(VaultSessionTest, GetWebsiteByUrlMatchesWhenRequestedUrlContainsWebsiteUrl) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);
    session.addCategory(std::move(makeCategory("Passwords")));
    const auto categoryId = session.getCategories().front()->getId();

    session.addEntryToCategory(categoryId, std::move(makeWebsite("example.com", "notes")));

    const auto websites = session.getWebsiteByUrl("https://example.com/login");

    ASSERT_EQ(websites.size(), 1U);
    ASSERT_NE(websites[0], nullptr);
    EXPECT_EQ(websites[0]->getUrl(), "example.com");
    EXPECT_EQ(websites[0]->getNotes(), "notes");
}

/**
 * Test that getWebsiteByUrl matches when the website URL contains the requested URL.
 */
TEST_F(VaultSessionTest, GetWebsiteByUrlMatchesWhenWebsiteUrlContainsRequestedUrl) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);
    session.addCategory(std::move(makeCategory("Passwords")));
    const auto categoryId = session.getCategories().front()->getId();

    session.addEntryToCategory(categoryId,
        std::move(makeWebsite("https://example.com/login", "notes")));

    const auto websites = session.getWebsiteByUrl("example.com");

    ASSERT_EQ(websites.size(), 1U);
    ASSERT_NE(websites[0], nullptr);
    EXPECT_EQ(websites[0]->getUrl(), "https://example.com/login");
    EXPECT_EQ(websites[0]->getNotes(), "notes");
}

/**
 * Test that removing an entry from a category removes the matching entry and updates the last
 * modified date.
 */
TEST_F(VaultSessionTest, RemoveEntryFromCategoryRemovesMatchingEntry) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);
    session.addCategory(makeCategory("Passwords"));
    const auto categoryId = session.getCategories().front()->getId();

    auto gmailEntry = makeEntry("gmail");
    const auto gmailEntryId = gmailEntry->getId();
    auto bankEntry = makeEntry("bank");
    session.addEntryToCategory(categoryId, std::move(gmailEntry));
    session.addEntryToCategory(categoryId, std::move(bankEntry));
    const auto lastModifiedBefore = session.getLastModifiedDate();

    session.removeEntryFromCategory(categoryId, gmailEntryId);

    ASSERT_EQ(session.getCategories().front()->getEntries().size(), 1U);
    EXPECT_EQ(session.getCategories().front()->getEntries().front()->getNotes(), "bank");
    EXPECT_GE(session.getLastModifiedDate(), lastModifiedBefore);
}

/**
 * Test that removing an entry from a category only changes the targeted category.
 */
TEST_F(VaultSessionTest, RemoveEntryFromCategoryOnlyChangesTargetCategory) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);
    session.addCategory(makeCategory("Passwords"));
    session.addCategory(makeCategory("Banking"));

    const auto passwordsCategoryId = session.getCategories()[0]->getId();
    const auto bankingCategoryId = session.getCategories()[1]->getId();

    auto passwordsEntry = makeEntry("gmail");
    const auto passwordsEntryId = passwordsEntry->getId();
    auto bankingEntry = makeEntry("visa");
    session.addEntryToCategory(passwordsCategoryId, std::move(passwordsEntry));
    session.addEntryToCategory(bankingCategoryId, std::move(bankingEntry));

    session.removeEntryFromCategory(passwordsCategoryId, passwordsEntryId);

    EXPECT_TRUE(session.getCategories()[0]->getEntries().empty());
    ASSERT_EQ(session.getCategories()[1]->getEntries().size(), 1U);
    EXPECT_EQ(session.getCategories()[1]->getEntries().front()->getNotes(), "visa");
    EXPECT_NE(passwordsCategoryId, bankingCategoryId);
}

/**
 * Test that removing a missing entry from an existing category throws a EntryNotFoundError and
 * leaves the category unchanged.
 */
TEST_F(VaultSessionTest, RemoveMissingEntryFromExistingCategoryLeavesCategoryUnchanged) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);
    session.addCategory(makeCategory("Passwords"));
    const auto categoryId = session.getCategories().front()->getId();

    auto entry = makeEntry("gmail");
    auto entryId = entry->getId();
    session.addEntryToCategory(categoryId, std::move(entry));
    const auto lastModifiedBefore = session.getLastModifiedDate();

    EXPECT_THROW(session.removeEntryFromCategory(categoryId, entryId + 1), EntryNotFoundError);

    ASSERT_EQ(session.getCategories().front()->getEntries().size(), 1U);
    EXPECT_EQ(session.getCategories().front()->getEntries().front()->getNotes(), "gmail");
    EXPECT_GE(session.getLastModifiedDate(), lastModifiedBefore);
}

/**
 * Test that removing an entry from a missing category throws a CategoryNotFoundError and leaves
 * the state unchanged.
 */
TEST_F(VaultSessionTest, RemoveEntryFromMissingCategoryThrowsAndLeavesStateUnchanged) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);
    session.addCategory(makeCategory("Passwords"));
    const auto lastModifiedBefore = session.getLastModifiedDate();
    const auto missingCategoryId = session.getCategories().front()->getId() + 1;

    EXPECT_THROW(session.removeEntryFromCategory(missingCategoryId, 123), CategoryNotFoundError);

    EXPECT_EQ(session.getCategories().size(), 1U);
    EXPECT_EQ(session.getCategories().front()->getName(), "Passwords");
    EXPECT_TRUE(session.getCategories().front()->getEntries().empty());
    EXPECT_EQ(session.getLastModifiedDate(), lastModifiedBefore);
}

/**
 * Test that removing a persona from a VaultSession removes only the matching persona and
 * updates the last modified date.
 */
TEST_F(VaultSessionTest, RemovePersonaRemovesOnlyMatchingPersona) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);

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
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);

    session.addPersona(makePersona("Ada", "Lovelace"));

    const auto previousLastModified = session.getLastModifiedDate();

    session.removePersona(session.getPersonas().front()->getId() + 1);

    ASSERT_EQ(session.getPersonas().size(), 1U);
    EXPECT_GE(session.getLastModifiedDate(), previousLastModified);
}

/**
 * Test that searchEntriesInCategory returns the entries whose notes contain the search
 * term.
 */
TEST_F(VaultSessionTest, SearchEntriesInCategoryReturnsMatchingEntries) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);
    session.addCategory(std::move(makeCategory("Passwords")));
    const auto categoryId = session.getCategories().front()->getId();

    auto githubEntry = makeEntry("GitHub personal account");
    const auto githubEntryId = githubEntry->getId();
    auto gmailEntry = makeEntry("Gmail work account");
    session.addEntryToCategory(categoryId, std::move(githubEntry));
    session.addEntryToCategory(categoryId, std::move(gmailEntry));

    const auto matches = session.searchEntriesInCategory(categoryId, "GitHub");

    ASSERT_EQ(matches.size(), 1U);
    ASSERT_NE(matches[0], nullptr);
    EXPECT_EQ(matches[0]->getId(), githubEntryId);
    EXPECT_EQ(matches[0]->getNotes(), "GitHub personal account");
}

/**
 * Test that searchEntriesInCategory returns all matching entries when several notes contain
 * the term.
 */
TEST_F(VaultSessionTest, SearchEntriesInCategoryReturnsAllMatchingEntries) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);
    session.addCategory(std::move(makeCategory("Passwords")));
    const auto categoryId = session.getCategories().front()->getId();

    auto personalGitHubEntry = makeEntry("GitHub personal account");
    auto workGitHubEntry = makeEntry("GitHub work account");
    auto unrelatedEntry = makeEntry("Bitwarden backup");
    const auto firstMatchId = personalGitHubEntry->getId();
    const auto secondMatchId = workGitHubEntry->getId();
    session.addEntryToCategory(categoryId, std::move(personalGitHubEntry));
    session.addEntryToCategory(categoryId, std::move(workGitHubEntry));
    session.addEntryToCategory(categoryId, std::move(unrelatedEntry));

    const auto matches = session.searchEntriesInCategory(categoryId, "GitHub");

    ASSERT_EQ(matches.size(), 2U);
    ASSERT_NE(matches[0], nullptr);
    ASSERT_NE(matches[1], nullptr);

    std::vector<int64_t> matchedIds;
    matchedIds.push_back(matches[0]->getId());
    matchedIds.push_back(matches[1]->getId());

    EXPECT_THAT(matchedIds, ::testing::UnorderedElementsAre(firstMatchId, secondMatchId));
}

/**
 * Test that searchEntriesInCategory only returns entries from the requested category, even
 * if other categories also match.
 */
TEST_F(VaultSessionTest, SearchEntriesInCategoryOnlyReturnsEntriesFromRequestedCategory) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);
    session.addCategory(std::move(makeCategory("Personal")));
    session.addCategory(std::move(makeCategory("Work")));

    const auto personalCategoryId = session.getCategories()[0]->getId();
    const auto workCategoryId = session.getCategories()[1]->getId();

    auto personalGitHubEntry = makeEntry("GitHub personal account");
    auto personalMailEntry = makeEntry("GitHub mail backup");
    auto workGitHubEntry = makeEntry("GitHub work account");
    const auto firstPersonalId = personalGitHubEntry->getId();
    const auto secondPersonalId = personalMailEntry->getId();
    const auto workId = workGitHubEntry->getId();

    session.addEntryToCategory(personalCategoryId, std::move(personalGitHubEntry));
    session.addEntryToCategory(personalCategoryId, std::move(personalMailEntry));
    session.addEntryToCategory(workCategoryId, std::move(workGitHubEntry));

    const auto matches = session.searchEntriesInCategory(personalCategoryId, "GitHub");

    ASSERT_EQ(matches.size(), 2U);
    std::vector<int64_t> matchedIds;
    for (const auto *entry : matches) {
        ASSERT_NE(entry, nullptr);
        matchedIds.push_back(entry->getId());
    }

    EXPECT_THAT(matchedIds, ::testing::UnorderedElementsAre(firstPersonalId, secondPersonalId));
    EXPECT_NE(matchedIds[0], workId);
    EXPECT_NE(matchedIds[1], workId);
}

/**
 * Test that searchEntriesInCategory can match a Website based on its URL.
 */
TEST_F(VaultSessionTest, SearchEntriesInCategoryMatchesWebsiteUrl) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);
    session.addCategory(std::move(makeCategory("Passwords")));
    const auto categoryId = session.getCategories().front()->getId();

    auto website =
        makeWebsite("Example note", "GitHub", "alice", "secret", "https://github.com/alice");
    const auto websiteId = website->getId();
    session.addEntryToCategory(categoryId, std::move(website));

    const auto matches = session.searchEntriesInCategory(categoryId, "github.com");

    ASSERT_EQ(matches.size(), 1U);
    ASSERT_NE(matches[0], nullptr);
    EXPECT_EQ(matches[0]->getId(), websiteId);
    EXPECT_EQ(matches[0]->getNotes(), "Example note");
}

/**
 * Test that searchEntriesInCategory returns an empty vector when no entry matches the
 * search term.
 */
TEST_F(VaultSessionTest, SearchEntriesInCategoryReturnsEmptyVectorWhenNoEntryMatches) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);
    session.addCategory(std::move(makeCategory("Passwords")));
    const auto categoryId = session.getCategories().front()->getId();

    session.addEntryToCategory(categoryId, std::move(makeEntry("GitHub personal account")));
    session.addEntryToCategory(categoryId, std::move(makeEntry("Gmail work account")));

    const auto matches = session.searchEntriesInCategory(categoryId, "Bitwarden");

    EXPECT_TRUE(matches.empty());
}

/**
 * Test that searchEntriesInCategory throws a CategoryNotFoundError when the category does
 * not exist.
 */
TEST_F(VaultSessionTest, SearchEntriesInCategoryThrowsWhenCategoryDoesNotExist) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey(), nullptr);
    session.addCategory(std::move(makeCategory("Passwords")));

    EXPECT_THROW(
        session.searchEntriesInCategory(session.getCategories().front()->getId() + 1, "GitHub"),
        CategoryNotFoundError);
}

// ------------- json parsing tests --------------------

/*
 * Tests that parsing a valid JSON representation of a VaultSession correctly populates the
 * fields.
 */
TEST_F(VaultSessionTest, ParseReadsVaultMetadata) {
    const json input = makeVaultJson();

    const std::string body = input.dump();
    const Bytes bytes{ reinterpret_cast<const uint8_t *>(body.data()), body.size() };

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
    const Bytes bytes{ reinterpret_cast<const uint8_t *>(body.data()), body.size() };

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
 * Tests that parsing a valid JSON representation of a VaultSession correctly populates the
 * entries of the categories, including all supported entry types (Website, Wifi, CreditCard).
 */
TEST_F(VaultSessionTest, ParseReadsAllSupportedEntryTypes) {
    const json input = makeVaultJson();

    const std::string body = input.dump();
    const Bytes bytes{ reinterpret_cast<const uint8_t *>(body.data()), body.size() };

    VaultSession session = VaultSession::parse(bytes);

    const auto &entries = session.getCategories()[0]->getEntries();

    ASSERT_EQ(entries.size(), 3U);

    const auto *website = dynamic_cast<const Website *>(entries[0].get());
    ASSERT_NE(website, nullptr);
    EXPECT_EQ(website->getId(), 3000);
    EXPECT_EQ(website->getTitle(), "Example");
    EXPECT_EQ(website->getUsername(), "alice");
    EXPECT_EQ(website->getUrl(), "https://example.com");
    EXPECT_EQ(website->getPersonaId(), 4000);

    const auto *wifi = dynamic_cast<const Wifi *>(entries[1].get());
    ASSERT_NE(wifi, nullptr);
    EXPECT_EQ(wifi->getId(), 3001);
    EXPECT_EQ(wifi->getNetworkName(), "Home Wi-Fi");
    EXPECT_EQ(wifi->getPassword(), "wifi-password");

    const auto *card = dynamic_cast<const CreditCard *>(entries[2].get());
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

    const Bytes bytes{ reinterpret_cast<const uint8_t *>(body.data()), body.size() };

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
    const Bytes bytes{ reinterpret_cast<const uint8_t *>(body.data()), body.size() };

    EXPECT_THROW(VaultSession::parse(bytes), ParseError);
}
