#include "VaultSession.h"
#include "entities/Website.h"
#include "entities/Wifi.h"
#include "entities/CreditCard.h"

VaultSession::VaultSession(std::string name, EncKey encKey, AuthKey authKey)
    : VaultSession(std::chrono::system_clock::now(), std::chrono::system_clock::now(), std::move(name), std::move(encKey), std::move(authKey), {}, {}) {
}

VaultSession::VaultSession(DateTime creationDate, DateTime lastModifiedDate, std::string name, EncKey encKey, AuthKey authKey, std::vector<std::unique_ptr<Category>> categories, std::vector<std::shared_ptr<Persona>> personas)
    : DatedItem(creationDate, lastModifiedDate), encKey(std::move(encKey)), authKey(std::move(authKey)), name(std::move(name)), categories(std::move(categories)), personas(std::move(personas)) {
}

const std::string &VaultSession::getName() const noexcept {
    return name;
}

const std::vector<std::unique_ptr<Category>> &VaultSession::getCategories() const noexcept {
    return categories;
}

void VaultSession::addCategory(std::unique_ptr<Category> category) {
    categories.emplace_back(std::move(category));
    setLastModifiedDate(std::chrono::system_clock::now());
}

const std::vector<std::shared_ptr<Persona>> &VaultSession::getPersonas() const noexcept {
    return personas;
}

void VaultSession::addPersona(std::shared_ptr<Persona> persona) {
    personas.emplace_back(std::move(persona));
    setLastModifiedDate(std::chrono::system_clock::now());
}

void VaultSession::removePersona(int64_t personaId) {
    personas.erase(std::remove_if(personas.begin(), personas.end(), [personaId](const std::shared_ptr<Persona> &persona) {
        return persona->getId() == personaId;
        }), personas.end());
    setLastModifiedDate(std::chrono::system_clock::now());
}

void VaultSession::addEntryToCategory(int64_t categoryId, std::unique_ptr<Entry> entry) {

    std::unique_ptr<Category> &category = findCategoryById(categoryId);
    category->addEntry(std::move(entry));
    setLastModifiedDate(std::chrono::system_clock::now());
}

void VaultSession::removeEntryFromCategory(int64_t categoryId, int64_t entryId) {
    std::unique_ptr<Category> &category = findCategoryById(categoryId);
    if (!category->removeEntry(entryId)) {
        throw EntryNotFoundError("Entry with ID " + std::to_string(entryId) + " not found in category with ID " + std::to_string(categoryId) + ".");
    }
    setLastModifiedDate(std::chrono::system_clock::now());
}

std::unique_ptr<Category> &VaultSession::findCategoryById(int64_t categoryId) {
    auto it = std::find_if(categories.begin(), categories.end(), [categoryId](std::unique_ptr<Category> &category) {
        return category->getId() == categoryId;
        });

    if (it != categories.end()) {
        return *it;
    } else {
        throw CategoryNotFoundError("Category with ID " + std::to_string(categoryId) + " not found.");
    }
}

const std::unique_ptr<Category> &VaultSession::findCategoryById(int64_t categoryId) const {
    auto it = std::find_if(categories.begin(), categories.end(), [categoryId](const std::unique_ptr<Category> &category) {
        return category->getId() == categoryId;
        });

    if (it != categories.end()) {
        return *it;
    } else {
        throw CategoryNotFoundError("Category with ID " + std::to_string(categoryId) + " not found.");
    }
}

std::vector<const Entry *> VaultSession::searchEntriesInCategory(int64_t categoryId, const std::string &searchTerm) const {
    const std::unique_ptr<Category> &category = findCategoryById(categoryId);

    std::vector<const Entry *> matchingEntries;
    for (const auto &entry : category->getEntries()) {
        if (entry->getNotes().find(searchTerm) != std::string::npos) {
            matchingEntries.push_back(entry.get());
        } else if (const auto websiteEntry = dynamic_cast<const Website *>(entry.get())) {
            if (websiteEntry->getTitle().find(searchTerm) != std::string::npos ||
                websiteEntry->getUsername().find(searchTerm) != std::string::npos ||
                websiteEntry->getUrl().find(searchTerm) != std::string::npos ||
                websiteEntry->getComments().find(searchTerm) != std::string::npos ||
                websiteEntry->getAlias().find(searchTerm) != std::string::npos) {
                matchingEntries.push_back(entry.get());
            }
        } else if (const auto wifiEntry = dynamic_cast<const Wifi *>(entry.get())) {
            if (wifiEntry->getNetworkName().find(searchTerm) != std::string::npos) {
                matchingEntries.push_back(entry.get());
            }
        } else if (const auto cardEntry = dynamic_cast<const CreditCard *>(entry.get())) {
            if (cardEntry->getCardHolderName().find(searchTerm) != std::string::npos) {
                matchingEntries.push_back(entry.get());
            }
        }
    }

    return matchingEntries;
}
