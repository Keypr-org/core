#include "entities/CreditCard.h"

CreditCard::CreditCard(std::string cardHolderName, std::string cardNumber, std::string expiration,
                       std::string securityCode, std::string notes)
    : Entry(std::move(notes)), cardHolderName(std::move(cardHolderName)),
      cardNumber(std::move(cardNumber)), expiration(std::move(expiration)),
      securityCode(std::move(securityCode)) {}

const std::string& CreditCard::getCardHolderName() const noexcept {
    return cardHolderName;
}

void CreditCard::setCardHolderName(std::string cardHolderName) {
    this->cardHolderName = std::move(cardHolderName);
    setLastModifiedDate(std::chrono::system_clock::now());
}

const std::string& CreditCard::getCardNumber() const noexcept {
    return cardNumber;
}

void CreditCard::setCardNumber(std::string cardNumber) {
    this->cardNumber = std::move(cardNumber);
    setLastModifiedDate(std::chrono::system_clock::now());
}

const std::string& CreditCard::getExpiration() const noexcept {
    return expiration;
}

void CreditCard::setExpiration(std::string expiration) {
    this->expiration = std::move(expiration);
    setLastModifiedDate(std::chrono::system_clock::now());
}

const std::string& CreditCard::getSecurityCode() const noexcept {
    return securityCode;
}

void CreditCard::setSecurityCode(std::string securityCode) {
    this->securityCode = std::move(securityCode);
    setLastModifiedDate(std::chrono::system_clock::now());
}

void to_json(json& j, const CreditCard& card) {
    card.serializeEntry(j);

    j["cardHolderName"] = card.cardHolderName;
    j["cardNumber"] = card.cardNumber;
    j["expiration"] = card.expiration;
    j["securityCode"] = card.securityCode;
}

void from_json(const json& j, CreditCard& card) {
    card.parseEntry(j);

    j.at("cardHolderName").get_to(card.cardHolderName);
    j.at("cardNumber").get_to(card.cardNumber);
    j.at("expiration").get_to(card.expiration);
    j.at("securityCode").get_to(card.securityCode);
}
