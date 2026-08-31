#include <gtest/gtest.h>

#include <filesystem>

#include "FileHandler.h"
#include "RawVault.h"
#include "Types.h"
#include "VaultRepository.h"

namespace fs = std::filesystem;

class FileHandlerTest : public ::testing::Test {
  protected:
    fs::path testDirectory;

    void SetUp() override {
        const auto timestamp = std::chrono::steady_clock::now().time_since_epoch().count();

        testDirectory =
            fs::temp_directory_path() / ("vault_repositry_test_" + std::to_string(timestamp));

        fs::create_directories(testDirectory);
    }

    void TearDown() override {
        std::error_code error;
        fs::remove_all(testDirectory, error);
    }

    fs::path path(const std::string& filename) const { return testDirectory / filename; }

    std::string createVaultFile(bool valid) {
        std::array<uint8_t, VAULT_FILE_MIN_SIZE> testData;
        fs::path filepath = path((valid ? "valid" : "invalid") + std::string("_vaultfile.kvdb"));

        // Create header
        std::string magicString = (valid ? VAULT_MAGIC_BYTES : std::string(MAGIC_BYTES_SIZE, 'x'));
        std::copy(magicString.begin(), magicString.end(), testData.begin());
        uint32_t formatVersion = VAULT_FORMAT_CURRENT_VERSION;
        std::copy(reinterpret_cast<uint8_t*>(&formatVersion),
                  reinterpret_cast<uint8_t*>(&formatVersion) + sizeof(formatVersion),
                  testData.begin() + FORMAT_VERSION_OFFSET);
        std::array<uint8_t, ARGON2_SALT_SIZE> argon2Salt;
        randombytes_buf(argon2Salt.data(), argon2Salt.size());
        std::copy(argon2Salt.begin(), argon2Salt.end(), testData.begin() + ARGON2_SALT_OFFSET);
        uint64_t argon2OpLimit = crypto_pwhash_OPSLIMIT_INTERACTIVE;
        std::copy(reinterpret_cast<uint8_t*>(&argon2OpLimit),
                  reinterpret_cast<uint8_t*>(&argon2OpLimit) + sizeof(argon2OpLimit),
                  testData.begin() + ARGON2_OPSLIMIT_OFFSET);
        uint64_t argon2MemLimit = crypto_pwhash_MEMLIMIT_INTERACTIVE;
        std::copy(reinterpret_cast<uint8_t*>(&argon2MemLimit),
                  reinterpret_cast<uint8_t*>(&argon2MemLimit) + sizeof(argon2MemLimit),
                  testData.begin() + ARGON2_MEMLIMIT_OFFSET);

        // Create header MAC
        std::array<uint8_t, 32> headerMAC;
        randombytes_buf(headerMAC.data(), headerMAC.size());
        std::copy(headerMAC.begin(), headerMAC.end(), testData.begin() + HEADER_MAC_OFFSET);

        // Create XSalsa20 nonce
        std::array<uint8_t, 24> xSalsa20Nonce;
        randombytes_buf(xSalsa20Nonce.data(), xSalsa20Nonce.size());
        std::copy(xSalsa20Nonce.begin(), xSalsa20Nonce.end(),
                  testData.begin() + XSALSA20_NONCE_OFFSET);

        // Create ciphertext MAC
        std::array<uint8_t, 16> ciphertextMAC;
        randombytes_buf(ciphertextMAC.data(), ciphertextMAC.size());
        std::copy(ciphertextMAC.begin(), ciphertextMAC.end(),
                  testData.begin() + CIPHERTEXT_MAC_OFFSET);
        // Create ciphertext
        std::vector<uint8_t> ciphertext(VAULT_BODY_MIN_SIZE, 0);
        std::copy(ciphertext.begin(), ciphertext.end(), testData.begin() + CIPHERTEXT_OFFSET);

        // Write file
        FileHandler::saveFileAtomically(filepath.string(), Bytes{testData.data(), testData.size()});

        return filepath.string();
    }
};

/*
 * vaultExists returns true if given file exists and is a valid vault file (RawVault parsing
 * succeeds)
 */
TEST_F(FileHandlerTest, vaultExistsReturnsTrueForValidVaultFile) {
    const auto filename = createVaultFile(true);

    VaultRepository repo;
    EXPECT_TRUE(repo.vaultExists(filename));
}

/*
 * vaultExists returns false if given file doesn't exist
 */
TEST_F(FileHandlerTest, vaultExistsReturnsFalseForMissingFile) {
    const auto filename = path("missing_vaultfile.kvdb").string();

    VaultRepository repo;
    EXPECT_FALSE(repo.vaultExists(filename));
}

/*
 * vaultExists returns false if given file exists but is not a valid vault file (RawVault parsing
 * fails)
 */
TEST_F(FileHandlerTest, vaultExistsReturnsFalseForInvalidVaultFile) {
    const auto filename = createVaultFile(false);

    VaultRepository repo;
    EXPECT_FALSE(repo.vaultExists(filename));
}
