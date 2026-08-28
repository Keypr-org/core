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

    // Generate to_json/from_json for CreditCard
    NLOHMANN_DEFINE_DERIVED_TYPE_INTRUSIVE(CreditCard, Entry, cardHolderName, cardNumber,
                                           expiration, securityCode)

  private:
    CreditCard() = default;
    std::string cardHolderName;
    std::string cardNumber;
    std::string expiration;
    std::string securityCode;
};
