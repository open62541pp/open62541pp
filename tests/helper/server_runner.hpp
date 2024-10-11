#pragma once

#include <atomic>
#include <thread>

#include "open62541pp/server.hpp"

using namespace opcua;

/// Helper class to run server in background thread.
class ServerRunner {
public:
    explicit ServerRunner(Server& server) {
        // disable logging to prevent data races
        if (auto* logger = detail::getLogger(&detail::getConfig(server)); logger != nullptr) {
            getEmptyLogger().assign(*logger);
        }
        server.runIterate();  // make sure server is running within constructor
        thread_ = std::thread([&] {
            while (!stopFlag_) {
                server.runIterate();
                // no sleep here, process server events as fast a possible
            }
        });
    }

    void stop() {
        if (!stopFlag_) {
            stopFlag_ = true;
            thread_.join();
        }
    }

    ~ServerRunner() {
        stop();
    }

private:
    LoggerDefault& getEmptyLogger() {
        static LoggerDefault logger([](auto&&...) {});
        return logger;
    }

    std::atomic<bool> stopFlag_{false};
    std::thread thread_;
};
