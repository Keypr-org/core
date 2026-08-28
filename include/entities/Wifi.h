#pragma once

#include "Entry.h"

#include <nlohmann/json.hpp>

class Wifi : public Entry {
    friend class Entry;

  public:
    explicit Wifi(std::string networkName, std::string password, std::string notes = {});

    const std::string& getNetworkName() const noexcept;
    void setNetworkName(std::string networkName);

    const std::string& getPassword() const noexcept;
    void setPassword(std::string password);

    std::string getType() const override { return "Wifi"; }

    // Generate to_json/from_json for Wifi
    NLOHMANN_DEFINE_DERIVED_TYPE_INTRUSIVE(Wifi, Entry, networkName, password)

    friend std::unique_ptr<Entry> Entry::parse(const json& input);

  private:
    Wifi() = default;
    std::string networkName;
    std::string password;
};
