#pragma once

#include "Entry.h"
#include <nlohmann/json.hpp>

class CreditCard : public Entry {
    friend class Entry;

  public:
    explicit CreditCard(std::string cardHolderName, std::string cardNumber, std::string expiration,
                        std::string securityCode, std::string notes = {});

    const std::string& getCardHolderName() const noexcept;
    void setCardHolderName(std::string cardHolderName);

    const std::string& getCardNumber() const noexcept;
    void setCardNumber(std::string cardNumber);

    const std::string& getExpiration() const noexcept;
    void setExpiration(std::string expiration);

    const std::string& getSecurityCode() const noexcept;
    void setSecurityCode(std::string securityCode);

    std::string getType() const override { return "CreditCard"; }

    friend void to_json(json& j, const CreditCard& card);
    friend void from_json(const json& j, CreditCard& card);

  private:
    CreditCard() = default;
    std::string cardHolderName;
    std::string cardNumber;
    std::string expiration;
    std::string securityCode;
};
