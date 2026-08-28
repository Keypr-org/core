#include "entities/DatedItem.h"

DatedItem::DatedItem(DateTime creationDate, DateTime lastModifiedDate)
    : Item(), creationAt(creationDate), updatedAt(lastModifiedDate) {
}

DatedItem::DatedItem(DateTime creationDate)
    : Item(), creationAt(creationDate), updatedAt(creationDate) {
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
