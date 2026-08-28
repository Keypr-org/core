#include "VaultSession.h"

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
