#include "entities/DatedItem.h"

DatedItem::DatedItem(int64_t id, DateTime creationDate, DateTime lastModifiedDate)
    : Item(id), creationAt(creationDate), updatedAt(lastModifiedDate) {
}

DatedItem::DatedItem(int64_t id, DateTime creationDate)
    : Item(id), creationAt(creationDate), updatedAt(creationDate) {
}

DateTime DatedItem::getCreationDate() const noexcept {
    return creationAt;
}

DateTime DatedItem::getLastModifiedDate() const noexcept {
    return updatedAt;
}

void DatedItem::setLastModifiedDate(DateTime lastModifiedDate) noexcept {
    this->updatedAt = lastModifiedDate;
}
