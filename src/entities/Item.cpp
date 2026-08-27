#include "entities/Item.h"

Item::Item(int64_t id) : id(id) {}

int64_t Item::getId() const {
    return id;
}
