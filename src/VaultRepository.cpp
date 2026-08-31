#include "VaultRepository.h"
#include "CryptoService.h"
#include "FileHandler.h"
#include "RawVault.h"
#include "Types.h"

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
            throw UnlockVaultError("Invalid master password or corrupted vault file");
        }

        // Decrypt the vault body
        std::vector<uint8_t> decryptedBody = CryptoService::decrypt(
            encKey, rawVault.xSalsa20Nonce(), rawVault.ciphertextMAC(), rawVault.ciphertext());

        // Parse the decrypted vault body into a VaultSession
        VaultSession vaultSession = VaultSession::parse(decryptedBody);

        // Set encryption and authentication keys in the VaultSession
        vaultSession.encKey = encKey;
        vaultSession.authKey = authKey;

        return std::make_unique<VaultSession>(std::move(vaultSession));

    } catch (FileHandlerError& e) {
        throw UnlockVaultError("Failed to read vault file: " + std::string(e.what()));
    } catch (RawVaultError& e) {
        throw UnlockVaultError("Failed to parse vault file: " + std::string(e.what()));
    } catch (KeyDerivationError& e) {
        throw UnlockVaultError("Failed to derive keys: " + std::string(e.what()));
    } catch (AuthenticationError& e) {
        throw UnlockVaultError("Failed to verify vault integrity: " + std::string(e.what()));
    } catch (DecryptionError& e) {
        throw UnlockVaultError("Failed to decrypt vault body: " + std::string(e.what()));
    } catch (ParseError& e) {
        throw UnlockVaultError("Failed to parse decrypted vault body: " + std::string(e.what()));
    }
}
