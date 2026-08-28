#pragma once

#include "entities/Persona.h"
#include "entities/Category.h"
#include "Types.h"
#include <string>
#include <vector>
#include <memory>

class VaultSession : public DatedItem {
public:

    VaultSession(std::string name, EncKey encKey, AuthKey authKey);

    const std::string &getName() const noexcept;

    const std::vector<std::unique_ptr<Category>> &getCategories() const noexcept;
    void addCategory(std::unique_ptr<Category> category);

    const std::vector<std::shared_ptr<Persona>> &getPersonas() const noexcept;
    void addPersona(std::shared_ptr<Persona> persona);
    void removePersona(int64_t personaId);


private:
    VaultSession(DateTime creationDate, DateTime lastModifiedDate, std::string name, EncKey encKey, AuthKey authKey, std::vector<std::unique_ptr<Category>> categories, std::vector<std::shared_ptr<Persona>> personas);

    const EncKey encKey;
    const AuthKey authKey;
    std::string name;
    std::vector<std::unique_ptr<Category>> categories;
    std::vector<std::shared_ptr<Persona>> personas;
};
