#pragma once

#include "entities/Persona.h"
#include "entities/Category.h"
#include "Types.h"
#include <string>
#include <vector>

class VaultSession {
public:

    const EncKey &getEncKey() const noexcept;

    const AuthKey &getAuthKey() const noexcept;

    const std::string &getName() const noexcept;

    const std::vector<Category> &getCategories() const noexcept;
    void addCategory(const Category &category);

    const std::vector<Persona> &getPersonas() const noexcept;
    void addPersona(const Persona &persona);
    void removePersona(int64_t personaId);


private:
    VaultSession(int64_t id, std::string name, EncKey encKey, AuthKey authKey, std::vector<Category> categories, std::vector<Persona> personas);

    EncKey encKey;
    AuthKey authKey;
    std::string name;
    std::vector<Category> categories;
    std::vector<Persona> personas;
};
