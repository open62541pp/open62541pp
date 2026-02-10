#pragma once

#include "open62541pp/config.hpp"
#include "open62541pp/detail/open62541/push_options.h"

#if __has_include(<open62541.h>)

#include <open62541.h>

#else

#if __has_include(<open62541/common.h>)  // since v1.1
#include <open62541/common.h>
#endif
#if __has_include(<open62541/constants.h>)  // v1.0
#include <open62541/constants.h>
#endif
#include <open62541/nodeids.h>
#include <open62541/statuscodes.h>
#include <open62541/types.h>  // NOLINT(*include-cycle)
#include <open62541/types_generated.h>  // NOLINT(*include-cycle)
#include <open62541/util.h>

// plugins
#include <open62541/plugin/accesscontrol.h>
#include <open62541/plugin/log.h>
#if UAPP_OPEN62541_VER_GE(1, 2)  // nodestore plugins defined in server.h before v1.2
#include <open62541/plugin/nodestore.h>
#endif
#if __has_include(<open62541/plugin/create_certificate.h>)  // since v1.3
#include <open62541/plugin/create_certificate.h>
#endif
#if __has_include(<open62541/plugin/nodesetloader.h>)  // since v1.4
#include <open62541/plugin/nodesetloader.h>
#endif

#endif

#include "open62541pp/detail/open62541/pop_options.h"
