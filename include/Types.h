#pragma once

#include <array>
#include <chrono>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <sodium.h>
#include <span>

using Bytes = std::span<const uint8_t>;
using MutableBytes = std::span<uint8_t>;

// Crypto related types
// Encryption/Decryption key for libsodium's crypto_secretbox API
using EncKey = std::array<uint8_t, crypto_secretbox_KEYBYTES>;
// Authentication key for libsodium's crypto_auth API
using AuthKey = std::array<uint8_t, crypto_auth_KEYBYTES>;
// Authentication tag for libsodium's crypto_auth API
using AuthMAC = std::array<uint8_t, crypto_auth_BYTES>;
// Encryption/Decryption tag for libsodium's crypto_secretbox API
using EncMAC = std::array<uint8_t, crypto_secretbox_MACBYTES>;
// Nonce for libsodium's crypto_secretbox API
using EncNonce = std::array<uint8_t, crypto_secretbox_NONCEBYTES>;

using DateTime = std::chrono::system_clock::time_point;

inline std::int64_t toUnixMilliseconds(DateTime dateTime) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(dateTime.time_since_epoch())
        .count();
}

inline DateTime fromUnixMilliseconds(std::int64_t milliseconds) {
    return DateTime{std::chrono::milliseconds{milliseconds}};
}

using json = nlohmann::json;
