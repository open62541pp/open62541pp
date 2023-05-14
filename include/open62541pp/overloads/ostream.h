#include <ostream>

namespace opcua {

// forward declarations
class String;
class Guid;
class XmlElement;

std::ostream& operator<<(std::ostream& os, const String& string);
// std::ostream& operator<<(std::ostream& os, const ByteString& byteString);  // as hex string?
std::ostream& operator<<(std::ostream& os, const Guid& guid);
std::ostream& operator<<(std::ostream& os, const XmlElement& xmlElement);

}  // namespace opcua
