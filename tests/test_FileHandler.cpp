/*
 * This test file uses GoogleTest to verify that FileHandler.cpp works correctly.
 *
 * It was written using AI assistance as it was the starting point of our testsing journey.
 * It is rich in comments so it can be used as a reference for future tests
 */

#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

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
     * Creates a read-only byte span over a vector's underlying data.
     *
     * This does not copy the data. The vector must remain alive and must not
     * be modified in a way that causes reallocation while the span is used.
     */
    static Bytes asBytes(const std::vector<unsigned char>& data) {
        return Bytes{data.data(), data.size()};
    }

    /*
     * Creates a read-only byte span over a string's underlying data.
     *
     * This does not copy the data. The string must remain alive while the
     * span is used.
     */
    static Bytes asBytes(const std::string& data) {
        return Bytes{reinterpret_cast<const unsigned char*>(data.data()), data.size()};
    }
};

/*
 * Verifies that saveFileAtomically() creates a file and that readFile()
 * can read the saved content.
 */
TEST_F(FileHandlerTest, SavesAndReadsFile) {
    const auto filename = path("example.txt").string();
    const std::string content = "Hello, GoogleTest!";

    ASSERT_NO_THROW(FileHandler::saveFileAtomically(filename, asBytes(content)));

    EXPECT_TRUE(FileHandler::fileExists(filename));

    const std::vector<unsigned char> expected(content.begin(), content.end());

    EXPECT_EQ(FileHandler::readFile(filename), expected);
}

/*
 * Verifies that saving empty content still creates an empty file.
 */
TEST_F(FileHandlerTest, SavesEmptyFile) {
    const auto filename = path("empty.txt").string();
    const std::vector<unsigned char> content;

    ASSERT_NO_THROW(FileHandler::saveFileAtomically(filename, asBytes(content)));

    ASSERT_TRUE(FileHandler::fileExists(filename));
    EXPECT_TRUE(FileHandler::readFile(filename).empty());
}

/*
 * Verifies that saving to an existing filename replaces its content.
 */
TEST_F(FileHandlerTest, ReplacesExistingFile) {
    const auto filename = path("replace.txt").string();

    const std::string initialContent = "Original content";
    ASSERT_NO_THROW(FileHandler::saveFileAtomically(filename, asBytes(initialContent)));

    const std::string newContent = "Updated content";
    ASSERT_NO_THROW(FileHandler::saveFileAtomically(filename, asBytes(newContent)));

    const std::vector<unsigned char> expected(newContent.begin(), newContent.end());

    EXPECT_EQ(FileHandler::readFile(filename), expected);
}

/*
 * Verifies that fileExists() returns true for a file that was created.
 */
TEST_F(FileHandlerTest, ReportsExistingFile) {
    const auto filename = path("exists.txt").string();
    const std::string content = "Some content";

    ASSERT_NO_THROW(FileHandler::saveFileAtomically(filename, asBytes(content)));

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
 * Verifies that reading a missing file throws FileNotFoundError.
 */
TEST_F(FileHandlerTest, ReadingMissingFileFails) {
    const auto filename = path("missing.txt").string();

    EXPECT_THROW(FileHandler::readFile(filename), FileNotFoundError);
}

/*
 * Verifies that saving to a path whose parent directory does not exist
 * throws FileWriteError.
 */
TEST_F(FileHandlerTest, SavingToInvalidPathFails) {
    const auto filename = (testDirectory / "missing_directory" / "file.txt").string();

    const std::string content = "This write should fail.";

    EXPECT_THROW(FileHandler::saveFileAtomically(filename, asBytes(content)), FileNotFoundError);
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

    const std::vector<unsigned char> content = {0x01, 0x02, 0x00, 0x03, 0xFF};

    ASSERT_NO_THROW(FileHandler::saveFileAtomically(filename, asBytes(content)));

    EXPECT_EQ(FileHandler::readFile(filename), content);
}

/*
 * Verifies that newline characters and multiple lines are preserved.
 */
TEST_F(FileHandlerTest, PreservesMultilineContent) {
    const auto filename = path("multiline.txt").string();

    const std::string content = "First line\n"
                                "Second line\n"
                                "Third line\n";

    ASSERT_NO_THROW(FileHandler::saveFileAtomically(filename, asBytes(content)));

    const std::vector<unsigned char> expected(content.begin(), content.end());

    EXPECT_EQ(FileHandler::readFile(filename), expected);
}
