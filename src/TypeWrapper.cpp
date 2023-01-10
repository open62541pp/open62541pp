#include "open62541pp/TypeWrapper.h"

namespace opcua {

String::String(std::string_view str)
    : String(UA_String{detail::allocUaString(str)}) {}

bool String::operator==(const String& other) const noexcept {
    return UA_String_equal(handle(), other.handle());
}

bool String::operator!=(const String& other) const noexcept {
    return !operator==(other);
}

std::string String::get() const {
    return detail::toString(*handle());
}

std::string_view String::getView() const {
    return detail::toStringView(*handle());
}

}  // namespace opcua
