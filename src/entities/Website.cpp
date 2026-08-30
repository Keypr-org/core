#include "entities/Website.h"

Website::Website(std::string notes, std::string title, std::string username, std::string password,
                 std::string url, std::string comments, std::shared_ptr<Persona> persona,
                 std::string alias)
    : Entry(std::move(notes)), title(std::move(title)), comments(std::move(comments)),
      username(std::move(username)), password(std::move(password)), url(std::move(url)),
      persona(persona), alias(std::move(alias)) {}

const std::string& Website::getTitle() const noexcept {
    return title;
}

void Website::setTitle(std::string title) {
    this->title = std::move(title);
    setLastModifiedDate(std::chrono::system_clock::now());
}

const std::string& Website::getComments() const noexcept {
    return comments;
}

void Website::setComments(std::string comments) {
    this->comments = std::move(comments);
    setLastModifiedDate(std::chrono::system_clock::now());
}

const std::string& Website::getUsername() const noexcept {
    return username;
}

void Website::setUsername(std::string username) {
    this->username = std::move(username);
    setLastModifiedDate(std::chrono::system_clock::now());
}

const std::string& Website::getPassword() const noexcept {
    return password;
}

void Website::setPassword(std::string password) {
    this->password = std::move(password);
    setLastModifiedDate(std::chrono::system_clock::now());
}

const std::string& Website::getUrl() const noexcept {
    return url;
}

void Website::setUrl(std::string url) {
    this->url = std::move(url);
    setLastModifiedDate(std::chrono::system_clock::now());
}

std::weak_ptr<Persona> Website::getPersona() const noexcept {
    return persona;
}

void Website::setPersona(std::shared_ptr<Persona> persona) {
    this->persona = persona;
    setLastModifiedDate(std::chrono::system_clock::now());
}

const std::string& Website::getAlias() const noexcept {
    return alias;
}

void Website::setAlias(std::string alias) {
    this->alias = std::move(alias);
    setLastModifiedDate(std::chrono::system_clock::now());
}

void to_json(json& j, const Website& website) {
    website.serializeEntry(j);

    j["title"] = website.title;
    j["comments"] = website.comments;
    j["username"] = website.username;
    j["password"] = website.password;
    j["url"] = website.url;
    j["alias"] = website.alias;

    if (auto persona = website.persona.lock()) {
        j["personaId"] = persona->getId();
    } else {
        j["personaId"] = nullptr;
    }
}

void from_json(const json& j, Website& website) {
    website.parseEntry(j);

    j.at("title").get_to(website.title);
    j.at("comments").get_to(website.comments);
    j.at("username").get_to(website.username);
    j.at("password").get_to(website.password);
    j.at("url").get_to(website.url);
    j.at("alias").get_to(website.alias);

    // TODO: Resolve persona lookup. We must point to an existing persona object as we store the
    // persona relationship as a pointer
    website.persona.reset();
}
