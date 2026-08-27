#include "entities/Category.h"

Category::Category(int64_t id, std::string name) : Item(id), name(std::move(name)) {}

const std::string &Category::getName() const noexcept {
    return name;
}

const std::vector<Entry *> &Category::getEntries() const {
    return entries;
}

