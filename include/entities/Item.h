#pragma once

#include "Types.h"
#include "snowflake.h"
#include <cstdint>
#include <nlohmann/json.hpp>

using snowflake_t = snowflake<1534832906275L>;

class Item {
  public:
    Item();
    virtual ~Item() = default;

    int64_t getId() const;
    virtual std::string getType() const = 0;

  protected:
    void parseItem(const json& j);
    void serializeItem(json& j) const;

  private:
    int64_t getNextId() const;

    int64_t id;
    static snowflake_t snowflake;
};
