#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "VaultSession.h"
#include "entities/Entry.h"

#include <chrono>
#include <utility>

namespace {

    EncKey makeEncKey() {
        EncKey key{};
        return key;
    }

    AuthKey makeAuthKey() {
        AuthKey key{};
        return key;
    }

    Persona makePersona(std::string firstName, std::string lastName) {
        return Persona(std::move(firstName), std::move(lastName), std::chrono::system_clock::time_point{},
            "Address", "000000000");
    }

    Category makeCategory(std::string name) {
        return Category(std::move(name));
    }

    class TestEntry : public Entry {
    public:
        explicit TestEntry(std::string notes = {}) : Entry(std::move(notes)) {}
    };

    TestEntry makeEntry(std::string notes) {
        return TestEntry(std::move(notes));
    }

} // namespace


class VaultSessionTest : public ::testing::Test {
};

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

TEST_F(VaultSessionTest, AddCategoryAppendsCategoryAndUpdatesLastModifiedDate) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
    const auto previousLastModified = session.getLastModifiedDate();

    session.addCategory(makeCategory("Passwords"));

    ASSERT_EQ(session.getCategories().size(), 1U);
    EXPECT_EQ(session.getCategories().front().getName(), "Passwords");
    EXPECT_GE(session.getLastModifiedDate(), previousLastModified);
}

TEST_F(VaultSessionTest, AddPersonaAppendsPersonaAndUpdatesLastModifiedDate) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
    const auto previousLastModified = session.getLastModifiedDate();

    session.addPersona(makePersona("Ada", "Lovelace"));

    ASSERT_EQ(session.getPersonas().size(), 1U);
    EXPECT_EQ(session.getPersonas().front().getFirstName(), "Ada");
    EXPECT_EQ(session.getPersonas().front().getLastName(), "Lovelace");
    EXPECT_GE(session.getLastModifiedDate(), previousLastModified);
}

TEST_F(VaultSessionTest, AddEntryToCategoryAppendsEntryToMatchingCategory) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
    session.addCategory(makeCategory("Passwords"));
    const auto categoryId = session.getCategories().front().getId();
    const auto entry = makeEntry("gmail");
    const auto entryId = entry.getId();
    const auto previousLastModified = session.getLastModifiedDate();

    session.addEntryToCategory(categoryId, entry);

    ASSERT_EQ(session.getCategories().size(), 1U);
    ASSERT_EQ(session.getCategories().front().getEntries().size(), 1U);
    EXPECT_EQ(session.getCategories().front().getEntries().front().getNotes(), "gmail");
    EXPECT_EQ(session.getCategories().front().getEntries().front().getId(), entryId);
    EXPECT_GE(session.getLastModifiedDate(), previousLastModified);
}

TEST_F(VaultSessionTest, AddEntryToCategoryOnlyChangesTargetCategory) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
    session.addCategory(makeCategory("Passwords"));
    session.addCategory(makeCategory("Banking"));

    const auto passwordsCategoryId = session.getCategories()[0].getId();
    const auto bankingCategoryId = session.getCategories()[1].getId();

    session.addEntryToCategory(passwordsCategoryId, makeEntry("gmail"));

    ASSERT_EQ(session.getCategories()[0].getEntries().size(), 1U);
    EXPECT_EQ(session.getCategories()[0].getEntries().front().getNotes(), "gmail");
    EXPECT_TRUE(session.getCategories()[1].getEntries().empty());
    EXPECT_NE(passwordsCategoryId, bankingCategoryId);
}

TEST_F(VaultSessionTest, AddEntryToMissingCategoryThrowsAndLeavesStateUnchanged) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
    session.addCategory(makeCategory("Passwords"));
    const auto lastModifiedBefore = session.getLastModifiedDate();
    const auto missingCategoryId = session.getCategories().front().getId() + 1;

    EXPECT_THROW(session.addEntryToCategory(missingCategoryId, makeEntry("gmail")),
        CategoryNotFoundError);

    EXPECT_EQ(session.getCategories().size(), 1U);
    EXPECT_EQ(session.getCategories().front().getName(), "Passwords");
    EXPECT_TRUE(session.getCategories().front().getEntries().empty());
    EXPECT_EQ(session.getLastModifiedDate(), lastModifiedBefore);
}

TEST_F(VaultSessionTest, RemovePersonaRemovesOnlyMatchingPersona) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
    session.addPersona(makePersona("Ada", "Lovelace"));
    session.addPersona(makePersona("Grace", "Hopper"));

    session.removePersona(session.getPersonas().front().getId());

    ASSERT_EQ(session.getPersonas().size(), 1U);
    EXPECT_EQ(session.getPersonas().front().getFirstName(), "Grace");
    EXPECT_EQ(session.getPersonas().front().getLastName(), "Hopper");
}

TEST_F(VaultSessionTest, RemoveMissingPersonaLeavesCollectionUnchanged) {
    VaultSession session("My Vault", makeEncKey(), makeAuthKey());
    session.addPersona(makePersona("Ada", "Lovelace"));
    const auto previousLastModified = session.getLastModifiedDate();

    session.removePersona(session.getPersonas().front().getId() + 1);

    ASSERT_EQ(session.getPersonas().size(), 1U);
    EXPECT_GE(session.getLastModifiedDate(), previousLastModified);
}
