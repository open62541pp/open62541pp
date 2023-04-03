#include <tuple>

#include "open62541_impl.h"

#define UAPP_OPEN62541_VER_GE(MAJOR, MINOR)                                                        \
    (UA_OPEN62541_VER_MAJOR >= MAJOR) && (UA_OPEN62541_VER_MINOR >= MINOR)

#define UAPP_OPEN62541_VER_LE(MAJOR, MINOR)                                                        \
    (UA_OPEN62541_VER_MAJOR <= MAJOR) && (UA_OPEN62541_VER_MINOR <= MINOR)

namespace opcua::detail {

using VersionNumberTriplet = std::tuple<int, int, int>;

constexpr VersionNumberTriplet open62541Version{
    UA_OPEN62541_VER_MAJOR,
    UA_OPEN62541_VER_MINOR,
    UA_OPEN62541_VER_PATCH,
};

}  // namespace opcua::detail
