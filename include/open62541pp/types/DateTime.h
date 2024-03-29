#pragma once

#include <chrono>
#include <cstdint>
#include <ratio>
#include <string>
#include <string_view>

#include "open62541pp/TypeWrapper.h"
#include "open62541pp/detail/open62541/common.h"

namespace opcua {

/**
 * UA_DateTime wrapper class.
 *
 * An instance in time. A DateTime value is encoded as a 64-bit signed integer which represents the
 * number of 100 nanosecond intervals since January 1, 1601 (UTC).
 *
 * @see https://reference.opcfoundation.org/Core/Part6/v105/docs/5.2.2.5
 * @ingroup Wrapper
 */
class DateTime : public TypeWrapper<UA_DateTime, UA_TYPES_DATETIME> {
public:
    using DefaultClock = std::chrono::system_clock;
    using UaDuration = std::chrono::duration<int64_t, std::ratio<1, 10'000'000>>;

    using TypeWrapper::TypeWrapper;  // inherit constructors

    template <typename Clock, typename Duration>
    DateTime(std::chrono::time_point<Clock, Duration> timePoint)  // NOLINT, implicit wanted
        : DateTime(fromTimePoint(timePoint)) {}

    /// Get current DateTime.
    static DateTime now() noexcept {
        return DateTime(UA_DateTime_now());  // NOLINT
    }

    /// Get DateTime from std::chrono::time_point.
    template <typename Clock, typename Duration>
    static DateTime fromTimePoint(std::chrono::time_point<Clock, Duration> timePoint) {
        return DateTime(
            int64_t{UA_DATETIME_UNIX_EPOCH} +
            std::chrono::duration_cast<UaDuration>(timePoint.time_since_epoch()).count()
        );
    }

    /// Get DateTime from Unix time.
    static DateTime fromUnixTime(int64_t unixTime) noexcept {
        return DateTime(UA_DateTime_fromUnixTime(unixTime));  // NOLINT
    }

    /// Offset of local time to UTC.
    static int64_t localTimeUtcOffset() noexcept {
        return UA_DateTime_localTimeUtcOffset();
    }

    /// Convert to std::chrono::time_point.
    template <typename Clock = DefaultClock, typename Duration = UaDuration>
    std::chrono::time_point<Clock, Duration> toTimePoint() const {
        const std::chrono::time_point<Clock, Duration> unixEpoch{};
        if (get() < UA_DATETIME_UNIX_EPOCH) {
            return unixEpoch;
        }
        const auto sinceEpoch = UaDuration(get() - UA_DATETIME_UNIX_EPOCH);
        return unixEpoch + std::chrono::duration_cast<Duration>(sinceEpoch);
    }

    /// Convert to Unix time (number of seconds since January 1, 1970 UTC).
    int64_t toUnixTime() const noexcept {
        if (get() < UA_DATETIME_UNIX_EPOCH) {
            return 0;
        }
        return UA_DateTime_toUnixTime(get());
    }

    /// Convert to UA_DateTimeStruct.
    UA_DateTimeStruct toStruct() const noexcept {
        return UA_DateTime_toStruct(get());
    }

    /// Get DateTime value as 100 nanosecond intervals since January 1, 1601 (UTC).
    int64_t get() const noexcept {
        return *handle();
    }

    /// Convert to string with given format (same format codes as strftime).
    /// @see https://en.cppreference.com/w/cpp/chrono/c/strftime
    std::string format(std::string_view format, bool localtime = false) const;
};

}  // namespace opcua
