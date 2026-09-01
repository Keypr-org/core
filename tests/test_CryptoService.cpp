/*
 * @brief Unit tests for the CryptoService class.
 *
 * These tests were written with the assistance of AI. They verify the correctness of key
 * derivation, authentication, and decryption operations provided by the CryptoService class.
 *
 * @author Nolan Evard (with AI assistance)
 * @date 27.08.2026
 */
#include <gtest/gtest.h>
#include <sodium.h>

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "CryptoService.h"
#include "Types.h"

class CryptoServiceTest : public ::testing::Test {
  protected:
    void SetUp() override { ASSERT_GE(sodium_init(), 0); }

    static Bytes asBytes(const std::vector<uint8_t>& data) {
        return Bytes{reinterpret_cast<const unsigned char*>(data.data()), data.size()};
    }
};

/*
 * Verifies that key derivation produces the same key as libsodium directly.
 */
TEST_F(CryptoServiceTest, DeriveKeyReturnsExpectedKey) {
    const std::string password = "correct horse battery staple";

    std::array<unsigned char, crypto_pwhash_SALTBYTES> salt{};
    for (std::size_t i = 0; i < salt.size(); ++i) {
        salt[i] = static_cast<unsigned char>(i);
    }

    constexpr std::size_t keyLength = 32;
    constexpr uint64_t opsLimit = crypto_pwhash_OPSLIMIT_MIN;
    constexpr std::size_t memLimit = crypto_pwhash_MEMLIMIT_MIN;

    std::vector<uint8_t> expected(keyLength);

    ASSERT_EQ(crypto_pwhash(expected.data(), expected.size(), password.data(), password.size(),
                            salt.data(), opsLimit, memLimit, KDF_ALGORITHM),
              0);

    const auto actual = CryptoService::deriveKey(
        password, keyLength, Bytes{salt.data(), salt.size()}, opsLimit, memLimit);

    EXPECT_EQ(actual, expected);
}

/*
 * Verifies that different passwords produce different derived keys.
 */
TEST_F(CryptoServiceTest, DeriveKeyProducesDifferentKeysForDifferentPasswords) {
    std::array<unsigned char, crypto_pwhash_SALTBYTES> salt{};

    const auto first =
        CryptoService::deriveKey("password-one", 32, Bytes{salt.data(), salt.size()},
                                 crypto_pwhash_OPSLIMIT_MIN, crypto_pwhash_MEMLIMIT_MIN);

    const auto second =
        CryptoService::deriveKey("password-two", 32, Bytes{salt.data(), salt.size()},
                                 crypto_pwhash_OPSLIMIT_MIN, crypto_pwhash_MEMLIMIT_MIN);

    EXPECT_NE(first, second);
}

/*
 * Verifies that key derivation throws when the salt has an invalid size.
 */
TEST_F(CryptoServiceTest, DeriveKeyThrowsForInvalidSalt) {
    std::array<unsigned char, crypto_pwhash_SALTBYTES - 1> invalidSalt{};

    EXPECT_THROW(CryptoService::deriveKey("password", 32,
                                          Bytes{invalidSalt.data(), invalidSalt.size()},
                                          crypto_pwhash_OPSLIMIT_MIN, crypto_pwhash_MEMLIMIT_MIN),
                 KeyDerivationError);
}

/*
 * Verifies that authentication produces the same MAC as libsodium directly.
 */
TEST_F(CryptoServiceTest, AuthenticateReturnsExpectedMAC) {
    AuthKey key{};
    for (std::size_t i = 0; i < key.size(); ++i) {
        key[i] = static_cast<uint8_t>(i);
    }

    const std::string message = "message to authenticate";

    AuthMAC expected{};
    ASSERT_EQ(crypto_auth(expected.data(), reinterpret_cast<const unsigned char*>(message.data()),
                          message.size(), key.data()),
              0);

    const AuthMAC actual = CryptoService::authenticate(
        key, Bytes{reinterpret_cast<const unsigned char*>(message.data()), message.size()});

    EXPECT_EQ(actual, expected);
}

/*
 * Verifies that different content produces different authentication MACs.
 */
TEST_F(CryptoServiceTest, AuthenticateProducesDifferentMACsForDifferentContent) {
    AuthKey key{};

    const std::string firstMessage = "first message";
    const std::string secondMessage = "second message";

    const auto first = CryptoService::authenticate(
        key,
        Bytes{reinterpret_cast<const unsigned char*>(firstMessage.data()), firstMessage.size()});

    const auto second = CryptoService::authenticate(
        key,
        Bytes{reinterpret_cast<const unsigned char*>(secondMessage.data()), secondMessage.size()});

    EXPECT_NE(first, second);
}

/*
 * Verifies that valid ciphertext is correctly decrypted to the original plaintext.
 */
TEST_F(CryptoServiceTest, DecryptReturnsOriginalPlaintext) {
    EncKey key{};
    EncNonce nonce{};

    for (std::size_t i = 0; i < key.size(); ++i) {
        key[i] = static_cast<uint8_t>(i);
    }

    for (std::size_t i = 0; i < nonce.size(); ++i) {
        nonce[i] = static_cast<uint8_t>(i + 1);
    }

    const std::string plaintext = "secret message";

    std::vector<uint8_t> ciphertext(plaintext.size());
    EncMAC mac{};

    ASSERT_EQ(crypto_secretbox_detached(ciphertext.data(), mac.data(),
                                        reinterpret_cast<const unsigned char*>(plaintext.data()),
                                        plaintext.size(), nonce.data(), key.data()),
              0);

    const auto decrypted = CryptoService::decrypt(
        key, nonce, mac,
        Bytes{reinterpret_cast<const unsigned char*>(ciphertext.data()), ciphertext.size()});

    EXPECT_EQ(decrypted, std::vector<uint8_t>(plaintext.begin(), plaintext.end()));
}

