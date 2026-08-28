#pragma once

#include "DatedItem.h"
#include <string>

class Entry : public DatedItem {
public:
    virtual ~Entry() = default;

    const std::string &getNotes() const noexcept;
    void setNotes(std::string notes);

protected:
    explicit Entry(int64_t id, std::string notes = {});

private:
    std::string notes;
};
