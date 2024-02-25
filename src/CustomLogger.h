#pragma once

#include "open62541pp/Logger.h"

#include "open62541_impl.h"  // UA_Logger

namespace opcua {

class CustomLogger {
public:
    void set(UA_Logger& native, Logger logger);
    const Logger& get() const noexcept;

private:
    Logger logger_;
};

}  // namespace opcua
