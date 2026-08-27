/*
 * @brief CryptoService class provides cryptographic services such as key derivation,
 * authentication, decryption and encryption.
 *
 * @author Nolan Evard
 * @date 27.08.2026
 */
#pragma once

#include <cstdint>
#include <sodium.h>
#include <stdexcept>
#include <string>
#include <vector>

#include "Types.h"

#define KDF_ALGORITHM crypto_pwhash_ALG_ARGON2ID13

class CryptoService {
  private:
    CryptoService() = delete;
    CryptoService(const CryptoService&) = delete;
    CryptoService& operator=(const CryptoService&) = delete;
    CryptoService(CryptoService&&) = delete;

  public:
    /*
     * @brief Derives a cryptographic key from a master password using the Argon2id algorithm.
     *
     * @param masterpass The master password from which to derive the key.
     * @param keyLen The desired length of the derived key in bytes.
     * @param salt A unique salt value to use in the key derivation process.
     * @param opLimits The computational cost of the key derivation process (default:
     * crypto_pwhash_OPSLIMIT_INTERACTIVE).
     * @param memLimit The memory cost of the key derivation process (default:
     * crypto_pwhash_MEMLIMIT_INTERACTIVE).
     *
     * @return A vector containing the derived key.
     *
     * @throws KeyDerivationError If the key derivation process fails or if the input parameters are
     * invalid.
     */
    static std::vector<uint8_t> deriveKey(const std::string& masterpass, uint64_t keyLen,
                                          Bytes salt,
                                          uint64_t opLimits = crypto_pwhash_OPSLIMIT_INTERACTIVE,
                                          uint64_t memLimit = crypto_pwhash_MEMLIMIT_INTERACTIVE);

    /*
     * @brief Authenticates a message using a given key and returns the authentication MAC.
     *
     * @param key The authentication key to use for generating the MAC.
     * @param content The message content to authenticate.
     *
     * @return The authentication MAC generated for the given content.
     *
     * @throws AuthenticationError If the authentication process fails.
     */
    static AuthMAC authenticate(const AuthKey& key, Bytes content);

    /*
     * @brief Verifies the authenticity of a message using a given key and authentication MAC.
     *
     * @param key The authentication key to use for verification.
     * @param in The authentication MAC to verify against.
     * @param content The message content to verify.
     *
     * @return True if the authentication MAC is valid for the given content and key, false
     * otherwise.
     */
    static bool verify(const AuthKey& key, const AuthMAC& in, Bytes content);

    /*
     * @brief Decrypts a ciphertext using a given key, nonce, and authentication MAC.
     *
     * @param key The encryption/decryption key to use for decryption.
     * @param nonce The nonce used during encryption.
     * @param mac The authentication MAC generated during encryption.
     * @param ciphertext The ciphertext to decrypt.
     *
     * @return A vector containing the decrypted plaintext.
     *
     * @throws DecryptionError If the decryption process fails or if the key or the authentication
     * MAC is invalid.
     */
    static std::vector<uint8_t> decrypt(const EncKey& key, const EncNonce& nonce, const EncMAC& mac,
                                        Bytes ciphertext);
};

// ----------  CryptoService exceptions ---------------
class CryptoServiceError : public std::runtime_error {
  public:
    // Makes CryptoServiceError inherit runtime_error constructors
    using std::runtime_error::runtime_error;
};

class KeyDerivationError : public CryptoServiceError {
  public:
    using CryptoServiceError::CryptoServiceError;
};

class AuthenticationError : public CryptoServiceError {
  public:
    using CryptoServiceError::CryptoServiceError;
};

class DecryptionError : public CryptoServiceError {
  public:
    using CryptoServiceError::CryptoServiceError;
};
