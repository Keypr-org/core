#include "VaultSession.h"

VaultSession::VaultSession(int64_t id, std::string name, EncKey encKey, AuthKey authKey, std::vector<Category> categories, std::vector<Persona> personas)
    : encKey(std::move(encKey)), authKey(std::move(authKey)), name(std::move(name)), categories(std::move(categories)), personas(std::move(personas)) {
}

const EncKey &VaultSession::getEncKey() const noexcept {
    return encKey;
}

const AuthKey &VaultSession::getAuthKey() const noexcept {
    return authKey;
}

const std::string &VaultSession::getName() const noexcept {
    return name;
}

const std::vector<Category> &VaultSession::getCategories() const noexcept {
    return categories;
}

void VaultSession::addCategory(const Category &category) {
    categories.push_back(category);
}

const std::vector<Persona> &VaultSession::getPersonas() const noexcept {
    return personas;
}

void VaultSession::addPersona(const Persona &persona) {
    personas.push_back(persona);
}

void VaultSession::removePersona(int64_t personaId) {
    personas.erase(std::remove_if(personas.begin(), personas.end(), [personaId](const Persona &persona) {
        return persona.getId() == personaId;
        }), personas.end());
}
