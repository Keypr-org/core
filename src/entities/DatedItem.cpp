#include "entities/DatedItem.h"

DatedItem::DatedItem(DateTime creationDate, DateTime lastModifiedDate)
    : Item(), creationAt(creationDate), updatedAt(lastModifiedDate) {}

DatedItem::DatedItem(DateTime creationDate)
    : Item(), creationAt(creationDate), updatedAt(creationDate) {}

DateTime DatedItem::getCreationDate() const noexcept {
    return creationAt;
}

DateTime DatedItem::getLastModifiedDate() const noexcept {
    return updatedAt;
}

void DatedItem::setLastModifiedDate(DateTime lastModifiedDate) noexcept {
    this->updatedAt = lastModifiedDate;
}

void DatedItem::parseDatedItem(const json& j) {
    parseItem(j);

    creationAt = fromUnixMilliseconds(j.at("creationAt").get<std::int64_t>());

    updatedAt = fromUnixMilliseconds(j.at("updatedAt").get<std::int64_t>());
}

void DatedItem::serializeDatedItem(json& j) const {
    serializeItem(j);

    j["creationAt"] = toUnixMilliseconds(creationAt);
    j["updatedAt"] = toUnixMilliseconds(updatedAt);
}
