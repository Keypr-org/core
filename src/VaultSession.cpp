/*
 * @brief VaultSession class implementation
 *
 * This file contains the implementation of the VaultSession class, which represents a session in a
 * vault application. The class provides methods to manage categories, personas, and entries within
 * the session. It also includes serialization and deserialization methods for JSON representation.
 *
 * @author Nolan Evard
 * @author Maikol Correia Da Silva
 *
 * @date 31.08.2026
 */
#include "VaultSession.h"

VaultSession::VaultSession(std::string name, EncKey encKey, AuthKey authKey)
    : DatedItem(std::chrono::system_clock::now(), std::chrono::system_clock::now()),
      name(std::move(name)), encKey(std::move(encKey)), authKey(std::move(authKey)) {}

const std::string& VaultSession::getName() const noexcept {
    return name;
}

const std::vector<std::unique_ptr<Category>>& VaultSession::getCategories() const noexcept {
    return categories;
}

void VaultSession::addCategory(std::unique_ptr<Category> category) {
    categories.emplace_back(std::move(category));
    setLastModifiedDate(std::chrono::system_clock::now());
}

const std::vector<std::unique_ptr<Persona>>& VaultSession::getPersonas() const noexcept {
    return personas;
}

void VaultSession::addPersona(std::unique_ptr<Persona> persona) {
    personas.emplace_back(std::move(persona));
    setLastModifiedDate(std::chrono::system_clock::now());
}

void VaultSession::removePersona(int64_t personaId) {
    personas.erase(std::remove_if(personas.begin(), personas.end(),
                                  [personaId](const std::unique_ptr<Persona>& persona) {
                                      return persona->getId() == personaId;
                                  }),
                   personas.end());
    setLastModifiedDate(std::chrono::system_clock::now());
}

void VaultSession::addEntryToCategory(int64_t categoryId, std::unique_ptr<Entry> entry) {
    auto it = std::find_if(categories.begin(), categories.end(),
                           [categoryId](std::unique_ptr<Category>& category) {
                               return category->getId() == categoryId;
                           });

    if (it != categories.end()) {
        (*it)->addEntry(std::move(entry));
        setLastModifiedDate(std::chrono::system_clock::now());
    } else {
        throw CategoryNotFoundError("Category with ID " + std::to_string(categoryId) +
                                    " not found.");
    }
}

void to_json(json& j, const VaultSession& vaultSession) {
    vaultSession.serializeDatedItem(j);

    j["name"] = vaultSession.name;
    j["categories"] = json::array();
    for (const auto& category : vaultSession.categories) {
        j["categories"].push_back(*category);
    }
    j["personas"] = json::array();
    for (const auto& persona : vaultSession.personas) {
        j["personas"].push_back(*persona);
    }
}
void from_json(const json& j, VaultSession& vaultSession) {
    vaultSession.parseDatedItem(j);

    // Enc key and auth key attributes are not meant to be parsed to JSON for obvious security
    // reasons

    vaultSession.name = j.at("name").get<std::string>();
    vaultSession.categories.clear();
    for (const auto& categoryJson : j.at("categories")) {
        vaultSession.categories.push_back(std::make_unique<Category>(categoryJson.get<Category>()));
    }
    vaultSession.personas.clear();
    for (const auto& personaJson : j.at("personas")) {
        vaultSession.personas.push_back(std::make_unique<Persona>(personaJson.get<Persona>()));
    }
}

VaultSession VaultSession::parse(Bytes vaultBody) {

    VaultSession result;
    try {
        const json j = json::parse(vaultBody.begin(), vaultBody.end());

        from_json(j, result);
    } catch (const json::exception& e) {
        throw ParseError("Failed to parse VaultSession JSON: " + std::string(e.what()));
    }

    return result;
}
