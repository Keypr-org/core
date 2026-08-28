#include "entities/Category.h"

Category::Category(std::string name) : Item(), name(std::move(name)) {}

const std::string& Category::getName() const noexcept {
    return name;
}

const std::vector<std::unique_ptr<Entry>>& Category::getEntries() const {
    return entries;
}

void Category::addEntry(std::unique_ptr<Entry> entry) {
    entries.emplace_back(std::move(entry));
}

void Category::removeEntry(int64_t entryId) {
    entries.erase(std::remove_if(entries.begin(), entries.end(),
                                 [entryId](const std::unique_ptr<Entry>& entry) {
                                     return entry->getId() == entryId;
                                 }),
                  entries.end());
}

void to_json(json& j, const Category& category) {
    category.serializeItem(j);

    // Add Category-specific fields
    j["name"] = category.name;
    j["entries"] = json::array();

    for (const auto& entry : category.entries) {
        j["entries"].push_back(entry ? Entry::serialize(*entry) : json(nullptr));
    }
}
void from_json(const json& j, Category& category) {
    category.parseItem(j);

    j.at("name").get_to(category.name);

    category.entries.clear();

    for (const auto& entryJson : j.at("entries")) {
        category.entries.push_back(Entry::parse(entryJson));
    }
}
