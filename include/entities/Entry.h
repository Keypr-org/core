#pragma once

#include "DatedItem.h"
#include "Types.h"
#include <string>

class Entry : public DatedItem {
  public:
    virtual ~Entry() = default;

    const std::string& getNotes() const noexcept;
    void setNotes(std::string notes);

    virtual std::string getType() const = 0;

    static std::unique_ptr<Entry> parse(const json& input);
    static json serialize(const Entry& entry);

  protected:
    explicit Entry(std::string notes = {});
    void parseEntry(const json& j);
    void serializeEntry(json& j) const;

  private:
    std::string notes;
};
