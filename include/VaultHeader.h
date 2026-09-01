#pragma once

#include "Types.h"
#include <array>
#include <cstdint>
#include <stdexcept>

// Defines the size of the vault header in bytes. This is used for validation and parsing.
#define VAULT_HEADER_BYTES 44
#define VAULT_MAGIC_BYTES "KVLT2026"
#define VAULT_FORMAT_CURRENT_VERSION 1

// Header offsets constants
#define MAGIC_BYTES_OFFSET 0
#define FORMAT_VERSION_OFFSET 8
#define ARGON2_SALT_OFFSET 12
#define ARGON2_OPSLIMIT_OFFSET 28
#define ARGON2_MEMLIMIT_OFFSET 36

// Header size constants
#define MAGIC_BYTES_SIZE 8
#define ARGON2_SALT_SIZE 16

class VaultHeader {
  private:
    const std::array<uint8_t, MAGIC_BYTES_SIZE> _magicBytes;
    const uint32_t _formatVersion;
    const std::array<uint8_t, ARGON2_SALT_SIZE> _argon2Salt;
    const uint64_t _argon2OpLimit;
    const uint64_t _argon2MemLimit;

    /*
     * @brief Reads a 32-bit unsigned integer from a byte span in little-endian order.
     *
     * @param data A span of bytes from which to read the integer. Must be at least 4 bytes long.
     *
     * @return The 32-bit unsigned integer read from the byte span.
     *
     * @throws VaultHeaderParsingError if the provided data span is smaller than 4 bytes.
     */
    static uint32_t read_u32_le(Bytes data);

    /*
     * @brief Reads a 64-bit unsigned integer from a byte span in little-endian order.
     *
     * @param data A span of bytes from which to read the integer. Must be at least 8 bytes long.
     *
     * @return The 64-bit unsigned integer read from the byte span.
     *
     * @throws VaultHeaderParsingError if the provided data span is smaller than 8 bytes.
     */
    static uint64_t read_u64_le(Bytes data);

    static void write_u32_le(uint32_t value, MutableBytes data);

    static void write_u64_le(uint64_t value, MutableBytes data);

  public:
    VaultHeader(std::array<uint8_t, MAGIC_BYTES_SIZE> magicBytes, uint32_t formatVersion,
                std::array<uint8_t, ARGON2_SALT_SIZE> argon2Salt, uint64_t argon2OpLimit,
                uint64_t argon2MemLimit)
        : _magicBytes(magicBytes), _formatVersion(formatVersion), _argon2Salt(argon2Salt),
          _argon2OpLimit(argon2OpLimit), _argon2MemLimit(argon2MemLimit) {}
    // Getters
    Bytes magicBytes() const { return _magicBytes; }
    uint32_t formatVersion() const { return _formatVersion; }
    Bytes argon2Salt() const { return _argon2Salt; }
    uint64_t argon2OpLimit() const { return _argon2OpLimit; }
    uint64_t argon2MemLimit() const { return _argon2MemLimit; }

    /*
     * @brief Parses a byte span into a VaultHeader object.
     *
     * @param data A span of bytes representing the vault header. Must be at least
     * VAULT_HEADER_BYTES long.
     *
     * @return A VaultHeader object constructed from the provided byte span.
     *
     * @throws VaultHeaderParsingError if the provided data is too small, has invalid magic bytes,
     *         or contains unsupported format version or invalid Argon2 parameters.
     */
    static VaultHeader parse(Bytes data);

    /*
     * @brief Serializes a VaultHeader object into a byte vector.
     *
     * @param header The VaultHeader object to serialize.
     *
     * @return A vector of bytes representing the serialized VaultHeader.
     *
     * @throws VaultHeaderSerializeError if serialization fails for any reason.
     */
    static std::array<uint8_t, VAULT_HEADER_BYTES> serialize(const VaultHeader& header);
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

class VaultHeaderSerializeError : public VaultHeaderError {
  public:
    using VaultHeaderError::VaultHeaderError;
};
