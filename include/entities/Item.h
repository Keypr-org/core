#pragma once

#include <cstdint>

class Item {
public:
    explicit Item(int64_t id);
    virtual ~Item() = default;

    int64_t getId() const;

private:
    int64_t id;
};
