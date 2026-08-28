#pragma once

#include <cstdint>
#include "snowflake.h"

using snowflake_t = snowflake<1534832906275L>;

class Item {
public:
    Item();
    virtual ~Item() = default;

    int64_t getId() const;

private:
    int64_t getNextId() const;

    int64_t id;
    static snowflake_t snowflake;
};
