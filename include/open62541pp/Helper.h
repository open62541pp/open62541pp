#pragma once

#include <string>

#include "open62541/types.h"
#include "open62541/types_generated_handling.h"

namespace opcua {

inline std::string uaStringToString(UA_String input) noexcept {
    if (input.data == nullptr)
        return {};
    
    return std::string((const char*) input.data, input.length); // NOLINT
}

class QualifiedName {
public:
    QualifiedName(uint16_t namespaceIndex, std::string_view name)
        : name_(UA_QUALIFIEDNAME_ALLOC(namespaceIndex, name.data())) {}

    ~QualifiedName() { UA_QualifiedName_deleteMembers(&name_); }

    QualifiedName(const QualifiedName& other)            { UA_QualifiedName_copy(&other.name_, &name_); }
    QualifiedName& operator=(const QualifiedName& other) { UA_QualifiedName_copy(&other.name_, &name_); return *this; }

    inline bool operator==(const QualifiedName& other) const { return UA_QualifiedName_equal(&name_, other.handle()); }
    inline bool operator!=(const QualifiedName& other) const { return !operator==(other); }

    inline uint16_t    getNamespaceIndex() const { return name_.namespaceIndex; } 
    inline std::string getName()           const { return uaStringToString(name_.name); } 

    inline       UA_QualifiedName* handle()       { return &name_; }
    inline const UA_QualifiedName* handle() const { return &name_; }
private:
    UA_QualifiedName name_;
};


class LocalizedText {
public:
    LocalizedText(std::string_view text, std::string_view locale = "")
        : text_(UA_LOCALIZEDTEXT_ALLOC(locale.data(), text.data())) {}

    ~LocalizedText() { UA_LocalizedText_deleteMembers(&text_); }

    LocalizedText(const LocalizedText& other)            { UA_LocalizedText_copy(&other.text_, &text_); }
    LocalizedText& operator=(const LocalizedText& other) { UA_LocalizedText_copy(&other.text_, &text_); return *this; }

    inline std::string getText()   const { return uaStringToString(text_.text); } 
    inline std::string getLocale() const { return uaStringToString(text_.locale); } 

    inline       UA_LocalizedText* handle()       { return &text_; }
    inline const UA_LocalizedText* handle() const { return &text_; }
private:
    UA_LocalizedText text_;
};

} // namespace opcua
