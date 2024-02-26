#pragma once

#include "open62541pp/detail/open62541/push_options.h"

#if __has_include(<open62541.h>)

#include <open62541.h>

#else

#include <open62541/common.h>
#include <open62541/nodeids.h>
#include <open62541/statuscodes.h>
#include <open62541/types.h>
#include <open62541/types_generated.h>
#include <open62541/util.h>

// plugins
#include <open62541/plugin/accesscontrol.h>
#include <open62541/plugin/log.h>
#if __has_include(<open62541/plugin/create_certificate.h>)  // since v1.3
#include <open62541/plugin/create_certificate.h>
#endif

#endif

#include "open62541pp/detail/open62541/pop_options.h"
