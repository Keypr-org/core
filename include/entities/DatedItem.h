#pragma once

#include "Item.h"
#include "Types.h"

class DatedItem : public Item {
public:
    explicit DatedItem(int64_t id, DateTime creationDate = std::chrono::system_clock::now());
    explicit DatedItem(int64_t id, DateTime creationDate, DateTime lastModifiedDate);
    virtual ~DatedItem() = default;

    DateTime getCreationDate() const noexcept;
    DateTime getLastModifiedDate() const noexcept;

protected:
    void setLastModifiedDate(DateTime lastModifiedDate) noexcept;

private:
    DateTime creationAt;
    DateTime updatedAt;
};
