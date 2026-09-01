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

    virtual bool lockVault(const VaultSession& session, std::string filename = "");
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
