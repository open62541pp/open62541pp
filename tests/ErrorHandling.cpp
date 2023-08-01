#include <doctest/doctest.h>

#include "open62541pp/ErrorHandling.h"

using namespace opcua;

TEST_CASE("invokeCatchIgnore") {
    CHECK_NOTHROW(detail::invokeCatchIgnore([] { return; }));
    CHECK_NOTHROW(detail::invokeCatchIgnore([] { throw BadStatus(UA_STATUSCODE_BADTIMEOUT); }));
    CHECK_NOTHROW(detail::invokeCatchIgnore([] { throw std::runtime_error("test"); }));
}

TEST_CASE("invokeCatchStatus") {
    CHECK_EQ(detail::invokeCatchStatus([] { return; }), UA_STATUSCODE_GOOD);
    CHECK_EQ(
        detail::invokeCatchStatus([] { return UA_STATUSCODE_BADTIMEOUT; }), UA_STATUSCODE_BADTIMEOUT
    );
    CHECK_EQ(
        detail::invokeCatchStatus([] { throw BadStatus(UA_STATUSCODE_BADTIMEOUT); }),
        UA_STATUSCODE_BADTIMEOUT
    );
    CHECK_EQ(
        detail::invokeCatchStatus([] { throw std::runtime_error("test"); }),
        UA_STATUSCODE_BADINTERNALERROR
    );
}
