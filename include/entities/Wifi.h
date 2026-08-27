#pragma once

#include "Entry.h"

class Wifi : public Entry {
public:
    explicit Wifi(int64_t id, std::string networkName, std::string password, std::string notes = {});

    const std::string &getNetworkName() const noexcept;
    void setNetworkName(std::string networkName);

    const std::string &getPassword() const noexcept;
    void setPassword(std::string password);

private:
    std::string networkName;
    std::string password;
};
