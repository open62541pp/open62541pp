#pragma once

#include <cstdint>
#include <functional>

#include "open62541pp/types.hpp"  // DateTime

namespace opcua {

class Client;
class Server;

using CallbackId = uint64_t;
using TimedCallback = std::function<void()>;
using RepeatedCallback = std::function<void()>;

/**
 * Add a callback for execution at a specified time.
 * If the indicated time lies in the past, then the callback is executed at the next iteration of
 * the event loop.
 * @param connection Instance of type Client or Server
 * @param callback Callback to be executed
 * @param date Execution time for the callback
 * @return Callback identifier
 */
template <typename T>
[[nodiscard]] CallbackId addTimedCallback(T& connection, TimedCallback callback, DateTime date);

/**
 * Add a callback for repeated execution.
 * @param connection Instance of type Client or Server
 * @param callback Callback to be executed
 * @param intervalMilliseconds Execution interval in milliseconds (must be > 0).
 *        The first execution occurs at now() + interval at the latest.
 * @return Callback identifier
 */
template <typename T>
[[nodiscard]] CallbackId addRepeatedCallback(
    T& connection, RepeatedCallback callback, double intervalMilliseconds
);

/**
 * Change the execution interval of an existing repeated callback.
 * @param connection Instance of type Client or Server
 * @param callbackId Callback identifier
 * @param intervalMilliseconds New interval in milliseconds (must be > 0)
 */
template <typename T>
void changeRepeatedCallbackInterval(
    T& connection, CallbackId callbackId, double intervalMilliseconds
);

/**
 * Remove a previously registered callback.
 * @param connection Instance of type Client or Server
 * @param callbackId Callback identifier
 */
template <typename T>
void removeCallback(T& connection, CallbackId callbackId);

}  // namespace opcua
