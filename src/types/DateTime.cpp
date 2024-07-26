#include "open62541pp/types/DateTime.h"

#include <ctime>  // gmtime, localtime
#include <iomanip>  // put_time
#include <sstream>

namespace opcua {

std::string DateTime::format(std::string_view format, bool localtime) const {
    const std::time_t unixTime = toUnixTime();
    std::ostringstream ss;
    const auto* timeinfo = localtime ? std::localtime(&unixTime) : std::gmtime(&unixTime);
    if (timeinfo != nullptr) {
        ss << std::put_time(timeinfo, std::string(format).c_str());
    }
    return ss.str();
}

}  // namespace opcua
