#pragma once

#include "entities/Entry.h"
#include "entities/Persona.h"
#include "entities/Category.h"
#include "Types.h"
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
    const std::string &getName() const noexcept;

    /**
     * Returns the categories in the VaultSession.
     * @return The categories in the VaultSession.
     */
    const std::vector<Category> &getCategories() const noexcept;

    /**
     * Adds a category to the VaultSession.
     * @param category The category to add.
     */
    void addCategory(const Category &category);

    /**
     * Returns the personas in the VaultSession.
     * @return The personas in the VaultSession.
     */
    const std::vector<Persona> &getPersonas() const noexcept;

    /**
     * Adds a persona to the VaultSession.
     * @param persona The persona to add.
     */
    void addPersona(const Persona &persona);

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
    void addEntryToCategory(int64_t categoryId, const Entry &entry);


private:
    VaultSession(DateTime creationDate, DateTime lastModifiedDate, std::string name, EncKey encKey, AuthKey authKey, std::vector<Category> categories, std::vector<Persona> personas);

    const EncKey encKey;
    const AuthKey authKey;
    std::string name;
    std::vector<Category> categories;
    std::vector<Persona> personas;
};

class VaultSessionError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class CategoryNotFoundError : public VaultSessionError {
public:
    using VaultSessionError::VaultSessionError;
};
