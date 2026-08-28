#pragma once

#include "DatedItem.h"

class Persona : public DatedItem {
public:
    explicit Persona(int64_t id, std::string firstName, std::string lastName, DateTime dateOfBirth, std::string address, std::string phone);
    explicit Persona(int64_t id, DateTime createdAt, DateTime updatedAt, std::string firstName, std::string lastName, DateTime dateOfBirth, std::string address, std::string phone);

    const std::string &getFirstName() const noexcept;
    void setFirstName(std::string firstName);

    const std::string &getLastName() const noexcept;
    void setLastName(std::string lastName);

    DateTime getDateOfBirth() const noexcept;
    void setDateOfBirth(DateTime dateOfBirth) noexcept;

    const std::string &getAddress() const noexcept;
    void setAddress(std::string address);

    const std::string &getPhone() const noexcept;
    void setPhone(std::string phone);

private:
    std::string firstName;
    std::string lastName;
    DateTime dateOfBirth;
    std::string address;
    std::string phone;
};
