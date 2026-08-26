#pragma once

#include "Types.h"
#include <array>
#include <cstdint>
#include <stdexcept>

// Defines the size of the vault header in bytes. This is used for validation and parsing.
#define VAULT_HEADER_BYTES 60
#define VAULT_MAGIC_BYTES "KVLT2026"
#define VAULT_FORMAT_CURRENT_VERSION 1

// Header offsets constants
#define MAGIC_BYTES_OFFSET 0
#define FORMAT_VERSION_OFFSET 8
#define ARGON2_SALT_OFFSET 12
#define ARGON2_OPSLIMIT_OFFSET 44
#define ARGON2_MEMLIMIT_OFFSET 52

// Header size constants
#define MAGIC_BYTES_SIZE 8
#define ARGON2_SALT_SIZE 32

class VaultHeader {
  private:
    const std::array<uint8_t, MAGIC_BYTES_SIZE> _magicBytes;
    const uint32_t _formatVersion;
    const std::array<uint8_t, ARGON2_SALT_SIZE> _argon2Salt;
    const uint64_t _argon2OpLimit;
    const uint64_t _argon2MemLimit;

    // Constructor is private because VaultHeader should always be created using its parse method
    VaultHeader(std::array<uint8_t, MAGIC_BYTES_SIZE> magicBytes, uint32_t formatVersion,
                std::array<uint8_t, ARGON2_SALT_SIZE> argon2Salt, uint64_t argon2OpLimit,
                uint64_t argon2MemLimit)
        : _magicBytes(magicBytes), _formatVersion(formatVersion), _argon2Salt(argon2Salt),
          _argon2OpLimit(argon2OpLimit), _argon2MemLimit(argon2MemLimit) {}

    static uint32_t read_u32_le(Bytes data);
    static uint64_t read_u64_le(Bytes data);

  public:
    // Getters
    Bytes magicBytes() const { return _magicBytes; }
    uint32_t formatVersion() const { return _formatVersion; }
    Bytes argon2Salt() const { return _argon2Salt; }
    uint64_t argon2OpLimit() const { return _argon2OpLimit; }
    uint64_t argon2MemLimit() const { return _argon2MemLimit; }

    static VaultHeader parse(Bytes data);
};

// Custom exceptions
class VaultHeaderError : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

class VaultHeaderParsingError : public VaultHeaderError {
  public:
    using VaultHeaderError::VaultHeaderError;
};
