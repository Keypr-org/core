#include "entities/Category.h"

Category::Category(std::string name) : Item(), name(std::move(name)) {}

Category::~Category() {
    entries.clear();
}

const std::string &Category::getName() const noexcept {
    return name;
}

const std::vector<std::unique_ptr<Entry>> &Category::getEntries() const {
    return entries;
}

void Category::addEntry(std::unique_ptr<Entry> entry) {
    entries.emplace_back(std::move(entry));
}

void Category::removeEntry(int64_t entryId) {
    entries.erase(std::remove_if(entries.begin(), entries.end(), [entryId](const std::unique_ptr<Entry> &entry) {
        return entry->getId() == entryId;
        }), entries.end());
}
