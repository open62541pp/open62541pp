#pragma once

#include "open62541pp/detail/open62541/push_options.h"

#if __has_include(<open62541.h>)

#include <open62541.h>

#else

#include <open62541/client.h>
#if __has_include(<open62541/client_config.h>)  // merged into client.h in v1.2
#include <open62541/client_config.h>
#endif
#include <open62541/client_config_default.h>
#include <open62541/client_subscriptions.h>

#endif

#include "open62541pp/detail/open62541/pop_options.h"
