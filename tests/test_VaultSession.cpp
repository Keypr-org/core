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

    std::unique_ptr<Website> makeWebsite(std::string notes, std::string title, std::string username,
        std::string password, std::string url) {
        return std::make_unique<Website>(std::move(notes), std::move(title), std::move(username),
            std::move(password), std::move(url));
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
 * Test that getWebsiteById returns the matching website when the ID exists.
 */
TEST_F(VaultSessionTest, GetWebsiteByIdReturnsMatchingWebsite) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
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
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
    session.addCategory(std::move(makeCategory("Personal")));
    session.addCategory(std::move(makeCategory("Work")));

    const auto personalCategoryId = session.getCategories()[0]->getId();
    const auto workCategoryId = session.getCategories()[1]->getId();

    auto personalWebsite = makeWebsite("notes", "Personal", "alice", "secret", "https://personal.example.com");
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
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
    session.addCategory(std::move(makeCategory("Passwords")));

    auto website = makeWebsite("notes", "GitHub", "alice", "secret", "https://github.com");
    auto websiteId = website->getId();
    session.addEntryToCategory(session.getCategories().front()->getId(), std::move(website));

    const auto *foundWebsite = session.getWebsiteById(websiteId + 1);

    EXPECT_EQ(foundWebsite, nullptr);
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
