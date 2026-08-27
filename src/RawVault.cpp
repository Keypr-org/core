#include "RawVault.h"
#include "VaultHeader.h"

RawVault RawVault::parse(Bytes data) {
    if (data.size() < VAULT_FILE_MIN_SIZE) {
        throw RawVaultParsingError("Data is too small to be a valid vault file.");
    }

    // Parse header
    // It must be initialized with a lambda as VaultHeader is not default-constructible
    // Help received from ChatGPT to figure this out
    VaultHeader header = [&] {
        try {
            return VaultHeader::parse(data);
        } catch (const VaultHeaderParsingError& e) {
            throw RawVaultParsingError("Failed to parse vault header: " + std::string(e.what()));
        }
    }();

    // Parse header MAC
    std::array<uint8_t, 32> headerMAC;
    std::copy(data.begin() + HEADER_MAC_OFFSET, data.begin() + HEADER_MAC_OFFSET + HEADER_MAC_BYTES,
              headerMAC.begin());

    // Parse XSalsa20 nonce
    std::array<uint8_t, 24> xSalsa20Nonce;
    std::copy(data.begin() + XSALSA20_NONCE_OFFSET,
              data.begin() + XSALSA20_NONCE_OFFSET + XSALSA20_NONCE_BYTES, xSalsa20Nonce.begin());

    // Parse ciphertext MAC
    std::array<uint8_t, 16> ciphertextMAC;
    std::copy(data.begin() + CIPHERTEXT_MAC_OFFSET,
              data.begin() + CIPHERTEXT_MAC_OFFSET + CIPHERTEXT_MAC_BYTES, ciphertextMAC.begin());

    // Parse ciphertext
    std::vector<uint8_t> ciphertext(data.begin() + CIPHERTEXT_OFFSET, data.end());

    return RawVault(header, headerMAC, xSalsa20Nonce, ciphertextMAC, ciphertext);
}
