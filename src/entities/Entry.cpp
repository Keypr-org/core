#include "entities/Entry.h"
#include "entities/CreditCard.h"
#include "entities/Website.h"
#include "entities/Wifi.h"

#include <memory>

Entry::Entry(std::string notes) : DatedItem(), notes(std::move(notes)) {}

const std::string& Entry::getNotes() const noexcept {
    return notes;
}

void Entry::setNotes(std::string notes) {
    this->notes = std::move(notes);
    setLastModifiedDate(std::chrono::system_clock::now());
}

void Entry::parseEntry(const json& j) {
    parseDatedItem(j);
    j.at("notes").get_to(notes);
}

void Entry::serializeEntry(json& j) const {
    serializeDatedItem(j);
    j["notes"] = notes;
}

std::unique_ptr<Entry> Entry::parse(const json& input) {
    if (input.is_null()) {
        return nullptr;
    }

    const std::string type = input.at("type").get<std::string>();

    json payload = input;
    payload.erase("type");

    if (type == "Website") {
        std::unique_ptr<Website> result(new Website());
        payload.get_to(*result);
        return result;
    }

    if (type == "CreditCard") {
        std::unique_ptr<CreditCard> result(new CreditCard());
        payload.get_to(*result);
        return result;
    }

    if (type == "Wifi") {
        std::unique_ptr<Wifi> result(new Wifi());
        payload.get_to(*result);
        return result;
    }

    throw std::runtime_error("Unknown Entry type: " + type);
}
json Entry::serialize(const Entry& entry) {
    json j;

    j["type"] = entry.getType();

    if (entry.getType() == "Website") {
        const auto& website = static_cast<const Website&>(entry);
        j.update(json(website));
    } else if (entry.getType() == "CreditCard") {
        const auto& card = static_cast<const CreditCard&>(entry);
        j.update(json(card));
    } else if (entry.getType() == "Wifi") {
        const auto& wifi = static_cast<const Wifi&>(entry);
        j.update(json(wifi));
    } else {
        throw std::runtime_error("Unknown Entry type: " + entry.getType());
    }

    return j;
}
