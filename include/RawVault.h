#pragma once

#include "Types.h"
#include "VaultHeader.h"
#include <array>
#include <cstdint>
#include <stdexcept>
#include <vector>

// Size constants
#define HEADER_MAC_BYTES 32
#define XSALSA20_NONCE_BYTES 24
#define CIPHERTEXT_MAC_BYTES 16
// Offsets constants
#define HEADER_OFFSET 0
#define HEADER_MAC_OFFSET (HEADER_OFFSET + VAULT_HEADER_BYTES)
#define XSALSA20_NONCE_OFFSET (HEADER_MAC_OFFSET + HEADER_MAC_BYTES)
#define CIPHERTEXT_MAC_OFFSET (XSALSA20_NONCE_OFFSET + XSALSA20_NONCE_BYTES)
#define CIPHERTEXT_OFFSET (CIPHERTEXT_MAC_OFFSET + CIPHERTEXT_MAC_BYTES)

// Defines the minimum size of a vault body in bytes. This was determined by measuring the size of a
// minimal vault body
#define VAULT_BODY_MIN_SIZE 119
// Defines the size of a raw vault in bytes.
#define RAW_VAULT_BYTES (CIPHERTEXT_OFFSET)
// Defines the minimum size of a vault file in bytes.
#define VAULT_FILE_MIN_SIZE (RAW_VAULT_BYTES + VAULT_BODY_MIN_SIZE)

class RawVault {
  private:
    const VaultHeader _header;
    const AuthMAC _headerMAC;
    const std::array<uint8_t, XSALSA20_NONCE_BYTES> _xSalsa20Nonce;
    const std::array<uint8_t, CIPHERTEXT_MAC_BYTES> _ciphertextMAC;
    const std::vector<uint8_t> _ciphertext;

  public:
    RawVault(VaultHeader header, AuthMAC headerMAC,
             std::array<uint8_t, XSALSA20_NONCE_BYTES> xSalsa20Nonce,
             std::array<uint8_t, CIPHERTEXT_MAC_BYTES> ciphertextMAC,
             std::vector<uint8_t> ciphertext)
        : _header(header), _headerMAC(headerMAC), _xSalsa20Nonce(xSalsa20Nonce),
          _ciphertextMAC(ciphertextMAC), _ciphertext(ciphertext) {}
    // Getters
    const VaultHeader& header() const { return _header; }
    std::array<uint8_t, VAULT_HEADER_BYTES> rawHeader() const {
        return VaultHeader::serialize(_header);
    }
    const AuthMAC& headerMAC() const { return _headerMAC; }
    const EncNonce& xSalsa20Nonce() const { return _xSalsa20Nonce; }
    const EncMAC& ciphertextMAC() const { return _ciphertextMAC; }
    Bytes ciphertext() const { return _ciphertext; }

    /*
     * @brief Parses a RawVault from a byte array.
     *
     * @param data The byte array to parse.
     *
     * @return A RawVault object.
     *
     * @throws RawVaultParsingError If the data is too small to be a valid vault file or if the
     * header cannot be parsed.
     */
    static RawVault parse(Bytes data);

    /*
     * @brief Serializes a RawVault into a byte array.
     *
     * @param vault The RawVault object to serialize.
     *
     * @return A byte array representing the serialized RawVault.
     *
     * @throws RawVaultSerializeError If serialization goes wrong
     */
    static std::vector<uint8_t> serialize(const RawVault& vault);
};

// Custom exceptions for the RawVault class

class RawVaultError : public std::runtime_error {
  public:
    using runtime_error::runtime_error;
};

class RawVaultParsingError : public RawVaultError {
  public:
    using RawVaultError::RawVaultError;
};

class RawVaultSerializeError : public RawVaultError {
  public:
    using RawVaultError::RawVaultError;
};
