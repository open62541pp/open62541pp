#include <ostream>

#include "open62541pp/types/Builtin.h"

namespace opcua {

std::ostream& operator<<(std::ostream& os, const String& string);
// std::ostream& operator<<(std::ostream& os, const ByteString& byteString);  // as hex string?
std::ostream& operator<<(std::ostream& os, const Guid& guid);
std::ostream& operator<<(std::ostream& os, const XmlElement& xmlElement);

}  // namespace opcua
