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

    friend std::unique_ptr<Entry> Entry::parse(const json& input);
    friend void to_json(json& j, const Wifi& wifi);
    friend void from_json(const json& j, Wifi& wifi);

  private:
    Wifi() = default;
    std::string networkName;
    std::string password;
};
