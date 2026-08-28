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
    // Call parent parseItem to get the id
    parseItem(j);

    const auto creationMs = j.at("creationAt").get<std::int64_t>();

    const auto updatedMs = j.at("updatedAt").get<std::int64_t>();

    creationAt = DateTime{std::chrono::milliseconds{creationMs}};

    updatedAt = DateTime{std::chrono::milliseconds{updatedMs}};
}

void DatedItem::serializeDatedItem(json& j) const {
    // Call parent serializeItem to set the id
    serializeItem(j);

    j["creationAt"] =
        duration_cast<std::chrono::milliseconds>(creationAt.time_since_epoch()).count();

    j["updatedAt"] = duration_cast<std::chrono::milliseconds>(updatedAt.time_since_epoch()).count();
}
