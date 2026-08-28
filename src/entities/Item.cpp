#include "entities/Item.h"

Item::Item() {
    static bool initialized = false;

    if (!initialized) {
        snowflake.init(0, 0);
        initialized = true;
    }

    id = getNextId();
}

int64_t Item::getId() const {
    return id;
}

int64_t Item::getNextId() const {
    return snowflake.nextid();
}
