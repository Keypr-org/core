#pragma once

#include "DatedItem.h"
#include <string>

class Entry : public DatedItem {
public:
    explicit Entry(std::string notes = {});
    virtual ~Entry() = default;

    const std::string &getNotes() const noexcept;
    void setNotes(std::string notes);

private:
    std::string notes;
};
