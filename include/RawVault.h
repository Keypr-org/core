#pragma once

#include "Types.h"
#include "VaultHeader.h"
#include <array>
#include <cstdint>
#include <stdexcept>
#include <vector>

// Defines the size of a raw vault in bytes.
#define RAW_VAULT_BYTES 132
// Defines the minimum size of a vault body in bytes. This was determined by measuring the size of a
// minimal vault body
#define VAULT_BODY_MIN_SIZE 142
// Defines the minimum size of a vault file in bytes.
#define VAULT_FILE_MIN_SIZE (VAULT_HEADER_BYTES + RAW_VAULT_BYTES + VAULT_BODY_MIN_SIZE)

class RawVault {
  private:
    const VaultHeader _header;
    const std::array<uint8_t, 32> _headerMAC;
    const std::array<uint8_t, 24> _xSalsa20Nonce;
    const std::array<uint8_t, 16> _ciphertextMAC;
    const std::vector<uint8_t> _ciphertext;

    // Constructor is private as a RawVault should only be instanciated using its parse() method
    RawVault(VaultHeader header, std::array<uint8_t, 32> headerMAC,
             std::array<uint8_t, 24> xSalsa20Nonce, std::array<uint8_t, 16> ciphertextMAC,
             std::vector<uint8_t> ciphertext)
        : _header(header), _headerMAC(headerMAC), _xSalsa20Nonce(xSalsa20Nonce),
          _ciphertextMAC(ciphertextMAC), _ciphertext(ciphertext) {}

  public:
    // Getters
    VaultHeader header() const { return _header; }
    Bytes headerMAC() const { return _headerMAC; }
    Bytes xSalsa20Nonce() const { return _xSalsa20Nonce; }
    Bytes ciphertextMAC() const { return _ciphertextMAC; }
    Bytes ciphertext() const { return _ciphertext; }

    static RawVault parse(Bytes data);
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
