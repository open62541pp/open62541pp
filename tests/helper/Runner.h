#pragma once

#include <atomic>
#include <thread>

#include "open62541pp/Server.h"

using namespace opcua;

/// Helper class to run server in background thread.
class ServerRunner {
public:
    explicit ServerRunner(Server& server) {
        server.setLogger({});  // disable logging to prevent data races
        server.runIterate();  // make sure server is running within constructor
        thread_ = std::thread([&] {
            while (!stopFlag_) {
                server.runIterate();
                // no sleep here, process server events as fast a possible
            }
        });
    }

    ~ServerRunner() {
        stopFlag_ = true;
        thread_.join();
    }

private:
    std::atomic<bool> stopFlag_{false};
    std::thread thread_;
};
