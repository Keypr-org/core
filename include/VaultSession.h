/*
 * @brief Represents a vault session that contains decrypted vault data, including categories,
 * entries, and personas.
 *
 * @author Nolan Evard
 * @author Maikol Correia Da Silva
 *
 * @date 31.08.2026
 */
#pragma once

#include "Types.h"
#include "VaultHeader.h"
#include "entities/Category.h"
#include "entities/Entry.h"
#include "entities/Persona.h"
#include "entities/Website.h"
#include <memory>
#include <nlohmann/json.hpp>
#include <stdexcept>
#include <string>
#include <vector>

class VaultSession : public DatedItem {
    // Must be a friend class so it can set and retrieve the auth and enc keys
    friend class VaultRepository;

  public:
    /**
     * Constructs a new VaultSession.
     * @param name The name of the VaultSession.
     * @param encKey The encryption key for the VaultSession.
     * @param authKey The authentication key for the VaultSession.
     */
    VaultSession(std::string name, EncKey encKey, AuthKey authKey,
                 std::unique_ptr<VaultHeader> header);

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
    const std::vector<std::unique_ptr<Persona>>& getPersonas() const noexcept;

    /**
     * Adds a persona to the VaultSession.
     * @param persona The persona to add.
     */
    void addPersona(std::unique_ptr<Persona> persona);

    /**
     * Removes a persona from the VaultSession.
     * @param personaId The ID of the persona to remove.
     * @throws PersonaNotFoundError if the persona with the specified ID does not exist.
     */
    void removePersona(int64_t personaId);

    /**
     * Links a persona to an entry in the VaultSession.
     * @param personaId The ID of the persona to link.
     * @param categoryId The ID of the category containing the entry to link the persona to.
     * @param entryId The ID of the entry to link the persona to.
     */
    void linkPersonaToEntry(int64_t personaId, int64_t categoryId, int64_t entryId);

    /**
     * Returns a persona by its ID.
     * @param personaId The ID of the persona to find.
     * @return A reference to the found persona.
     * @throws PersonaNotFoundError if the persona with the specified ID does not exist.
     */
    const std::unique_ptr<Persona>& getPersonaById(int64_t personaId) const;

    /**
     * Adds an entry to a category in the VaultSession.
     * @param categoryId The ID of the category to add the entry to.
     * @param entry The entry to add.
     * @throws CategoryNotFoundError if the category with the specified ID does not exist.
     */
    void addEntryToCategory(int64_t categoryId, std::unique_ptr<Entry> entry);

    /*
     * @brief Returns the type of the VaultSession.
     *
     * @return A string representing the type of the VaultSession.
     */
    std::string getType() const override { return "VaultSession"; }

    /**
     * Returns all websites in the VaultSession with the specified URL.
     * @param url The URL to search for.
     * @return A vector of pointers to the matching websites.
     */
    std::vector<const Website*> getWebsiteByUrl(const std::string& url) const;

    /**
     * Returns a website by its ID.
     * @param entryId The ID of the website to find.
     * @return A pointer to the website, or nullptr if not found.
     */
    const Website* getWebsiteById(int64_t entryId) const;

    /**
     * Removes an entry from a category in the VaultSession.
     * @param categoryId The ID of the category from which to remove the entry.
     * @param entryId The ID of the entry to remove.
     * @throws CategoryNotFoundError if the category with the specified ID does not exist.
     * @throws EntryNotFoundError if the entry with the specified ID does not exist in the category.
     */
    void removeEntryFromCategory(int64_t categoryId, int64_t entryId);

    /**
     * Searches for entries in a category that match the given search term.
     * @param categoryId The ID of the category to search in.
     * @param searchTerm The term to search for in the entries.
     * @return A vector of pointers to the matching entries.
     * @throws CategoryNotFoundError if the category with the specified ID does not exist.
     */
    std::vector<const Entry*> searchEntriesInCategory(int64_t categoryId,
                                                      const std::string& searchTerm) const;

    /*
     * @brief Parses a VaultSession from a JSON representation.
     *
     * @param vaultBody The JSON representation of the VaultSession.
     *
     * @return A VaultSession object parsed from the JSON representation.
     */
    static VaultSession parse(Bytes vaultBody);

    /*
     * @brief Serializes a VaultSession to a vector of bytes.
     *
     * @param session The VaultSession to serialize.
     *
     * @return A vector of bytes representing the serialized VaultSession.
     *
     * @throws SerializeError if serialization fails.
     */
    static std::vector<uint8_t> serialize(const VaultSession& session);

    friend void to_json(json& j, const VaultSession& vaultSession);
    friend void from_json(const json& j, VaultSession& vaultSession);

  private:
    // Default constructor for VaultSession. Needed for JSON deserialization.
    VaultSession() = default;

    /**
     * Finds a category by its ID.
     * @param categoryId The ID of the category to find.
     * @return A reference to the found category.
     * @throws CategoryNotFoundError if the category with the specified ID does not exist.
     */
    std::unique_ptr<Category>& findCategoryById(int64_t categoryId);

    /**
     * Finds a category by its ID (const version).
     * @param categoryId The ID of the category to find.
     * @return A reference to the found category.
     * @throws CategoryNotFoundError if the category with the specified ID does not exist.
     */
    const std::unique_ptr<Category>& findCategoryById(int64_t categoryId) const;

    EncKey encKey;
    AuthKey authKey;
    std::unique_ptr<VaultHeader> header;
    std::string name;
    std::vector<std::unique_ptr<Category>> categories;
    std::vector<std::unique_ptr<Persona>> personas;
};

// ----------------------------- Custom exceptions -----------------------------

class VaultSessionError : public std::runtime_error {
  public:
    using std::runtime_error::runtime_error;
};

class CategoryNotFoundError : public VaultSessionError {
  public:
    using VaultSessionError::VaultSessionError;
};

class PersonaNotFoundError : public VaultSessionError {
  public:
    using VaultSessionError::VaultSessionError;
};

class ParseError : public VaultSessionError {
  public:
    using VaultSessionError::VaultSessionError;
};

class SerializeError : public VaultSessionError {
  public:
    using VaultSessionError::VaultSessionError;
};

class EntryNotFoundError : public VaultSessionError {
  public:
    using VaultSessionError::VaultSessionError;
};

class EntryNotGoodTypeError : public VaultSessionError {
  public:
    using VaultSessionError::VaultSessionError;
};
