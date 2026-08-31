#include <gtest/gtest.h>

#include <filesystem>
#include <sodium.h>
#include <sodium/crypto_secretbox.h>

#include "CryptoService.h"
#include "FileHandler.h"
#include "RawVault.h"
#include "Types.h"
#include "VaultRepository.h"

namespace fs = std::filesystem;

class VaultRepositoryTest : public ::testing::Test {
  protected:
    fs::path testDirectory;
    std::string masterpass = "pass";

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

    std::string createVaultFile(bool valid) {
        json vaultJson = makeVaultJson();
        std::string vaultBody = vaultJson.dump();

        std::vector<uint8_t> testData(RAW_VAULT_BYTES + vaultBody.size());

        if (sodium_init() < 0) {
            throw std::runtime_error("Failed to initialize libsodium");
        }

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

        // Create XSalsa20 nonce
        std::array<uint8_t, 24> xSalsa20Nonce;
        randombytes_buf(xSalsa20Nonce.data(), xSalsa20Nonce.size());
        std::copy(xSalsa20Nonce.begin(), xSalsa20Nonce.end(),
                  testData.begin() + XSALSA20_NONCE_OFFSET);

        // Create header MAC
        std::vector<uint8_t> key =
            CryptoService::deriveKey(masterpass, crypto_secretbox_KEYBYTES + crypto_auth_KEYBYTES,
                                     argon2Salt, argon2OpLimit, argon2MemLimit);
        std::vector<uint8_t> authKey(key.begin() + crypto_secretbox_KEYBYTES, key.end());
        std::array<uint8_t, 32> headerMAC;
        crypto_auth(headerMAC.data(), testData.data(), VAULT_HEADER_BYTES, authKey.data());

        std::copy(headerMAC.begin(), headerMAC.end(), testData.begin() + HEADER_MAC_OFFSET);

        // Create ciphertextMAC + ciphertext from makeVaultJson()
        // Encrypt the vault body with XSalsa20 using a key derived from the master password and the
        // Argon2 salt
        std::vector<uint8_t> ciphertext(vaultBody.size());
        std::array<uint8_t, CIPHERTEXT_MAC_BYTES> ciphertextMAC;

        std::vector<uint8_t> encKey(key.begin(), key.begin() + crypto_secretbox_KEYBYTES);

        crypto_secretbox_detached(ciphertext.data(), ciphertextMAC.data(),
                                  reinterpret_cast<const uint8_t*>(vaultBody.data()),
                                  vaultBody.size(), xSalsa20Nonce.data(), encKey.data());

        std::copy(ciphertextMAC.begin(), ciphertextMAC.end(),
                  testData.begin() + CIPHERTEXT_MAC_OFFSET);
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
TEST_F(VaultRepositoryTest, vaultExistsReturnsTrueForValidVaultFile) {
    const auto filename = createVaultFile(true);

    VaultRepository repo;
    EXPECT_TRUE(repo.vaultExists(filename));
}

/*
 * vaultExists returns false if given file doesn't exist
 */
TEST_F(VaultRepositoryTest, vaultExistsReturnsFalseForMissingFile) {
    const auto filename = path("missing_vaultfile.kvdb").string();

    VaultRepository repo;
    EXPECT_FALSE(repo.vaultExists(filename));
}

/*
 * vaultExists returns false if given file exists but is not a valid vault file (RawVault parsing
 * fails)
 */
TEST_F(VaultRepositoryTest, vaultExistsReturnsFalseForInvalidVaultFile) {
    const auto filename = createVaultFile(false);

    VaultRepository repo;
    EXPECT_FALSE(repo.vaultExists(filename));
}

/*
 * unlockVault returns a VaultSession for a valid vault file and correct master password
 */
TEST_F(VaultRepositoryTest,
       unlockVaultReturnsVaultSessionForValidVaultFileAndCorrectMasterPassword) {
    const auto filename = createVaultFile(true);

    VaultRepository repo;
    EXPECT_NO_THROW({
        auto session = repo.unlockVault(masterpass, filename);
        EXPECT_NE(session, nullptr);
        EXPECT_EQ(session->getName(), "Parsed Vault");
    });
}

/*
 * unlockVault returns nullptr for a valid vault file but incorrect master password
 */
TEST_F(VaultRepositoryTest, unlockVaultReturnsNullptrForValidVaultFileAndIncorrectMasterPassword) {
    const auto filename = createVaultFile(true);

    VaultRepository repo;
    EXPECT_NO_THROW({
        auto session = repo.unlockVault("wrongpass", filename);
        EXPECT_EQ(session, nullptr);
    });
}

/*
 * unlockVault throws UnlockVaultError for a valid vault file but corrupted vault file (RawVault
 * parsing fails)
 */
TEST_F(VaultRepositoryTest, unlockVaultThrowsUnlockVaultErrorForCorruptedVaultFile) {
    const auto filename = createVaultFile(false);

    VaultRepository repo;
    EXPECT_THROW(repo.unlockVault(masterpass, filename), UnlockVaultError);
}

/*
 * createVault creates a new vault with the given name and returns a VaultSession
 */
TEST_F(VaultRepositoryTest, createVaultCreatesNewVaultAndReturnsVaultSession) {
    VaultRepository repo;
    EXPECT_NO_THROW({
        auto session = repo.createVault("New Vault", masterpass);
        EXPECT_NE(session, nullptr);
        EXPECT_EQ(session->getName(), "New Vault");
    });
}
