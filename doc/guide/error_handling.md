# Error handling {#error_handling}

@tableofcontents

In OPC UA, [status codes](https://reference.opcfoundation.org/Core/Part4/v105/docs/7.39) are used to indicate the result of an operation or request. They are returned as part of service responses and node values to describe the outcome, including success, failure, or error conditions.
The underlying open62541 C-library adopts this model, returning a status code (@ref UA_StatusCode) from any function that may encounter an error. In open62541pp, the @ref UA_StatusCode is wrapped as opcua::StatusCode and enhanced with utility functions for easier use.

The following two error handling methods are used, which are described below:
1. Exceptions in the high-level classes (`opcua` namespace)
2. Result return types for low-level functions (`opcua::services` namespace)

## Exceptions {#error_handling-exceptions}

Exceptions are the main used mechanism to report bad status codes in function calls.

The bases exception type opcua::BadStatus wraps a bad status code. The underlying status code can be accessed with opcua::BadStatus::code().
opcua::BadStatus::what() provides a human-readable name for the status code.

**Example:**

```cpp
const opcua::NodeId id(opcua::VariableId::Server_ServerStatus_CurrentTime);
opcua::Node node(client, id);
try {
    const auto var = node.readValue();  // may throw opcua::BadStatus
} catch (const opcua::BadStatus& e) {
    std::cerr << "Bad status: " << e.code() << ": " << e.what() << std::endl;
}
```

## Result type {#error_handling-result-type}

The @ref opcua::Result<T> type wraps both the result of a function and the corresponding status code.
Its design is similar to [`std::optional`](https://en.cppreference.com/w/cpp/utility/optional) or [`std::expected`](https://en.cppreference.com/w/cpp/utility/expected), making it suitable for scenarios that involve error handling without exceptions.  The key difference from `std::expected` is that the status code is always present, while the result is only available if the status code indicates success.
This approach avoids the performance overhead of exceptions and enhances clarity in function signatures. It also supports finer control over error management, especially in [asynchronous contexts](#async_model).

**Key functions:**

- opcua::Result<T>::value returns the value or throws an opcua::BadStatus exception if the status code is bad
- opcua::Result<T>::valueOr returns the value or the provided default value if the status code is bad
- opcua::Result<T>::code returns the status code directly

**Example:**

```cpp
const opcua::NodeId id(opcua::VariableId::Server_ServerStatus_CurrentTime);
opcua::Result<opcua::Variant> result = opcua::services::readValue(client, id);
if (result.code().isBad()) {
    std::cerr << "Bad status: " << result.code().get() << ": " << result.code().name() << std::endl;
} else {
    const opcua::Variant value = result.value();
}
```

## Error propagation is user callbacks

Since callbacks are executed within the open62541 C event loop, which does not support exceptions, any exceptions thrown inside user-defined callbacks are automatically caught and stored by opcua::detail::ExceptionCatcher. These exceptions are then propagated to the opcua::Client::run() and opcua::Server::run() functions, ensuring they can be handled properly.

**Example:**

```cpp
opcua::Client client;

// define callback that throws an exception
client.onSessionActivated([] { throw std::runtime_error("User exception!"); });

client.connect("opc.tcp://localhost:4840");
try {
    client.run();  // exceptions in callbacks are propagated to the run call
} catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
}
```
