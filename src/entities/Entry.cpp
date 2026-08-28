#include "entities/Entry.h"

Entry::Entry(std::string notes)
    : DatedItem(), notes(std::move(notes)) {
}

const std::string &Entry::getNotes() const noexcept {
    return notes;
}

void Entry::setNotes(std::string notes) {
    this->notes = std::move(notes);
    setLastModifiedDate(std::chrono::system_clock::now());
}
