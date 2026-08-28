#pragma once

#include "Types.h"
#include "entities/Category.h"
#include "entities/Entry.h"
#include "entities/Persona.h"
#include <memory>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

class VaultSession : public DatedItem {
  public:
    /**
     * Constructs a new VaultSession.
     * @param name The name of the VaultSession.
     * @param encKey The encryption key for the VaultSession.
     * @param authKey The authentication key for the VaultSession.
     */
    VaultSession(std::string name, EncKey encKey, AuthKey authKey);

    /**
     * Returns the name of the VaultSession.
     * @return The name of the VaultSession.
     */
    const std::string& getName() const noexcept;

    /**
     * Returns the categories in the VaultSession.
     * @return The categories in the VaultSession.
     */
    const std::vector<std::unique_ptr<Category>>& getCategories() const noexcept;

    /**
     * Adds a category to the VaultSession.
     * @param category The category to add.
     */
    void addCategory(std::unique_ptr<Category> category);

    /**
     * Returns the personas in the VaultSession.
     * @return The personas in the VaultSession.
     */
    const std::vector<std::shared_ptr<Persona>>& getPersonas() const noexcept;

    /**
     * Adds a persona to the VaultSession.
     * @param persona The persona to add.
     */
    void addPersona(std::shared_ptr<Persona> persona);

    /**
     * Removes a persona from the VaultSession.
     * @param personaId The ID of the persona to remove.
     */
    void removePersona(int64_t personaId);

    /**
     * Adds an entry to a category in the VaultSession.
     * @param categoryId The ID of the category to add the entry to.
     * @param entry The entry to add.
     * @throws CategoryNotFoundError if the category with the specified ID does not exist.
     */
    void addEntryToCategory(int64_t categoryId, std::unique_ptr<Entry> entry);

    std::string getType() const override { return "VaultSession"; }

    static VaultSession parse(Bytes vaultBody);

    friend void to_json(json& j, const VaultSession& vaultSession);
    friend void from_json(const json& j, VaultSession& vaultSession);

  private:
    VaultSession() = default;
    VaultSession(DateTime creationDate, DateTime lastModifiedDate, std::string name, EncKey encKey,
                 AuthKey authKey, std::vector<std::unique_ptr<Category>> categories,
                 std::vector<std::shared_ptr<Persona>> personas);

    EncKey encKey;
    AuthKey authKey;
    std::string name;
    std::vector<std::unique_ptr<Category>> categories;
    std::vector<std::shared_ptr<Persona>> personas;
};

class VaultSessionError : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

class CategoryNotFoundError : public VaultSessionError {
  public:
    using VaultSessionError::VaultSessionError;
};
