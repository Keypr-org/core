#include "entities/Entry.h"

Entry::Entry(int64_t id, std::string notes)
    : DatedItem(id), notes(std::move(notes)) {
}

const std::string &Entry::getNotes() const noexcept {
    return notes;
}

void Entry::setNotes(std::string notes) {
    this->notes = std::move(notes);
    setLastModifiedDate(std::chrono::system_clock::now());
}
