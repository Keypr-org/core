#pragma once
#include "Entry.h"
#include "Item.h"
#include "Types.h"
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

class Category : public Item {
  public:
    explicit Category(std::string name);
    Category() = default;

    const std::string& getName() const noexcept;

    const std::vector<std::unique_ptr<Entry>>& getEntries() const;

    /**
     * Adds an entry to the category.
     * @param entry The entry to add.
     */
    void addEntry(std::unique_ptr<Entry> entry);

    /**
     * Removes an entry from the category by its ID.
     * @param entryId The ID of the entry to remove.
     * @return true if the entry was found and removed, false otherwise.
     */
    bool removeEntry(int64_t entryId);

    std::string getType() const override { return "Category"; }

    friend void to_json(json& j, const Category& category);
    friend void from_json(const json& j, Category& category);

  private:
    std::string name;
    std::vector<std::unique_ptr<Entry>> entries;
};
