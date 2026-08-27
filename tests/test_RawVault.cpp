#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "RawVault.h"
#include "VaultHeader.h"
#include <algorithm>
#include <sodium.h>

class RawVaultTest : public ::testing::Test {
  protected:
    std::array<uint8_t, VAULT_FILE_MIN_SIZE> testData;

    void SetUp() override {
        // Create "valid" vault data for testing

        // Create header
        std::string magicString = VAULT_MAGIC_BYTES;
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
    }
};

// ------------- TESTS --------------------

/*
 * Data that represents a valid vault does not throw and is correctly parsed
 */
TEST_F(RawVaultTest, ValidDataParsesCorrectly) {
    EXPECT_NO_THROW({
        RawVault vault = RawVault::parse(testData);
        EXPECT_EQ(vault.header().formatVersion(), VAULT_FORMAT_CURRENT_VERSION);
        EXPECT_THAT(
            vault.header().argon2Salt(),
            ::testing::ElementsAreArray(testData.begin() + ARGON2_SALT_OFFSET,
                                        testData.begin() + ARGON2_SALT_OFFSET + ARGON2_SALT_SIZE));
        EXPECT_EQ(vault.header().argon2OpLimit(), crypto_pwhash_OPSLIMIT_INTERACTIVE);
        EXPECT_EQ(vault.header().argon2MemLimit(), crypto_pwhash_MEMLIMIT_INTERACTIVE);
        EXPECT_THAT(
            vault.headerMAC(),
            ::testing::ElementsAreArray(testData.begin() + HEADER_MAC_OFFSET,
                                        testData.begin() + HEADER_MAC_OFFSET + HEADER_MAC_BYTES));
        EXPECT_THAT(vault.xSalsa20Nonce(),
                    ::testing::ElementsAreArray(testData.begin() + XSALSA20_NONCE_OFFSET,
                                                testData.begin() + XSALSA20_NONCE_OFFSET +
                                                    XSALSA20_NONCE_BYTES));
        EXPECT_THAT(vault.ciphertextMAC(),
                    ::testing::ElementsAreArray(testData.begin() + CIPHERTEXT_MAC_OFFSET,
                                                testData.begin() + CIPHERTEXT_MAC_OFFSET +
                                                    CIPHERTEXT_MAC_BYTES));
        EXPECT_THAT(vault.ciphertext(), ::testing::ElementsAreArray(
                                            testData.begin() + CIPHERTEXT_OFFSET, testData.end()));
    });
}

/*
 * Data smaller than the minimum vault file size throws a RawVaultParsingError
 */
TEST_F(RawVaultTest, ParseThrowsOnSmallData) {
    std::vector<uint8_t> smallData(VAULT_FILE_MIN_SIZE - 1, 0);
    EXPECT_THROW(RawVault::parse(smallData), RawVaultParsingError);
}
