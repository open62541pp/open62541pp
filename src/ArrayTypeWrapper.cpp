#include <algorithm>

#include "open62541pp/ArrayTypeWrapper.h"

namespace opcua {

/* ------------------------------------------- String ------------------------------------------- */

StringArray::StringArray(const std::vector<std::string>& vStr)
    : ArrayTypeWrapper<UA_String, 11>(vStr.size()) {
    for (size_t i = 0; i < size(); i++) {
        *getPtrAt(i) = detail::allocUaString(vStr[i]);
    }
}

std::vector<std::string> StringArray::get() const {
    std::vector<std::string> v;
    int index = 0;
    std::generate_n(std::back_inserter(v), size(), [this, &index] {
        return detail::toString(operator[](index++));
    });
    return v;
}

}  // namespace opcua
