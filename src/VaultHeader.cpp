#include "VaultHeader.h"
#include <sodium.h>

/*
 * @brief Parses a byte span into a VaultHeader object.
 *
 * @param data A span of bytes representing the vault header. Must be at least VAULT_HEADER_BYTES
 * long.
 *
 * @return A VaultHeader object constructed from the provided byte span.
 *
 * @throws VaultHeaderParsingError if the provided data is too small, has invalid magic bytes,
 *         or contains unsupported format version or invalid Argon2 parameters.
 */
VaultHeader VaultHeader::parse(Bytes data) {
    if (data.size() < VAULT_HEADER_BYTES) {
        throw VaultHeaderParsingError("Data too small to contain a header");
    }

    std::array<uint8_t, MAGIC_BYTES_SIZE> magicBytes;
    std::copy(data.begin() + MAGIC_BYTES_OFFSET,
              data.begin() + MAGIC_BYTES_OFFSET + MAGIC_BYTES_SIZE, magicBytes.begin());
    if (!std::equal(magicBytes.begin(), magicBytes.end(), VAULT_MAGIC_BYTES)) {
        throw VaultHeaderParsingError("Invalid magic bytes");
    }

    uint32_t formatVersion = read_u32_le(data.subspan(FORMAT_VERSION_OFFSET, sizeof(uint32_t)));
    if (formatVersion != VAULT_FORMAT_CURRENT_VERSION) {
        throw VaultHeaderParsingError("Unsupported format version");
    }

    std::array<uint8_t, ARGON2_SALT_SIZE> argon2Salt;
    std::copy(data.begin() + ARGON2_SALT_OFFSET,
              data.begin() + ARGON2_SALT_OFFSET + ARGON2_SALT_SIZE, argon2Salt.begin());

    uint64_t argon2OpLimit = read_u64_le(data.subspan(ARGON2_OPSLIMIT_OFFSET, sizeof(uint64_t)));
    if (argon2OpLimit < crypto_pwhash_OPSLIMIT_MIN || argon2OpLimit > crypto_pwhash_OPSLIMIT_MAX) {
        throw VaultHeaderParsingError("Invalid Argon2 operation limit");
    }

    uint64_t argon2MemLimit = read_u64_le(data.subspan(ARGON2_MEMLIMIT_OFFSET, sizeof(uint64_t)));
    if (argon2MemLimit < crypto_pwhash_MEMLIMIT_MIN ||
        argon2MemLimit > crypto_pwhash_MEMLIMIT_MAX) {
        throw VaultHeaderParsingError("Invalid Argon2 memory limit");
    }

    return VaultHeader(magicBytes, formatVersion, argon2Salt, argon2OpLimit, argon2MemLimit);
}

/*
 * @brief Reads a 32-bit unsigned integer from a byte span in little-endian order.
 *
 * @param data A span of bytes from which to read the integer. Must be at least 4 bytes long.
 *
 * @return The 32-bit unsigned integer read from the byte span.
 *
 * @throws VaultHeaderParsingError if the provided data span is smaller than 4 bytes.
 */
uint32_t VaultHeader::read_u32_le(Bytes data) {
    if (data.size() < sizeof(uint32_t)) {
        throw VaultHeaderParsingError("Not enough data to read uint32_t");
    }
    uint32_t value = 0;
    for (size_t i = 0; i < sizeof(uint32_t); ++i) {
        value |= static_cast<uint32_t>(data[i]) << (8 * i);
    }
    return value;
}

/*
 * @brief Reads a 64-bit unsigned integer from a byte span in little-endian order.
 *
 * @param data A span of bytes from which to read the integer. Must be at least 8 bytes long.
 *
 * @return The 64-bit unsigned integer read from the byte span.
 *
 * @throws VaultHeaderParsingError if the provided data span is smaller than 8 bytes.
 */
uint64_t VaultHeader::read_u64_le(Bytes data) {
    // TODO: Implement this !
    // data is a std::span<const uint8_t>
    if (data.size() < sizeof(uint64_t)) {
        throw VaultHeaderParsingError("Not enough data to read uint64_t");
    }
    uint64_t value = 0;
    for (size_t i = 0; i < sizeof(uint64_t); ++i) {
        value |= static_cast<uint64_t>(data[i]) << (8 * i);
    }
    return value;
}
