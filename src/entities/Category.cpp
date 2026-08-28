#include "entities/Category.h"

Category::Category(std::string name) : Item(), name(std::move(name)) {}

const std::string &Category::getName() const noexcept {
    return name;
}

const std::vector<Entry> &Category::getEntries() const {
    return entries;
}

void Category::addEntry(const Entry &entry) {
    entries.push_back(entry);
}

bool Category::removeEntry(int64_t entryId) {
    auto it = std::remove_if(entries.begin(), entries.end(), [entryId](const Entry &entry) {
        return entry.getId() == entryId;
        });
    if (it != entries.end()) {
        entries.erase(it, entries.end());
        return true;
    }
    return false;
}
