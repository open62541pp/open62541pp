#include <chrono>
#include <iostream>

#include "open62541pp/callback.hpp"
#include "open62541pp/server.hpp"

int main() {
    opcua::Server server;

    size_t counter = 0;
    const double interval = 500;  // milliseconds
    const opcua::CallbackId id1 = opcua::addRepeatedCallback(
        server,
        [&] {
            ++counter;
            std::cout << "Repeated callback: " << counter << "\n"; },
        interval
    );

    const opcua::CallbackId id2 = opcua::addTimedCallback(
        server,
        [&] {
            std::cout << "Timed callback: Double interval of repeated callback\n";
            opcua::changeRepeatedCallbackInterval(server, id1, interval * 2);
        },
        opcua::DateTime::now() + std::chrono::seconds(2)
    );

    server.run();

    opcua::removeCallback(server, id1);
    opcua::removeCallback(server, id2);
}
