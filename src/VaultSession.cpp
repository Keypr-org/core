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
#include "entities/CreditCard.h"
#include "entities/Website.h"
#include "entities/Wifi.h"

VaultSession::VaultSession(std::string name, EncKey encKey, AuthKey authKey,
                           std::unique_ptr<VaultHeader> header)
    : DatedItem(std::chrono::system_clock::now(), std::chrono::system_clock::now()),
      name(std::move(name)), encKey(std::move(encKey)), authKey(std::move(authKey)),
      header(std::move(header)) {}

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
    getPersonaById(personaId); // Check if persona exists, throws PersonaNotFoundError if not
    personas.erase(std::remove_if(personas.begin(), personas.end(),
                                  [personaId](const std::unique_ptr<Persona>& persona) {
                                      return persona->getId() == personaId;
                                  }),
                   personas.end());
    setLastModifiedDate(std::chrono::system_clock::now());

    for (const auto& category : categories) {
        for (const auto& entry : category->getEntries()) {
            if (auto websiteEntry = dynamic_cast<Website*>(entry.get())) {
                if (websiteEntry->getPersonaId() == personaId) {
                    websiteEntry->setPersona(-1); // Unlink the persona from the entry
                }
            }
        }
    }
}

void VaultSession::linkPersonaToEntry(int64_t personaId, int64_t categoryId, int64_t entryId) {
    getPersonaById(personaId); // Check if persona exists, throws PersonaNotFoundError if not

    std::unique_ptr<Category>& category = findCategoryById(categoryId);
    auto entry = category->findEntryById(entryId);
    if (entry == nullptr) {
        throw EntryNotFoundError("Entry with ID " + std::to_string(entryId) +
                                 " not found in category with ID " + std::to_string(categoryId) +
                                 ".");
    }
    if (auto websiteEntry = dynamic_cast<Website*>(entry)) {
        websiteEntry->setPersona(personaId);
    } else {
        throw EntryNotGoodTypeError("Entry with ID " + std::to_string(entryId) +
                                    " is not a Website entry and cannot be linked to a persona.");
    }
}

const std::unique_ptr<Persona>& VaultSession::getPersonaById(int64_t personaId) const {
    auto it = std::find_if(personas.begin(), personas.end(),
                           [personaId](const std::unique_ptr<Persona>& persona) {
                               return persona->getId() == personaId;
                           });

    if (it != personas.end()) {
        return *it;
    } else {
        throw PersonaNotFoundError("Persona with ID " + std::to_string(personaId) + " not found.");
    }
}

void VaultSession::addEntryToCategory(int64_t categoryId, std::unique_ptr<Entry> entry) {

    std::unique_ptr<Category>& category = findCategoryById(categoryId);
    category->addEntry(std::move(entry));
    setLastModifiedDate(std::chrono::system_clock::now());
}

void VaultSession::removeEntryFromCategory(int64_t categoryId, int64_t entryId) {
    std::unique_ptr<Category>& category = findCategoryById(categoryId);
    if (!category->removeEntry(entryId)) {
        throw EntryNotFoundError("Entry with ID " + std::to_string(entryId) +
                                 " not found in category with ID " + std::to_string(categoryId) +
                                 ".");
    }
    setLastModifiedDate(std::chrono::system_clock::now());
}

std::unique_ptr<Category>& VaultSession::findCategoryById(int64_t categoryId) {
    auto it = std::find_if(categories.begin(), categories.end(),
                           [categoryId](std::unique_ptr<Category>& category) {
                               return category->getId() == categoryId;
                           });

    if (it != categories.end()) {
        return *it;
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

const Website* VaultSession::getWebsiteById(int64_t entryId) const {
    for (const auto& category : categories) {
        for (const auto& entry : category->getEntries()) {
            const auto* website = dynamic_cast<const Website*>(entry.get());
            if (website && website->getId() == entryId) {
                return website;
            }
        }
    }
    return nullptr;
}

void VaultSession::setAliasForWebsite(int64_t categoryId, int64_t entryId,
                                      const std::string& aliasId, const std::string& alias) {
    std::unique_ptr<Category>& category = findCategoryById(categoryId);
    auto entry = category->findEntryById(entryId);
    if (entry == nullptr) {
        throw EntryNotFoundError("Entry with ID " + std::to_string(entryId) +
                                 " not found in category with ID " + std::to_string(categoryId) +
                                 ".");
    }
    if (auto websiteEntry = dynamic_cast<Website*>(entry)) {
        websiteEntry->setAlias(alias);
        websiteEntry->setAliasId(aliasId);
    } else {
        throw EntryNotGoodTypeError("Entry with ID " + std::to_string(entryId) +
                                    " is not a Website entry and cannot have an alias set.");
    }
}

const std::unique_ptr<Category>& VaultSession::findCategoryById(int64_t categoryId) const {
    auto it = std::find_if(categories.begin(), categories.end(),
                           [categoryId](const std::unique_ptr<Category>& category) {
                               return category->getId() == categoryId;
                           });

    if (it != categories.end()) {
        return *it;
    } else {
        throw CategoryNotFoundError("Category with ID " + std::to_string(categoryId) +
                                    " not found.");
    }
}

std::vector<const Entry*>
VaultSession::searchEntriesInCategory(int64_t categoryId, const std::string& searchTerm) const {
    const std::unique_ptr<Category>& category = findCategoryById(categoryId);

    std::vector<const Entry*> matchingEntries;
    for (const auto& entry : category->getEntries()) {
        if (entry->getNotes().find(searchTerm) != std::string::npos) {
            matchingEntries.push_back(entry.get());
        } else if (const auto websiteEntry = dynamic_cast<const Website*>(entry.get())) {
            if (websiteEntry->getTitle().find(searchTerm) != std::string::npos ||
                websiteEntry->getUsername().find(searchTerm) != std::string::npos ||
                websiteEntry->getUrl().find(searchTerm) != std::string::npos ||
                websiteEntry->getComments().find(searchTerm) != std::string::npos ||
                websiteEntry->getAlias().find(searchTerm) != std::string::npos) {
                matchingEntries.push_back(entry.get());
            }
        } else if (const auto wifiEntry = dynamic_cast<const Wifi*>(entry.get())) {
            if (wifiEntry->getNetworkName().find(searchTerm) != std::string::npos) {
                matchingEntries.push_back(entry.get());
            }
        } else if (const auto cardEntry = dynamic_cast<const CreditCard*>(entry.get())) {
            if (cardEntry->getCardHolderName().find(searchTerm) != std::string::npos) {
                matchingEntries.push_back(entry.get());
            }
        }
    }

    return matchingEntries;
}

std::vector<const Website*> VaultSession::getWebsiteByUrl(const std::string& url) const {
    std::vector<const Website*> matchingWebsites;

    for (const auto& category : categories) {
        for (const auto& entry : category->getEntries()) {
            const Website* website = dynamic_cast<const Website*>(entry.get());
            if (website && (url.find(website->getUrl()) != std::string::npos) ||
                (website->getUrl().find(url) != std::string::npos)) {
                matchingWebsites.push_back(website);
            }
        }
    }

    return matchingWebsites;
}

std::vector<uint8_t> VaultSession::serialize(const VaultSession& session) {
    try {
        json j;
        to_json(j, session);

        const std::string body = j.dump();

        return std::vector<uint8_t>(body.begin(), body.end());
    } catch (const json::exception& e) {
        throw SerializeError("Failed to serialize VaultSession to JSON: " + std::string(e.what()));
    }
}
