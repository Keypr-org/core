#include "entities/Wifi.h"

Wifi::Wifi(std::string networkName, std::string password, std::string notes)
    : Entry(std::move(notes)), networkName(std::move(networkName)), password(std::move(password)) {}

const std::string& Wifi::getNetworkName() const noexcept {
    return networkName;
}

void Wifi::setNetworkName(std::string networkName) {
    this->networkName = std::move(networkName);
    setLastModifiedDate(std::chrono::system_clock::now());
}

const std::string& Wifi::getPassword() const noexcept {
    return password;
}

void Wifi::setPassword(std::string password) {
    this->password = std::move(password);
    setLastModifiedDate(std::chrono::system_clock::now());
}

void to_json(json& j, const Wifi& wifi) {
    wifi.serializeEntry(j);

    j["networkName"] = wifi.networkName;
    j["password"] = wifi.password;
}

void from_json(const json& j, Wifi& wifi) {
    wifi.parseEntry(j);

    j.at("networkName").get_to(wifi.networkName);
    j.at("password").get_to(wifi.password);
}
