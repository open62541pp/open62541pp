# Asynchronous model {#async_model}

@tableofcontents

Open62541pp adapts the well-proven [asynchronous model of (Boost) Asio](https://think-async.com/asio/asio-1.28.0/doc/asio/overview/model.html). A key goal of Asio's asynchronous model is to support multiple composition mechanisms. This is achieved via completion tokens.

## Completion tokens

Open62541pp accepts completion tokens as the final argument of asynchronous operations.

```cpp
// Function signature of the completion handler: void(opcua::Result<opcua::Variant>&)
template <typename CompletionToken = opcua::DefaultCompletionToken>
auto opcua::services::readValueAsync(
    opcua::Client& client,
    const opcua::NodeId& id,
    CompletionToken&& token = opcua::DefaultCompletionToken()
);
```

Following completion tokens can be used (and described in more detail below):

- A callback function (object) with the expected completion signature
- `opcua::useFuture` to return a future object `std::future<opcua::Result<T>>`
- `opcua::useDeferred` to return a callable for deferred execution
- `opcua::useDetached` to detach the asynchronous operation
- Custom tokens by providing template specializations for `opcua::AsyncResult`

### Callback function

If the user passes a function object as the completion token, the asynchronous operation behaves as previously described: the operation begins, and when the operation completes the result is passed to the callback. The callback function must match the expected signature:

```cpp
void(opcua::Result<T>);   // for void and non-void, trivially-copyable value types
void(opcua::Result<T>&);  // for non-void value types
```

```cpp
opcua::services::readValueAsync(
    client,
    id,
    [](opcua::Result<opcua::Variant>& result) {
        // ...
    }
);
```

The callback is executed within the client's or server's event loop. Please make sure not to block the event loop. Waiting for asynchronous results within the callback will block the further execution of the event loop.

### Future completion token

The special token `opcua::useFuture` can be passed as completion token to return a future object `std::future<opcua::Result<T>>`.

```cpp
std::future<opcua::Result<opcua::Variant>> future = opcua::services::readValueAsync(client, id, opcua::useFuture);
auto value = future.get().value();  // throws an exception if an error occurred
```

### Deferred completion token

The token `opcua::useDeferred` is used to indicate that an asynchronous operation should return a function object to lazily launch the operation.

```cpp
auto func = opcua::services::readValueAsync(client, id, opcua::useDeferred);

// start operation with provided completion token
func([](auto&& result) {
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

template <typename T>
struct AsyncResult<PrintResultToken, T> {
    template <typename Initiation, typename CompletionHandler, typename... Args>
    static void initiate(Initiation&& initiation, PrintResultToken, Args&&... args) {
        std::invoke(
            std::forward<Initiation>(initiation),
            [](opcua::Result<T>& result) {
                std::cout << "Async operation completed: code=" << result.code() << ", value=" << result.value() << std::endl;
            },
            std::forward<Args>(args)...
        );
    }
};

}
```

The trait's `opcua::AsyncResult::initiate` member function is called with three arguments:
1. A function object that launches the async operation (initiating function)
2. A concrete completion handler with the signature `void(opcua::Result<T>)` or `void(opcua::Result<T>&)`
3. Any additional arguments for the function object

Please have a look at implementations in @ref async.hpp for further details.
