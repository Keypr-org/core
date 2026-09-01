#include "entities/Website.h"

Website::Website(std::string notes, std::string title, std::string username, std::string password,
                 std::string url, std::string comments, int64_t personaId, std::string aliasId,
                 std::string alias)
    : Entry(std::move(notes)), title(std::move(title)), comments(std::move(comments)),
      username(std::move(username)), password(std::move(password)), url(std::move(url)),
      personaId(personaId), aliasId(std::move(aliasId)), alias(std::move(alias)) {}

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

int64_t Website::getPersonaId() const noexcept {
    return personaId;
}

void Website::setPersona(int64_t personaId) {
    this->personaId = personaId;
    setLastModifiedDate(std::chrono::system_clock::now());
}

const std::string& Website::getAliasId() const noexcept {
    return aliasId;
}

void Website::setAliasId(std::string aliasId) {
    this->aliasId = std::move(aliasId);
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
    j["aliasId"] = website.aliasId;
    j["alias"] = website.alias;
    j["personaId"] = website.personaId;
}

void from_json(const json& j, Website& website) {
    website.parseEntry(j);

    j.at("title").get_to(website.title);
    j.at("comments").get_to(website.comments);
    j.at("username").get_to(website.username);
    j.at("password").get_to(website.password);
    j.at("url").get_to(website.url);
    j.at("aliasId").get_to(website.aliasId);
    j.at("alias").get_to(website.alias);
    j.at("personaId").get_to(website.personaId);
}
