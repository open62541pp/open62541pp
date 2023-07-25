#pragma once

#include "open62541pp/open62541.h"

// hidden open62541 headers needed by open62541++ implementation

// turn off the -Wunused-parameter warning for open62541 (only for gcc/clang)
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#if __has_include(<open62541.h>)
// UA_ENABLE_AMALGAMATION=ON
#include <open62541.h>
#else

// UA_ENABLE_AMALGAMATION=OFF
// common
#include <open62541/config.h>
#include <open62541/plugin/accesscontrol.h>
#include <open62541/plugin/accesscontrol_default.h>
#include <open62541/plugin/log.h>
#if __has_include(<open62541/plugin/create_certificate.h>)  // since v1.3
#include <open62541/plugin/create_certificate.h>
#endif
#include <open62541/types_generated_handling.h>

// client
#include <open62541/client.h>
#if __has_include(<open62541/client_config.h>)  // merged into client.h in v1.2
#include <open62541/client_config.h>
#endif
#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/client_subscriptions.h>

// server
#include <open62541/server.h>
#if __has_include(<open62541/server_config.h>)  // merged into server.h in v1.2
#include <open62541/server_config.h>
#endif
#include <open62541/server_config_default.h>

#endif

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
