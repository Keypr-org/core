#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "VaultSession.h"
#include "entities/Entry.h"

#include <chrono>
#include <utility>
#include <memory>


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

    std::shared_ptr<Persona> makePersona(std::string firstName, std::string lastName) {
        return std::make_shared<Persona>(std::move(firstName), std::move(lastName), std::chrono::system_clock::time_point{},
            "Address", "000000000");
    }

    std::unique_ptr<Category> makeCategory(std::string name) {
        return std::make_unique<Category>(std::move(name));
    }

    class TestEntry : public Entry {
    public:
        explicit TestEntry(std::string notes = {}) : Entry(std::move(notes)) {}
    };


    std::unique_ptr<Entry> makeEntry(std::string notes) {
        return std::make_unique<TestEntry>(std::move(notes));
    }

    std::unique_ptr<Website> makeWebsite(std::string url, std::string notes = "notes", std::string title = "Title") {
        return std::make_unique<Website>(std::move(notes), std::move(title), "username", "password",
            std::move(url));
    }
};

// ------------- TESTS --------------------

/**
 * Test that the VaultSession constructor initializes the name and empty collections.
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

/**
 * Test that adding a category to a VaultSession appends it to the categories collection and updates the last modified date.
 */
TEST_F(VaultSessionTest, AddCategoryAppendsCategoryAndUpdatesLastModifiedDate) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
    const auto previousLastModified = session.getLastModifiedDate();

    session.addCategory(std::move(makeCategory("Passwords")));

    ASSERT_EQ(session.getCategories().size(), 1U);
    EXPECT_EQ(session.getCategories().front()->getName(), "Passwords");
    EXPECT_GE(session.getLastModifiedDate(), previousLastModified);
}

/**
 * Test that adding a persona to a VaultSession appends it to the personas collection and updates the last modified date.
 */
TEST_F(VaultSessionTest, AddPersonaAppendsPersonaAndUpdatesLastModifiedDate) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
    const auto previousLastModified = session.getLastModifiedDate();

    session.addPersona(std::move(makePersona("Ada", "Lovelace")));

    ASSERT_EQ(session.getPersonas().size(), 1U);
    EXPECT_EQ(session.getPersonas().front()->getFirstName(), "Ada");
    EXPECT_EQ(session.getPersonas().front()->getLastName(), "Lovelace");
    EXPECT_GE(session.getLastModifiedDate(), previousLastModified);
}

/**
 * Test that adding an entry to a category appends it to the matching category's entries and updates the last modified date.
 */
TEST_F(VaultSessionTest, AddEntryToCategoryAppendsEntryToMatchingCategory) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
    session.addCategory(std::move(makeCategory("Passwords")));
    const auto categoryId = session.getCategories().front()->getId();
    auto entry = makeEntry("gmail");
    const auto entryId = entry->getId();
    const auto previousLastModified = session.getLastModifiedDate();

    session.addEntryToCategory(categoryId, std::move(entry));

    ASSERT_EQ(session.getCategories().size(), 1U);
    ASSERT_EQ(session.getCategories().front()->getEntries().size(), 1U);
    EXPECT_EQ(session.getCategories().front()->getEntries().front()->getNotes(), "gmail");
    EXPECT_EQ(session.getCategories().front()->getEntries().front()->getId(), entryId);
    EXPECT_GE(session.getLastModifiedDate(), previousLastModified);
}

/**
 * Test that adding an entry to a category only changes the target category and not others.
 */
TEST_F(VaultSessionTest, AddEntryToCategoryOnlyChangesTargetCategory) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
    session.addCategory(std::move(makeCategory("Passwords")));
    session.addCategory(std::move(makeCategory("Banking")));

    const auto passwordsCategoryId = session.getCategories()[0]->getId();
    const auto bankingCategoryId = session.getCategories()[1]->getId();

    session.addEntryToCategory(passwordsCategoryId, std::move(makeEntry("gmail")));

    ASSERT_EQ(session.getCategories()[0]->getEntries().size(), 1U);
    EXPECT_EQ(session.getCategories()[0]->getEntries().front()->getNotes(), "gmail");
    EXPECT_TRUE(session.getCategories()[1]->getEntries().empty());
    EXPECT_NE(passwordsCategoryId, bankingCategoryId);
}

/**
 * Test that adding an entry to a missing category throws a CategoryNotFoundError and leaves the state unchanged.
 */
TEST_F(VaultSessionTest, AddEntryToMissingCategoryThrowsAndLeavesStateUnchanged) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
    session.addCategory(std::move(makeCategory("Passwords")));
    const auto lastModifiedBefore = session.getLastModifiedDate();
    const auto missingCategoryId = session.getCategories().front()->getId() + 1;

    EXPECT_THROW(session.addEntryToCategory(missingCategoryId, std::move(makeEntry("gmail"))),
        CategoryNotFoundError);

    EXPECT_EQ(session.getCategories().size(), 1U);
    EXPECT_EQ(session.getCategories().front()->getName(), "Passwords");
    EXPECT_TRUE(session.getCategories().front()->getEntries().empty());
    EXPECT_EQ(session.getLastModifiedDate(), lastModifiedBefore);
}

/**
 * Test that getWebsiteByUrl returns all websites matching the given URL.
 */
TEST_F(VaultSessionTest, GetWebsiteByUrlReturnsAllMatchingWebsites) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
    session.addCategory(std::move(makeCategory("Passwords")));
    const auto categoryId = session.getCategories().front()->getId();

    session.addEntryToCategory(categoryId, std::move(makeWebsite("https://example.com", "first", "First")));
    session.addEntryToCategory(categoryId, std::move(makeWebsite("https://example.com", "second", "Second")));

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
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
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
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
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
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
    session.addCategory(std::move(makeCategory("Passwords")));
    const auto categoryId = session.getCategories().front()->getId();

    session.addEntryToCategory(categoryId, std::move(makeWebsite("https://example.com/login", "notes")));

    const auto websites = session.getWebsiteByUrl("example.com");

    ASSERT_EQ(websites.size(), 1U);
    ASSERT_NE(websites[0], nullptr);
    EXPECT_EQ(websites[0]->getUrl(), "https://example.com/login");
    EXPECT_EQ(websites[0]->getNotes(), "notes");
}

/**
 * Test that removing a persona from a VaultSession removes only the matching persona and updates the last modified date.
 */
TEST_F(VaultSessionTest, RemovePersonaRemovesOnlyMatchingPersona) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
    session.addPersona(std::move(makePersona("Ada", "Lovelace")));
    session.addPersona(std::move(makePersona("Grace", "Hopper")));

    session.removePersona(session.getPersonas().front()->getId());

    ASSERT_EQ(session.getPersonas().size(), 1U);
    EXPECT_EQ(session.getPersonas().front()->getFirstName(), "Grace");
    EXPECT_EQ(session.getPersonas().front()->getLastName(), "Hopper");
}

/**
 * Test that removing a missing persona from a VaultSession leaves the collection unchanged and updates the last modified date.
 */
TEST_F(VaultSessionTest, RemoveMissingPersonaLeavesCollectionUnchanged) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
    session.addPersona(std::move(makePersona("Ada", "Lovelace")));
    const auto previousLastModified = session.getLastModifiedDate();

    session.removePersona(session.getPersonas().front()->getId() + 1);

    ASSERT_EQ(session.getPersonas().size(), 1U);
    EXPECT_GE(session.getLastModifiedDate(), previousLastModified);
}
