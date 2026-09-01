#include "VaultHeader.h"
#include <sodium.h>

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

uint64_t VaultHeader::read_u64_le(Bytes data) {
    if (data.size() < sizeof(uint64_t)) {
        throw VaultHeaderParsingError("Not enough data to read uint64_t");
    }
    uint64_t value = 0;
    for (size_t i = 0; i < sizeof(uint64_t); ++i) {
        value |= static_cast<uint64_t>(data[i]) << (8 * i);
    }
    return value;
}

void VaultHeader::write_u32_le(uint32_t value, MutableBytes data) {
    if (data.size() < sizeof(uint32_t)) {
        throw VaultHeaderSerializeError("Not enough space to write uint32_t");
    }
    for (size_t i = 0; i < sizeof(uint32_t); ++i) {
        data[i] = static_cast<uint8_t>((value >> (8 * i)) & 0xFF);
    }
}

void VaultHeader::write_u64_le(uint64_t value, MutableBytes data) {
    if (data.size() < sizeof(uint64_t)) {
        throw VaultHeaderSerializeError("Not enough space to write uint64_t");
    }
    for (size_t i = 0; i < sizeof(uint64_t); ++i) {
        data[i] = static_cast<uint8_t>((value >> (8 * i)) & 0xFF);
    }
}

std::vector<uint8_t> VaultHeader::serialize(const VaultHeader& header) {
    std::vector<uint8_t> out(VAULT_HEADER_BYTES);

    std::copy(header.magicBytes().begin(), header.magicBytes().end(),
              out.begin() + MAGIC_BYTES_OFFSET);

    write_u32_le(header.formatVersion(), {out.begin() + FORMAT_VERSION_OFFSET, sizeof(uint32_t)});

    std::copy(header.argon2Salt().begin(), header.argon2Salt().end(),
              out.begin() + ARGON2_SALT_OFFSET);

    write_u64_le(header.argon2OpLimit(), {out.begin() + ARGON2_OPSLIMIT_OFFSET, sizeof(uint64_t)});

    write_u64_le(header.argon2MemLimit(), {out.begin() + ARGON2_MEMLIMIT_OFFSET, sizeof(uint64_t)});

    return out;
}
