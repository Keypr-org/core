#include "entities/Persona.h"

Persona::Persona(int64_t id, std::string firstName, std::string lastName, DateTime dateOfBirth, std::string address, std::string phone)
    : DatedItem(id), firstName(std::move(firstName)), lastName(std::move(lastName)), dateOfBirth(dateOfBirth), address(std::move(address)), phone(std::move(phone)) {
}

const std::string &Persona::getFirstName() const noexcept {
    return firstName;
}

void Persona::setFirstName(std::string firstName) {
    this->firstName = std::move(firstName);
}

const std::string &Persona::getLastName() const noexcept {
    return lastName;
}

void Persona::setLastName(std::string lastName) {
    this->lastName = std::move(lastName);
}

DateTime Persona::getDateOfBirth() const noexcept {
    return dateOfBirth;
}

void Persona::setDateOfBirth(DateTime dateOfBirth) noexcept {
    this->dateOfBirth = dateOfBirth;
}

const std::string &Persona::getAddress() const noexcept {
    return address;
}

void Persona::setAddress(std::string address) {
    this->address = std::move(address);
}

const std::string &Persona::getPhone() const noexcept {
    return phone;
}

void Persona::setPhone(std::string phone) {
    this->phone = std::move(phone);
}
