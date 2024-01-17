# Asynchronous model {#async_model}

@tableofcontents

Open62541++ adapts the well-proven [asynchronous model of (Boost) Asio](https://think-async.com/asio/asio-1.28.0/doc/asio/overview/model.html). A key goal of Asio's asynchronous model is to support multiple composition mechanisms. This is achieved via completion tokens.

## Completion tokens

Open62541++ accepts completion tokens as the final argument of asynchronous operations.

```cpp
// Function signature of the completion handler: void(opcua::StatusCode, opcua::Variant&)
template <typename CompletionToken = opcua::DefaultCompletionToken>
auto opcua::services::readValueAsync(
    opcua::Client& client,
    const opcua::NodeId& id,
    CompletionToken&& token = opcua::DefaultCompletionToken()
);
```

Following completion tokens can be used (and described in more detail below):

- A callback function (object) with the expected completion signature
- `opcua::useFuture` to return a future object `std::future<T>`
- `opcua::useDeferred` to return a callable for deferred execution
- `opcua::useDetached` to detach the asynchronous operation
- Custom tokens by providing template specializations for `opcua::AsyncResult`

### Callback function

If the user passes a function object as the completion token, the asynchronous operation behaves as previously described: the operation begins, and when the operation completes the result is passed to the callback. The callback function must match the expected signature:

```cpp
void(opcua::StatusCode code)              // for void result types
void(opcua::StatusCode code, T& result);  // for non-void result types
void(opcua::StatusCode code, T result);   // for non-void result types (trivially-copyable)
```

```cpp
opcua::services::readValueAsync(
    client,
    id,
    [](opcua::StatusCode code, opcua::Variant& result) {
        // ...
    }
);
```

The status code `opcua::StatusCode` needs to be checked to determine the validity of the result.
The callback is executed within the client's or server's event loop. Please make sure not to block the event loop with synchronous requests.

### Future completion token

The special token `opcua::useFuture` can be passed as completion token to return a future object `std::future<T>`.

```cpp
std::future<opcua::Variant> future = opcua::services::readValueAsync(client, id, opcua::useFuture);
auto result = future.get();  // throws an exception if an error occurred
```

### Deferred completion token

The token `opcua::useDeferred` is used to indicate that an asynchronous operation should return a function object to lazily launch the operation.

```cpp
auto func = opcua::services::readValueAsync(client, id, opcua::useDeferred);

// start operation with provided completion token
func([](auto code, auto&& result) {
    // ...
});
```

### Detached completion token

The token `opcua::useDetached` is used to indicate that an asynchronous operation is detached.
That is, there is no completion handler waiting for the operation's result.

```cpp
opcua::services::writeValueAsync(client, id, opcua::Variant::fromScalar(1));
// no way to check if the operation succeeded...
```

### Custom completion token

The `opcua::AsyncResult` trait is a customization point to define user-defined completion tokens via template specialization:

```cpp
struct PrintResultToken {};

namespace opcua {

template <typename Result>
struct AsyncResult<PrintResultToken, Result> {
    template <typename Initiation, typename CompletionHandler, typename... Args>
    static void initiate(Initiation&& initiation, PrintResultToken, Args&&... args) {
        std::invoke(
            std::forward<Initiation>(initiation),
            [](StatusCode code, auto& result) {
                std::cout << "Async operation completed: code=" << code << ", result=" << result << std::endl;
            },
            std::forward<Args>(args)...
        );
    }
};

}
```

The `Result` template parameter specifies the result type of the asynchronous operation.
If `Result` is `void`, the expected signature of the completion handler is `void(opcua::StatusCode)`, otherwise it is `void(opcua::StatusCode, Result&)`.

The trait's `opcua::AsyncResult::initiate` member function is called with three arguments:
1. A function object that launches the async operation (initiating function)
2. A concrete completion handler with the signature `void(opcua::StatusCode, Result)`
3. Any additional arguments for the function object

Please have a look at implementations in @ref async.h for further details.
