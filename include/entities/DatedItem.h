#pragma once

#include "Item.h"
#include "Types.h"

class DatedItem : public Item {
public:
    explicit DatedItem(DateTime creationDate = std::chrono::system_clock::now());
    explicit DatedItem(DateTime creationDate, DateTime lastModifiedDate);
    virtual ~DatedItem() = default;

    DateTime getCreationDate() const noexcept;
    DateTime getLastModifiedDate() const noexcept;

protected:
    void setLastModifiedDate(DateTime lastModifiedDate) noexcept;

private:
    DateTime creationAt;
    DateTime updatedAt;
};
