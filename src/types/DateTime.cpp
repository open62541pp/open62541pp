#include "open62541pp/types/DateTime.h"

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

}  // namespace opcua
