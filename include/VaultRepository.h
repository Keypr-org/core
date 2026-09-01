#pragma once

#include "VaultSession.h"
#include <string>

class VaultRepository {
  public:
    VaultRepository() = default;

    /*
     * @brief Checks if a vault file exists and is valid.
     *
     * Marked as virtual so it can be overridden in derived classes for testing purposes.
     *
     * @param filename The name of the vault file to check.
     *
     * @return true if the vault file exists and is valid, false otherwise.
     */
    virtual bool vaultExists(const std::string& filename) const;

    /*
     * @brief Unlocks a vault file using the provided master password and returns a VaultSession.
     *
     * @param masterpass The master password to unlock the vault.
     * @param filename The name of the vault file to unlock.
     *
     * @return A unique pointer to a VaultSession object representing the unlocked vault.
     *
     * @throws UnlockVaultError If the vault cannot be unlocked due to an invalid master password,
     *         corrupted vault file, or any other error during the unlocking process.
     */
    virtual std::unique_ptr<VaultSession> unlockVault(const std::string& masterpass,
                                                      const std::string& filename) const;

    /*
     * @brief Creates a new vault file with the provided master password and vault name, and returns
     * a VaultSession.
     *
     * @param masterpass The master password to secure the new vault.
     * @param vaultName The name of the new vault file to create.
     *
     * @return A unique pointer to a VaultSession object representing the newly created vault.
     *
     * @throws CreateVaultError If the vault cannot be created due to an existing vault with the
     * same name, insufficient permissions, or any other error during the creation process.
     */
    virtual std::unique_ptr<VaultSession> createVault(const std::string& masterpass,
                                                      const std::string& vaultName) const;

    /*
     * @brief Writes the current state of the vault session to disk as encrypted data. If a filename
     * is provided, it will be used; otherwise, the vault name will be used to derive a filename.
     *
     * @param session The VaultSession object representing the vault to lock.
     * @param filename Optional. The name of the file to write the locked vault to. If not provided,
     * a filename will be derived from the vault name.
     *
     * @return true if the vault was successfully locked and written to disk, false otherwise.
     */
    virtual bool lockVault(const VaultSession& session, const std::string& filename) const;
};

// ----------  Vault repository exceptions ---------------

class VaultRepositoryError : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

class UnlockVaultError : public VaultRepositoryError {
  public:
    using VaultRepositoryError::VaultRepositoryError;
};

class CreateVaultError : public VaultRepositoryError {
  public:
    using VaultRepositoryError::VaultRepositoryError;
};

class LockVaultError : public VaultRepositoryError {
  public:
    using VaultRepositoryError::VaultRepositoryError;
};
