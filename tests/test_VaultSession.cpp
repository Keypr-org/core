#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "VaultSession.h"
#include "entities/Entry.h"
#include "entities/Website.h"

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
 * Test that removing an entry from a category removes the matching entry and updates the last modified date.
 */
TEST_F(VaultSessionTest, RemoveEntryFromCategoryRemovesMatchingEntry) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
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
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
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
 * Test that removing a missing entry from an existing category throws a EntryNotFoundError and leaves the category unchanged.
 */
TEST_F(VaultSessionTest, RemoveMissingEntryFromExistingCategoryLeavesCategoryUnchanged) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
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
 * Test that removing an entry from a missing category throws a CategoryNotFoundError and leaves the state unchanged.
 */
TEST_F(VaultSessionTest, RemoveEntryFromMissingCategoryThrowsAndLeavesStateUnchanged) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
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

/**
 * Test that searchEntriesInCategory returns the entries whose notes contain the search term.
 */
TEST_F(VaultSessionTest, SearchEntriesInCategoryReturnsMatchingEntries) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
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
 * Test that searchEntriesInCategory returns all matching entries when several notes contain the term.
 */
TEST_F(VaultSessionTest, SearchEntriesInCategoryReturnsAllMatchingEntries) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
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
 * Test that searchEntriesInCategory only returns entries from the requested category, even if other categories also match.
 */
TEST_F(VaultSessionTest, SearchEntriesInCategoryOnlyReturnsEntriesFromRequestedCategory) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
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
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
    session.addCategory(std::move(makeCategory("Passwords")));
    const auto categoryId = session.getCategories().front()->getId();

    auto website = makeWebsite("Example note", "GitHub", "alice", "secret", "https://github.com/alice");
    const auto websiteId = website->getId();
    session.addEntryToCategory(categoryId, std::move(website));

    const auto matches = session.searchEntriesInCategory(categoryId, "github.com");

    ASSERT_EQ(matches.size(), 1U);
    ASSERT_NE(matches[0], nullptr);
    EXPECT_EQ(matches[0]->getId(), websiteId);
    EXPECT_EQ(matches[0]->getNotes(), "Example note");
}

/**
 * Test that searchEntriesInCategory returns an empty vector when no entry matches the search term.
 */
TEST_F(VaultSessionTest, SearchEntriesInCategoryReturnsEmptyVectorWhenNoEntryMatches) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
    session.addCategory(std::move(makeCategory("Passwords")));
    const auto categoryId = session.getCategories().front()->getId();

    session.addEntryToCategory(categoryId, std::move(makeEntry("GitHub personal account")));
    session.addEntryToCategory(categoryId, std::move(makeEntry("Gmail work account")));

    const auto matches = session.searchEntriesInCategory(categoryId, "Bitwarden");

    EXPECT_TRUE(matches.empty());
}

/**
 * Test that searchEntriesInCategory throws a CategoryNotFoundError when the category does not exist.
 */
TEST_F(VaultSessionTest, SearchEntriesInCategoryThrowsWhenCategoryDoesNotExist) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
    session.addCategory(std::move(makeCategory("Passwords")));

    EXPECT_THROW(session.searchEntriesInCategory(session.getCategories().front()->getId() + 1, "GitHub"),
        CategoryNotFoundError);
}
