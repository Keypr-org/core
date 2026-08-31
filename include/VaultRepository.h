#pragma once

#include <string>

class VaultRepository {
  public:
    VaultRepository() = default;

    /*
     * @brief Checks if a vault file exists and is valid.
     *
     * @param filename The name of the vault file to check.
     *
     * @return true if the vault file exists and is valid, false otherwise.
     */
    bool vaultExists(const std::string& filename);
};
