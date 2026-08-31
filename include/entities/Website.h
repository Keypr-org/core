#pragma once

#include "Entry.h"
#include <nlohmann/json.hpp>
#include <string>

class Website : public Entry {
    friend class Entry;

  public:
    explicit Website(std::string notes, std::string title, std::string username,
                     std::string password, std::string url, std::string comments = "",
                     int64_t personaId = -1, std::string alias = "");

    const std::string& getTitle() const noexcept;
    void setTitle(std::string title);

    const std::string& getComments() const noexcept;
    void setComments(std::string comments);

    const std::string& getUsername() const noexcept;
    void setUsername(std::string username);

    const std::string& getPassword() const noexcept;
    void setPassword(std::string password);

    const std::string& getUrl() const noexcept;
    void setUrl(std::string url);

    int64_t getPersonaId() const noexcept;
    void setPersona(int64_t personaId);

    const std::string& getAlias() const noexcept;
    void setAlias(std::string alias);

    std::string getType() const override { return "Website"; }

    friend void to_json(json& j, const Website& website);
    friend void from_json(const json& j, Website& website);

  private:
    Website() = default;
    std::string title;
    std::string comments;
    std::string username;
    std::string password;
    std::string url;
    int64_t personaId;
    std::string alias;
};
