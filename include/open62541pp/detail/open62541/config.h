#pragma once

#include "open62541pp/detail/open62541/push_options.h"

#if __has_include(<open62541.h>)

#include <open62541.h>

#else

#include <open62541/config.h>  // NOLINT(misc-header-include-cycle)

#endif

#include "open62541pp/detail/open62541/pop_options.h"
