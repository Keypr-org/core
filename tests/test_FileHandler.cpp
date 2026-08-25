/*
 * This test file uses GoogleTest to verify that FileHandler.cpp works correctly.
 *
 * It was written using AI assistance as it was the starting point of our testsing journey.
 * It is rich in comments so it can be used as a reference for future tests
 */

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

#include "FileHandler.h"
#include "Types.h"

namespace fs = std::filesystem;

/*
 * Test fixture shared by all FileHandler tests.
 *
 * Each test gets its own temporary directory. This prevents tests from
 * interfering with one another and keeps generated files out of the project.
 */
class FileHandlerTest : public ::testing::Test {
  protected:
    fs::path testDirectory;

    /*
     * SetUp() runs before every test.
     *
     * A unique temporary directory is created for the test.
     */
    void SetUp() override {
        const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();

        testDirectory =
            fs::temp_directory_path() / ("file_handler_test_" + std::to_string(timestamp));

        fs::create_directories(testDirectory);
    }

    /*
     * TearDown() runs after every test.
     *
     * The temporary directory and all files inside it are removed.
     */
    void TearDown() override {
        std::error_code error;
        fs::remove_all(testDirectory, error);
    }

    /*
     * Creates a complete path inside the test directory.
     */
    fs::path path(const std::string& filename) const { return testDirectory / filename; }

    /*
     * Reads all remaining bytes from an opened file.
     *
     * A std::string is suitable here because it can contain null bytes and
     * other non-printable characters.
     */
    static std::string readFile(std::ifstream& file) {
        return {std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
    }

    /*
     * Creates a read-only byte span over a string's underlying data.
     *
     * This does not copy the data. The string must remain alive and must not
     * be modified in a way that causes reallocation while the span is used.
     */
    static Bytes asBytes(const std::string& data) {
        return Bytes{reinterpret_cast<const std::byte*>(data.data()), data.size()};
    }
};

/*
 * Verifies that saveFileAtomically() creates a file and that openFile()
 * can open it again.
 */
TEST_F(FileHandlerTest, SavesAndOpensFile) {
    const auto filename = path("example.txt").string();
    const std::string content = "Hello, GoogleTest!";
    std::ifstream file;

    FileHandler::saveFileAtomically(filename, asBytes(content));

    EXPECT_TRUE(FileHandler::fileExists(filename));

    EXPECT_NO_THROW(file = FileHandler::openFile(filename));

    ASSERT_TRUE(file.is_open());
    EXPECT_EQ(readFile(file), content);
}

/*
 * Verifies that saving empty content still creates an empty file.
 */
TEST_F(FileHandlerTest, SavesEmptyFile) {
    const auto filename = path("empty.txt").string();
    const std::string content;
    std::ifstream file;

    FileHandler::saveFileAtomically(filename, asBytes(content));

    ASSERT_TRUE(FileHandler::fileExists(filename));

    EXPECT_NO_THROW(file = FileHandler::openFile(filename));

    ASSERT_TRUE(file.is_open());
    EXPECT_TRUE(readFile(file).empty());
}

/*
 * Verifies that saving to an existing filename replaces its content.
 */
TEST_F(FileHandlerTest, ReplacesExistingFile) {
    const auto filename = path("replace.txt").string();
    std::ifstream file;

    const std::string initialContent = "Original content";
    EXPECT_NO_THROW(FileHandler::saveFileAtomically(filename, asBytes(initialContent)));

    const std::string newContent = "Updated content";
    EXPECT_NO_THROW(FileHandler::saveFileAtomically(filename, asBytes(newContent)));

    EXPECT_NO_THROW(file = FileHandler::openFile(filename));

    ASSERT_TRUE(file.is_open());
    EXPECT_EQ(readFile(file), newContent);
}

/*
 * Verifies that fileExists() returns true for a file that was created.
 */
TEST_F(FileHandlerTest, ReportsExistingFile) {
    const auto filename = path("exists.txt").string();
    const std::string content = "Some content";

    FileHandler::saveFileAtomically(filename, asBytes(content));

    EXPECT_TRUE(FileHandler::fileExists(filename));
}

/*
 * Verifies that fileExists() returns false for a missing file.
 */
TEST_F(FileHandlerTest, ReportsMissingFileAsNonexistent) {
    const auto filename = path("does_not_exist.txt").string();

    EXPECT_FALSE(FileHandler::fileExists(filename));
}

/*
 * Verifies that opening a missing file fails.
 */
TEST_F(FileHandlerTest, OpeningMissingFileFails) {
    const auto filename = path("missing.txt").string();

    EXPECT_THROW(FileHandler::openFile(filename), FileNotFoundError);
}

/*
 * Verifies that binary data is saved without being modified.
 *
 * The content includes:
 * - Non-printable bytes
 * - A null byte
 * - The byte value 0xFF
 */
TEST_F(FileHandlerTest, PreservesBinaryContent) {
    const auto filename = path("binary.dat").string();
    std::ifstream file;

    /*
     * The explicit char conversion is used because 0xFF may not fit in a
     * signed char on all platforms.
     */
    const std::string content{'\x01', '\x02', '\x00', '\x03', static_cast<char>(0xFF)};

    EXPECT_NO_THROW(FileHandler::saveFileAtomically(filename, asBytes(content)));

    EXPECT_NO_THROW(file = FileHandler::openFile(filename, std::ios::in | std::ios::binary));

    ASSERT_TRUE(file.is_open());
    EXPECT_EQ(readFile(file), content);
}

/*
 * Verifies that newline characters and multiple lines are preserved.
 */
TEST_F(FileHandlerTest, PreservesMultilineContent) {
    const auto filename = path("multiline.txt").string();
    std::ifstream file;

    const std::string content = "First line\n"
                                "Second line\n"
                                "Third line\n";

    EXPECT_NO_THROW(FileHandler::saveFileAtomically(filename, asBytes(content)));

    EXPECT_NO_THROW(file = FileHandler::openFile(filename));

    ASSERT_TRUE(file.is_open());
    EXPECT_EQ(readFile(file), content);
}
