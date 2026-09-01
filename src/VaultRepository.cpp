#include "VaultRepository.h"
#include "CryptoService.h"
#include "FileHandler.h"
#include "RawVault.h"
#include "Types.h"

#include <sodium.h>

// The derived key length is the sum of the encryption key size and the authentication key size
// because it will be split into two parts: one for encryption and one for authentication.
constexpr uint64_t KeyDerivationLength = EncKey{}.size() + AuthKey{}.size();

bool VaultRepository::vaultExists(const std::string& filename) const {
    if (!FileHandler::fileExists(filename))
        return false;
    try {
        RawVault::parse(FileHandler::readFile(filename));
    } catch (RawVaultParsingError& e) {
        return false;
    } catch (FileHandlerError& e) {
        return false;
    }
    return true;
}

std::unique_ptr<VaultSession> VaultRepository::unlockVault(const std::string& masterpass,
                                                           const std::string& filename) const {
    try {
        std::vector<uint8_t> fileContents = FileHandler::readFile(filename);
        RawVault rawVault = RawVault::parse(fileContents);
        std::vector<uint8_t> derivedKey = CryptoService::deriveKey(
            masterpass, KeyDerivationLength, rawVault.header().argon2Salt(),
            rawVault.header().argon2OpLimit(), rawVault.header().argon2MemLimit());

        // Split the derived key into encryption and authentication keys
        EncKey encKey;
        AuthKey authKey;
        std::copy(derivedKey.begin(), derivedKey.begin() + encKey.size(), encKey.begin());
        std::copy(derivedKey.begin() + encKey.size(), derivedKey.end(), authKey.begin());

        // Verify the header MAC
        auto headerSpan = std::span<const uint8_t>(fileContents.data(), VAULT_HEADER_BYTES);
        if (!CryptoService::verify(authKey, rawVault.headerMAC(), headerSpan)) {
            // If the header MAC verification fails, it is most likely due to an incorrect master
            // password. We don't want to throw an exception each time the user enters a wrong
            // password. Instead, we return nullptr to indicate that the vault could not be
            // unlocked.
            return nullptr;
        }

        // Decrypt the vault body
        std::vector<uint8_t> decryptedBody = CryptoService::decrypt(
            encKey, rawVault.xSalsa20Nonce(), rawVault.ciphertextMAC(), rawVault.ciphertext());

        // Parse the decrypted vault body into a VaultSession
        VaultSession vaultSession = VaultSession::parse(decryptedBody);

        // Set encryption and authentication keys in the VaultSession
        vaultSession.encKey = encKey;
        vaultSession.authKey = authKey;
        vaultSession.header = std::make_unique<VaultHeader>(rawVault.header());

        return std::make_unique<VaultSession>(std::move(vaultSession));

    } catch (FileHandlerError& e) {
        throw UnlockVaultError("Failed to read vault file: " + std::string(e.what()));
    } catch (RawVaultError& e) {
        throw UnlockVaultError("Failed to parse vault file: " + std::string(e.what()));
    } catch (KeyDerivationError& e) {
        throw UnlockVaultError("Failed to derive keys: " + std::string(e.what()));
    } catch (DecryptionError& e) {
        // Failing to decrypt is most likely due to an incorrect master password, so we don't want
        // to throw an exception each time the user enters a wrong password. Instead, we return
        // nullptr to indicate that the vault could not be unlocked.
        return nullptr;
    } catch (ParseError& e) {
        throw UnlockVaultError("Failed to parse decrypted vault body: " + std::string(e.what()));
    }
}

std::unique_ptr<VaultSession> VaultRepository::createVault(const std::string& masterpass,
                                                           const std::string& vaultName) const {
    try {
        // Generate salt
        std::array<uint8_t, crypto_pwhash_SALTBYTES> argon2Salt;
        randombytes_buf(argon2Salt.data(), argon2Salt.size());

        // Create header
        std::array<uint8_t, MAGIC_BYTES_SIZE> magicBytes;
        for (size_t i = 0; i < MAGIC_BYTES_SIZE; ++i) {
            magicBytes[i] = VAULT_MAGIC_BYTES[i];
        }
        VaultHeader header(magicBytes, VAULT_FORMAT_CURRENT_VERSION, argon2Salt,
                           crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE);

        std::vector<uint8_t> derivedKey =
            CryptoService::deriveKey(masterpass, KeyDerivationLength, header.argon2Salt(),
                                     header.argon2OpLimit(), header.argon2MemLimit());

        // Split the derived key into encryption and authentication keys
        EncKey encKey;
        AuthKey authKey;
        std::copy(derivedKey.begin(), derivedKey.begin() + encKey.size(), encKey.begin());
        std::copy(derivedKey.begin() + encKey.size(), derivedKey.end(), authKey.begin());

        return std::make_unique<VaultSession>(vaultName, encKey, authKey,
                                              std::make_unique<VaultHeader>(header));
    } catch (KeyDerivationError& e) {
        throw CreateVaultError("Failed to derive keys: " + std::string(e.what()));
    }
}

bool VaultRepository::lockVault(const VaultSession& session, const std::string& filename) const {
    try { // Serialize session to JSON
        std::vector<uint8_t> vaultBodyPlaintext = VaultSession::serialize(session);

        // Authenticate header
        std::array<uint8_t, VAULT_HEADER_BYTES> headerBytes =
            VaultHeader::serialize(*session.header);
        AuthMAC headerMAC = CryptoService::authenticate(session.authKey, headerBytes);

        // Generate random Nonce
        std::array<uint8_t, XSALSA20_NONCE_BYTES> xSalsa20Nonce;
        randombytes_buf(xSalsa20Nonce.data(), xSalsa20Nonce.size());

        // Encrypt vault body
        auto [ciphertextMAC, ciphertext] =
            CryptoService::encrypt(session.encKey, xSalsa20Nonce, vaultBodyPlaintext);

        // Create RawVault
        RawVault rawVault(*session.header, headerMAC, xSalsa20Nonce, ciphertextMAC, ciphertext);

        // Serialize RawVault to bytes
        std::vector<uint8_t> rawVaultBytes = RawVault::serialize(rawVault);

        FileHandler::saveFileAtomically(filename, rawVaultBytes);

        return true;

    } catch (std::runtime_error& e) {
        return false;
    }
}
