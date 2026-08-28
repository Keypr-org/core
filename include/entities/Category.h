#pragma once
#include "Item.h"
#include "Entry.h"
#include <vector>
#include <string>
#include <memory>


class Category : public Item {
public:
    explicit Category(std::string name);

    const std::string &getName() const noexcept;
    const std::vector<std::unique_ptr<Entry>> &getEntries() const;
    void addEntry(std::unique_ptr<Entry> entry);
    void removeEntry(int64_t entryId);

private:
    std::string name;
    std::vector<std::unique_ptr<Entry>> entries;
};
