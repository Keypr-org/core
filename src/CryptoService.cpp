/*
 * @brief Implementation of the CryptoService class for cryptographic operations using libsodium.
 *
 * @Author Nolan Evard
 * @date 27.08.2026
 */
#include "CryptoService.h"
#include <sodium.h>

std::vector<uint8_t> CryptoService::deriveKey(const std::string& masterpass, uint64_t keyLen,
                                              Bytes salt, uint64_t opLimits, uint64_t memLimit) {
    // Initialize libsodium if it hasn't been initialized yet. It is safe to call sodium_init
    // multiple times
    if (sodium_init() < 0) {
        throw KeyDerivationError("Failed to initialize libsodium");
    }

    std::vector<uint8_t> out(keyLen);
    // Check params boundaries
    if (keyLen == 0)
        return out;
    else if (keyLen < crypto_pwhash_BYTES_MIN)
        throw KeyDerivationError("Key length must be at least " +
                                 std::to_string(crypto_pwhash_BYTES_MIN) + " bytes");
    else if (keyLen > crypto_pwhash_BYTES_MAX)
        throw KeyDerivationError("Key length must be at most " +
                                 std::to_string(crypto_pwhash_BYTES_MAX) + " bytes");

    if (masterpass.size() < crypto_pwhash_PASSWD_MIN) {
        throw KeyDerivationError("Password length must be at least " +
                                 std::to_string(crypto_pwhash_PASSWD_MIN) + " bytes");
    } else if (masterpass.size() > crypto_pwhash_PASSWD_MAX) {
        throw KeyDerivationError("Password length must be at most " +
                                 std::to_string(crypto_pwhash_PASSWD_MAX) + " bytes");
    }

    if (salt.size() != crypto_pwhash_SALTBYTES) {
        throw KeyDerivationError("Invalid salt size");
    }

    if (opLimits < crypto_pwhash_OPSLIMIT_MIN || opLimits > crypto_pwhash_OPSLIMIT_MAX) {
        throw KeyDerivationError("Invalid ops limit");
    }

    if (memLimit < crypto_pwhash_MEMLIMIT_MIN || memLimit > crypto_pwhash_MEMLIMIT_MAX) {
        throw KeyDerivationError("Invalid memory limit");
    }

    if (crypto_pwhash(out.data(), out.size(), masterpass.data(), masterpass.size(), salt.data(),
                      opLimits, memLimit, KDF_ALGORITHM) < 0) {
        throw KeyDerivationError("Key derivation failed");
    }
    return out;
}

AuthMAC CryptoService::authenticate(const AuthKey& key, Bytes content) {
    // Initialize libsodium if it hasn't been initialized yet. It is safe to call sodium_init
    // multiple times
    if (sodium_init() < 0) {
        throw KeyDerivationError("Failed to initialize libsodium");
    }

    AuthMAC out{};
    if (crypto_auth(out.data(), content.data(), content.size(), key.data()) != 0) {
        throw AuthenticationError("Authentication failed");
    }
    return out;
}

bool CryptoService::verify(const AuthKey& key, const AuthMAC& in, Bytes content) {
    if (crypto_auth_verify(in.data(), content.data(), content.size(), key.data()) != 0) {
        return false;
    }
    return true;
}

std::vector<uint8_t> CryptoService::decrypt(const EncKey& key, const EncNonce& nonce,
                                            const EncMAC& mac, Bytes ciphertext) {
    // Initialize libsodium if it hasn't been initialized yet. It is safe to call sodium_init
    // multiple times
    if (sodium_init() < 0) {
        throw KeyDerivationError("Failed to initialize libsodium");
    }

    std::vector<uint8_t> out(ciphertext.size());
    if (ciphertext.size() == 0) {
        return out;
    }
    if (crypto_secretbox_open_detached(out.data(), ciphertext.data(), mac.data(), ciphertext.size(),
                                       nonce.data(), key.data()) != 0) {
        throw DecryptionError("Decryption failed");
    }
    return out;
}

std::pair<std::array<uint8_t, crypto_secretbox_MACBYTES>, std::vector<uint8_t>>
CryptoService::encrypt(const EncKey& key, const EncNonce& nonce, Bytes plaintext) {
    if (sodium_init() < 0) {
        throw EncryptionError("Failed to initialize libsodium");
    }

    std::vector<uint8_t> ciphertext(plaintext.size());

    std::array<uint8_t, crypto_secretbox_MACBYTES> mac{};

    if (plaintext.size() != 0 &&
        crypto_secretbox_detached(ciphertext.data(), mac.data(), plaintext.data(), plaintext.size(),
                                  nonce.data(), key.data()) != 0) {
        throw EncryptionError("Encryption failed");
    }

    return std::make_pair(mac, ciphertext);
}
