#pragma once
#include "Item.h"
#include "Entry.h"
#include <vector>
#include <string>


class Category : public Item {
public:
    explicit Category(int64_t id, std::string name);

    const std::string &getName() const noexcept;
    const std::vector<Entry *> &getEntries() const;

private:
    std::string name;
    std::vector<Entry *> entries;
};
