#pragma once

#include <iostream>

#include "open62541pp/Logger.h"

#include "open62541_impl.h"  // UA_Logger

namespace opcua {

class CustomLogger {
public:
    explicit CustomLogger(UA_Logger& logger);

    ~CustomLogger() {
        std::cout << "Shutdown!\n";
    }

    void setLogger(Logger logger);
    const Logger& getLogger() const noexcept;

private:
    UA_Logger& nativeLogger_;
    Logger logger_;
};

}  // namespace opcua
