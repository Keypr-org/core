#pragma once

#include "DatedItem.h"
#include <nlohmann/json.hpp>

class Persona : public DatedItem {
  public:
    // Default constructor needed for JSON parsing
    Persona() = default;
    explicit Persona(std::string firstName, std::string lastName, DateTime dateOfBirth,
                     std::string address, std::string phone);
    explicit Persona(DateTime createdAt, DateTime updatedAt, std::string firstName,
                     std::string lastName, DateTime dateOfBirth, std::string address,
                     std::string phone);

    const std::string& getFirstName() const noexcept;
    void setFirstName(std::string firstName);

    const std::string& getLastName() const noexcept;
    void setLastName(std::string lastName);

    DateTime getDateOfBirth() const noexcept;
    void setDateOfBirth(DateTime dateOfBirth) noexcept;

    const std::string& getAddress() const noexcept;
    void setAddress(std::string address);

    const std::string& getPhone() const noexcept;
    void setPhone(std::string phone);

    std::string getType() const override { return "Persona"; }

    friend void to_json(json& j, const Persona& persona);
    friend void from_json(const json& j, Persona& persona);

  private:
    std::string firstName;
    std::string lastName;
    DateTime dateOfBirth;
    std::string address;
    std::string phone;
};
