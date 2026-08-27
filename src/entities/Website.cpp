#include "entities/Website.h"


Website::Website(int64_t id, std::string notes, std::string title, std::string username, std::string password, std::string url, std::string comments, std::shared_ptr<Persona> persona, std::string alias)
    : Entry(id, std::move(notes)), title(std::move(title)), comments(std::move(comments)), username(std::move(username)), password(std::move(password)), url(std::move(url)), persona(persona), alias(std::move(alias)) {
}

const std::string &Website::getTitle() const noexcept {
    return title;
}

void Website::setTitle(std::string title) {
    this->title = std::move(title);
}

const std::string &Website::getComments() const noexcept {
    return comments;
}

void Website::setComments(std::string comments) {
    this->comments = std::move(comments);
}

const std::string &Website::getUsername() const noexcept {
    return username;
}

void Website::setUsername(std::string username) {
    this->username = std::move(username);
}

const std::string &Website::getPassword() const noexcept {
    return password;
}

void Website::setPassword(std::string password) {
    this->password = std::move(password);
}

const std::string &Website::getUrl() const noexcept {
    return url;
}

void Website::setUrl(std::string url) {
    this->url = std::move(url);
}

std::shared_ptr<Persona> Website::getPersona() const noexcept {
    return persona.lock();
}

void Website::setPersona(std::shared_ptr<Persona> persona) {
    this->persona = persona;
}

const std::string &Website::getAlias() const noexcept {
    return alias;
}

void Website::setAlias(std::string alias) {
    this->alias = std::move(alias);
}
