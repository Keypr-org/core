#include "entities/Wifi.h"

Wifi::Wifi(std::string networkName, std::string password, std::string notes)
    : Entry(std::move(notes)), networkName(std::move(networkName)), password(std::move(password)) {
}

const std::string &Wifi::getNetworkName() const noexcept {
    return networkName;
}

void Wifi::setNetworkName(std::string networkName) {
    this->networkName = std::move(networkName);
    setLastModifiedDate(std::chrono::system_clock::now());
}

const std::string &Wifi::getPassword() const noexcept {
    return password;
}

void Wifi::setPassword(std::string password) {
    this->password = std::move(password);
    setLastModifiedDate(std::chrono::system_clock::now());
}
