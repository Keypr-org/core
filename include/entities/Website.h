#pragma once

#include "Entry.h"
#include "Persona.h"
#include <string>
#include <memory>

class Website : public Entry {
public:
    explicit Website(std::string notes, std::string title, std::string username, std::string password, std::string url, std::string comments = "", std::shared_ptr<Persona> persona = nullptr, std::string alias = "");

    const std::string &getTitle() const noexcept;
    void setTitle(std::string title);

    const std::string &getComments() const noexcept;
    void setComments(std::string comments);

    const std::string &getUsername() const noexcept;
    void setUsername(std::string username);

    const std::string &getPassword() const noexcept;
    void setPassword(std::string password);

    const std::string &getUrl() const noexcept;
    void setUrl(std::string url);

    std::weak_ptr<Persona> getPersona() const noexcept;
    void setPersona(std::shared_ptr<Persona> persona);

    const std::string &getAlias() const noexcept;
    void setAlias(std::string alias);

private:
    std::string title;
    std::string comments;
    std::string username;
    std::string password;
    std::string url;
    std::weak_ptr<Persona> persona;
    std::string alias;
};
