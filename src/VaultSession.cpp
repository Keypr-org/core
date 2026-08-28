#include "VaultSession.h"

VaultSession::VaultSession(std::string name, EncKey encKey, AuthKey authKey)
    : VaultSession(std::chrono::system_clock::now(), std::chrono::system_clock::now(), std::move(name), std::move(encKey), std::move(authKey), {}, {}) {
}

VaultSession::VaultSession(DateTime creationDate, DateTime lastModifiedDate, std::string name, EncKey encKey, AuthKey authKey, std::vector<Category> categories, std::vector<Persona> personas)
    : DatedItem(creationDate, lastModifiedDate), encKey(std::move(encKey)), authKey(std::move(authKey)), name(std::move(name)), categories(std::move(categories)), personas(std::move(personas)) {
}

const std::string &VaultSession::getName() const noexcept {
    return name;
}

const std::vector<Category> &VaultSession::getCategories() const noexcept {
    return categories;
}

void VaultSession::addCategory(const Category &category) {
    categories.push_back(category);
    setLastModifiedDate(std::chrono::system_clock::now());
}

const std::vector<Persona> &VaultSession::getPersonas() const noexcept {
    return personas;
}

void VaultSession::addPersona(const Persona &persona) {
    personas.push_back(persona);
    setLastModifiedDate(std::chrono::system_clock::now());
}

void VaultSession::removePersona(int64_t personaId) {
    personas.erase(std::remove_if(personas.begin(), personas.end(), [personaId](const Persona &persona) {
        return persona.getId() == personaId;
        }), personas.end());
    setLastModifiedDate(std::chrono::system_clock::now());
}

void VaultSession::addEntryToCategory(int64_t categoryId, const Entry &entry) {

    Category &category = findCategoryById(categoryId);
    category.addEntry(entry);
    setLastModifiedDate(std::chrono::system_clock::now());
}

void VaultSession::removeEntryFromCategory(int64_t categoryId, int64_t entryId) {
    Category &category = findCategoryById(categoryId);
    if (!category.removeEntry(entryId)) {
        throw EntryNotFoundError("Entry with ID " + std::to_string(entryId) + " not found in category with ID " + std::to_string(categoryId) + ".");
    }
    setLastModifiedDate(std::chrono::system_clock::now());
}

Category &VaultSession::findCategoryById(int64_t categoryId) {
    auto it = std::find_if(categories.begin(), categories.end(), [categoryId](const Category &category) {
        return category.getId() == categoryId;
        });

    if (it != categories.end()) {
        return *it;
    } else {
        throw CategoryNotFoundError("Category with ID " + std::to_string(categoryId) + " not found.");
    }
}
