#pragma once
#include "Item.h"
#include "Entry.h"
#include <vector>
#include <string>


class Category : public Item {
public:
    explicit Category(std::string name);

    const std::string &getName() const noexcept;
    const std::vector<Entry> &getEntries() const;

    /**
     * Adds an entry to the category.
     * @param entry The entry to add.
     */
    void addEntry(const Entry &entry);

    /**
     * Removes an entry from the category by its ID.
     * @param entryId The ID of the entry to remove.
     */
    void removeEntry(int64_t entryId);

private:
    std::string name;
    std::vector<Entry> entries;
};
