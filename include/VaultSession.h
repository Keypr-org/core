#pragma once

#include "entities/Persona.h"
#include "entities/Category.h"
#include "Types.h"
#include <string>
#include <vector>

class VaultSession : public DatedItem {
public:

    VaultSession(std::string name, EncKey encKey, AuthKey authKey);

    const std::string &getName() const noexcept;

    const std::vector<Category> &getCategories() const noexcept;
    void addCategory(const Category &category);

    const std::vector<Persona> &getPersonas() const noexcept;
    void addPersona(const Persona &persona);
    void removePersona(int64_t personaId);


private:
    VaultSession(DateTime creationDate, DateTime lastModifiedDate, std::string name, EncKey encKey, AuthKey authKey, std::vector<Category> categories, std::vector<Persona> personas);

    const EncKey encKey;
    const AuthKey authKey;
    std::string name;
    std::vector<Category> categories;
    std::vector<Persona> personas;
};
