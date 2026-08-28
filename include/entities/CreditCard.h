#pragma once

#include "Entry.h"
#include "Types.h"

class CreditCard : public Entry {
public:
    explicit CreditCard(int64_t id, std::string cardHolderName, std::string cardNumber, std::string expiration, std::string securityCode, std::string notes = {});

    const std::string &getCardHolderName() const noexcept;
    void setCardHolderName(std::string cardHolderName);

    const std::string &getCardNumber() const noexcept;
    void setCardNumber(std::string cardNumber);

    const std::string &getExpiration() const noexcept;
    void setExpiration(std::string expiration);

    const std::string &getSecurityCode() const noexcept;
    void setSecurityCode(std::string securityCode);

private:
    std::string cardHolderName;
    std::string cardNumber;
    std::string expiration;
    std::string securityCode;
};
