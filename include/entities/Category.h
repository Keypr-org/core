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
    void addEntry(const Entry &entry);
    void removeEntry(int64_t entryId);

private:
    std::string name;
    std::vector<Entry> entries;
};
