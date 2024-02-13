#include <doctest/doctest.h>

#include <stdexcept>

#include "open62541pp/ErrorHandling.h"

using namespace opcua;

TEST_CASE("getStatusCode") {
    CHECK(detail::getStatusCode(nullptr) == UA_STATUSCODE_GOOD);
    CHECK(
        detail::getStatusCode(std::make_exception_ptr(BadStatus(UA_STATUSCODE_GOOD))) ==
        UA_STATUSCODE_GOOD
    );
    CHECK(
        detail::getStatusCode(std::make_exception_ptr(BadStatus(UA_STATUSCODE_BADDISCONNECT))) ==
        UA_STATUSCODE_BADDISCONNECT
    );
    CHECK(
        detail::getStatusCode(std::make_exception_ptr(std::runtime_error("test"))) ==
        UA_STATUSCODE_BADINTERNALERROR
    );
}
