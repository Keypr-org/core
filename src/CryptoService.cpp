#include "CryptoService.h"
#include <sodium.h>
#include <cstring>
#include <sstream>
#include <iomanip>


// Convertir bytes en hex string
static std::string bytesToHex(const unsigned char* bytes, size_t length) {
    std::stringstream ss;
    for (size_t i = 0; i < length; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)bytes[i];
    }
    return ss.str();
}

// Convertir hex string en bytes
static std::string hexToBytes(const std::string& hex) {
    std::string bytes;
    for (size_t i = 0; i < hex.length(); i += 2) {
        std::string byteString = hex.substr(i, 2);
        char byte = (char)(int)strtol(byteString.c_str(), nullptr, 16);
        bytes.push_back(byte);
    }
    return bytes;
}

CryptoService::CryptoService() {
    if (sodium_init() < 0) {
        throw std::runtime_error("Failed to initialize libsodium");
    }
}

std::string CryptoService::generateRandomBytes(size_t length) {
    unsigned char* buffer = new unsigned char[length];
    randombytes_buf(buffer, length);
    std::string result(reinterpret_cast<char*>(buffer), length);
    delete[] buffer;
    return result;
}

std::string CryptoService::hashPassword(const std::string& password) {
    unsigned char salt[crypto_pwhash_SALTBYTES];
    randombytes_buf(salt, sizeof(salt));

    unsigned char hash[crypto_pwhash_STRBYTES];

    if (crypto_pwhash(
            hash, sizeof(hash),
            password.c_str(), password.length(),
            salt,
            crypto_pwhash_OPSLIMIT_MODERATE,
            crypto_pwhash_MEMLIMIT_MODERATE,
            crypto_pwhash_ALG_DEFAULT) != 0) {
        throw std::runtime_error("Failed to hash password");
    }

    return std::string(reinterpret_cast<char*>(hash), sizeof(hash));
}

std::string CryptoService::encrypt(const std::string& plaintext, const std::string& key) {
    if (key.length() != crypto_secretbox_KEYBYTES) {
        throw std::runtime_error("Invalid key length. Expected " +
                                 std::to_string(crypto_secretbox_KEYBYTES) + " bytes");
    }

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    randombytes_buf(nonce, sizeof(nonce));

    unsigned char* ciphertext = new unsigned char[plaintext.length() + crypto_secretbox_MACBYTES];

    crypto_secretbox_easy(
        ciphertext,
        reinterpret_cast<const unsigned char*>(plaintext.c_str()),
        plaintext.length(),
        nonce,
        reinterpret_cast<const unsigned char*>(key.c_str())
        );

    // Retourner : nonce + ciphertext (en hex)
    std::string nonce_hex = bytesToHex(nonce, sizeof(nonce));
    std::string cipher_hex = bytesToHex(ciphertext, plaintext.length() + crypto_secretbox_MACBYTES);

    delete[] ciphertext;

    return nonce_hex + cipher_hex;
}

std::string CryptoService::decrypt(const std::string& ciphertext, const std::string& key) {
    if (key.length() != crypto_secretbox_KEYBYTES) {
        throw std::runtime_error("Invalid key length. Expected " +
                                 std::to_string(crypto_secretbox_KEYBYTES) + " bytes");
    }

    size_t nonce_hex_len = crypto_secretbox_NONCEBYTES * 2;
    if (ciphertext.length() < nonce_hex_len) {
        throw std::runtime_error("Ciphertext too short");
    }

    std::string nonce_hex = ciphertext.substr(0, nonce_hex_len);
    std::string cipher_hex = ciphertext.substr(nonce_hex_len);

    unsigned char nonce[crypto_secretbox_NONCEBYTES];
    std::string nonce_bytes = hexToBytes(nonce_hex);
    std::memcpy(nonce, nonce_bytes.c_str(), sizeof(nonce));

    std::string cipher_bytes = hexToBytes(cipher_hex);
    unsigned char* plaintext = new unsigned char[cipher_bytes.length() - crypto_secretbox_MACBYTES];

    if (crypto_secretbox_open_easy(
            plaintext,
            reinterpret_cast<const unsigned char*>(cipher_bytes.c_str()),
            cipher_bytes.length(),
            nonce,
            reinterpret_cast<const unsigned char*>(key.c_str())) != 0) {
        delete[] plaintext;
        throw std::runtime_error("Decryption failed - authentication check failed");
    }

    std::string result(reinterpret_cast<char*>(plaintext), cipher_bytes.length() - crypto_secretbox_MACBYTES);
    delete[] plaintext;

    return result;
}
