#pragma once

#include "open62541pp/detail/open62541/push_options.h"

#if __has_include(<open62541.h>)

#include <open62541.h>

#else

#include <open62541/server.h>
#if __has_include(<open62541/server_config.h>)  // merged into server.h in v1.2
#include <open62541/server_config.h>
#endif
#include <open62541/server_config_default.h>
#ifdef UA_ENABLE_HISTORIZING
#include <open62541/plugin/historydata/history_data_backend_memory.h>
#include <open62541/plugin/historydata/history_data_gathering_default.h>
#include <open62541/plugin/historydata/history_database_default.h>
#endif

#endif

#include "open62541pp/detail/open62541/pop_options.h"
