#pragma once

#include "open62541pp/open62541.h"

// hidden open62541 headers needed by open62541++ implementation

// turn off the -Wunused-parameter warning for open62541 (only for gcc/clang)
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include <open62541/config.h>
#include <open62541/types.h>
#include <open62541/types_generated_handling.h>
#include <open62541/server.h>
#if __has_include(<open62541/server_config.h>)  // merged into server.h in v1.2
#include <open62541/server_config.h>
#endif
#include <open62541/server_config_default.h>
#include <open62541/plugin/accesscontrol_default.h>

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
