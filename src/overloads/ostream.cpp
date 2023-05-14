#include "open62541pp/overloads/ostream.h"

#include "open62541pp/types/Builtin.h"

namespace opcua {

std::ostream& operator<<(std::ostream& os, const String& string) {
    os << string.get();
    return os;
}

std::ostream& operator<<(std::ostream& os, const Guid& guid) {
    os << guid.toString();
    return os;
}

std::ostream& operator<<(std::ostream& os, const XmlElement& xmlElement) {
    os << xmlElement.get();
    return os;
}

}  // namespace opcua
