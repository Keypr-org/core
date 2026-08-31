#pragma once

#include "Item.h"
#include "Types.h"
#include <nlohmann/json.hpp>

class DatedItem : public Item {
  public:
    explicit DatedItem(DateTime creationDate = std::chrono::system_clock::now());
    explicit DatedItem(DateTime creationDate, DateTime lastModifiedDate);
    virtual ~DatedItem() = default;

    DateTime getCreationDate() const noexcept;
    DateTime getLastModifiedDate() const noexcept;

    virtual std::string getType() const = 0;

  protected:
    void setLastModifiedDate(DateTime lastModifiedDate) noexcept;

    void parseDatedItem(const json& j);
    void serializeDatedItem(json& j) const;

  private:
    DateTime creationAt;
    DateTime updatedAt;
};
