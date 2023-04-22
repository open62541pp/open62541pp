#include "open62541pp/types/DateTime.h"

#include <ctime>  // gmtime, localtime
#include <iomanip>  // put_time
#include <sstream>

#include "../open62541_impl.h"

namespace opcua {

DateTime DateTime::now() {
    return DateTime(UA_DateTime_now());  // NOLINT
}

DateTime DateTime::fromUnixTime(int64_t unixTime) {
    return DateTime(UA_DateTime_fromUnixTime(unixTime));  // NOLINT
}

int64_t DateTime::localTimeUtcOffset() {
    return UA_DateTime_localTimeUtcOffset();
}

int64_t DateTime::toUnixTime() const noexcept {
    if (get() < UA_DATETIME_UNIX_EPOCH) {
        return 0;
    }
    return UA_DateTime_toUnixTime(get());
}

UA_DateTimeStruct DateTime::toStruct() const {
    return UA_DateTime_toStruct(get());
}

int64_t DateTime::get() const noexcept {
    return *handle();
}

std::string DateTime::format(std::string_view format, bool localtime) const {
    const std::time_t unixTime = toUnixTime();
    const std::string formatStr(format);
    std::stringstream ss;
    if (localtime) {
        ss << std::put_time(std::localtime(&unixTime), formatStr.c_str());
    } else {
        ss << std::put_time(std::gmtime(&unixTime), formatStr.c_str());
    }
    return ss.str();
}

}  // namespace opcua