/*
 * Verifies that decryption rejects modified ciphertext.
 */
TEST_F(CryptoServiceTest, DecryptThrowsWhenCiphertextIsTampered) {
    EncKey key{};
    EncNonce nonce{};

    const std::string plaintext = "secret message";

    std::vector<uint8_t> ciphertext(plaintext.size());
    EncMAC mac{};

    ASSERT_EQ(crypto_secretbox_detached(ciphertext.data(), mac.data(),
                                        reinterpret_cast<const unsigned char*>(plaintext.data()),
                                        plaintext.size(), nonce.data(), key.data()),
              0);

    ciphertext[0] ^= 0x01;

    EXPECT_THROW(
        CryptoService::decrypt(key, nonce, mac, Bytes{ciphertext.data(), ciphertext.size()}),
        DecryptionError);
}

/*
 * Verifies that decryption rejects a modified authentication MAC.
 */
TEST_F(CryptoServiceTest, DecryptThrowsWhenMACIsTampered) {
    EncKey key{};
    EncNonce nonce{};

    const std::string plaintext = "secret message";

    std::vector<uint8_t> ciphertext(plaintext.size());
    EncMAC mac{};

    ASSERT_EQ(crypto_secretbox_detached(ciphertext.data(), mac.data(),
                                        reinterpret_cast<const unsigned char*>(plaintext.data()),
                                        plaintext.size(), nonce.data(), key.data()),
              0);

    mac[0] ^= 0x01;

    EXPECT_THROW(
        CryptoService::decrypt(key, nonce, mac, Bytes{ciphertext.data(), ciphertext.size()}),
        DecryptionError);
}

/*
 * Verifies that a valid MAC is accepted for the corresponding content.
 */
TEST_F(CryptoServiceTest, VerifyReturnsTrueForValidMAC) {
    AuthKey key{};

    const std::string message = "message to verify";

    const AuthMAC mac = CryptoService::authenticate(
        key, Bytes{reinterpret_cast<const unsigned char*>(message.data()), message.size()});

    const bool result = CryptoService::verify(
        key, mac, Bytes{reinterpret_cast<const unsigned char*>(message.data()), message.size()});

    EXPECT_TRUE(result);
}

/*
 * Verifies that verification rejects a modified MAC.
 */
TEST_F(CryptoServiceTest, VerifyReturnsFalseForTamperedMAC) {
    AuthKey key{};

    const std::string message = "message to verify";

    AuthMAC mac = CryptoService::authenticate(
        key, Bytes{reinterpret_cast<const unsigned char*>(message.data()), message.size()});

    mac[0] ^= 0x01;

    const bool result = CryptoService::verify(
        key, mac, Bytes{reinterpret_cast<const unsigned char*>(message.data()), message.size()});

    EXPECT_FALSE(result);
}

/*
 *Verifies that verification rejects content different from the content used to create the MAC.
 */
TEST_F(CryptoServiceTest, VerifyReturnsFalseForModifiedContent) {
    AuthKey key{};

    const std::string originalMessage = "original message";
    const std::string modifiedMessage = "modified message";

    const AuthMAC mac = CryptoService::authenticate(
        key, Bytes{reinterpret_cast<const unsigned char*>(originalMessage.data()),
                   originalMessage.size()});

    const bool result =
        CryptoService::verify(key, mac,
                              Bytes{reinterpret_cast<const unsigned char*>(modifiedMessage.data()),
                                    modifiedMessage.size()});

    EXPECT_FALSE(result);
}

/*
 *Verifies that verification rejects a MAC when a different key is used.
 */
TEST_F(CryptoServiceTest, VerifyReturnsFalseForDifferentKey) {
    AuthKey signingKey{};
    AuthKey verificationKey{};

    verificationKey[0] = 0x01;

    const std::string message = "message to verify";

    const AuthMAC mac = CryptoService::authenticate(
        signingKey, Bytes{reinterpret_cast<const unsigned char*>(message.data()), message.size()});

    const bool result = CryptoService::verify(
        verificationKey, mac,
        Bytes{reinterpret_cast<const unsigned char*>(message.data()), message.size()});

    EXPECT_FALSE(result);
}

/*
 * Verifies that encryption and decryption are consistent, i.e., decrypting the ciphertext produced
 * by encrypt returns the original plaintext.
 */
TEST_F(CryptoServiceTest, EncryptAndDecryptAreConsistent) {
    EncKey key{};
    EncNonce nonce{};

    // Generate a key and nonce with known values for testing
    for (std::size_t i = 0; i < key.size(); ++i) {
        key[i] = static_cast<uint8_t>(i);
    }

    for (std::size_t i = 0; i < nonce.size(); ++i) {
        nonce[i] = static_cast<uint8_t>(i + 1);
    }

    const std::string plaintext = "secret message";

    const auto [mac, ciphertext] = CryptoService::encrypt(
        key, nonce,
        Bytes{reinterpret_cast<const unsigned char*>(plaintext.data()), plaintext.size()});

    const auto decrypted = CryptoService::decrypt(
        key, nonce, mac,
        Bytes{reinterpret_cast<const unsigned char*>(ciphertext.data()), ciphertext.size()});

    EXPECT_EQ(decrypted, std::vector<uint8_t>(plaintext.begin(), plaintext.end()));
}
