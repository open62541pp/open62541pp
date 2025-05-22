# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.19.0] - 2025-05-22

### Added

- Comparison overloads for registered types (#577)
- Async operations to offload method calls to worker thread (#590)
- `String` conversion for all string-like types (#595)
- `DataTypeBuilder::addField` with manual offsets (#597)
- `MethodCallback` with session, methodId and objectId (#601)
- Policy-based type handling within `Wrapper` (#607)
- Support return-by-value overloads in `TypeConverter` (#608)
- Traits `IsWrapper`, `IsRegistered`, `IsConvertible` (#621)
- Performance optimizations:
  - Reuse log buffer (#604)
  - Move-aware `Variant` construction and assignment (#614)
  - Ref-qualified `Variant::to` (#616)

### Changed

- Use `String` instead of `std::string`, avoid `<string>` include (#581, #583, #623)
- Deprecate specialized `Node::readValue*` and `Node::writeValue*` functions (#584)
- Deprecate specialized `VariableAttributes::setValue` functions (#585)
- Remove deprecations ≤ v0.17.0 (#591, #592, #593, #594)
- Deprecate `TypeWrapper`, use `WrapperNative` alias instead (#607)

### Fixed

- Trim null terminator of string literals in `char[N]` `TypeConverter` (#599)
- Confusing log output when creating `Client`/`Server` with non-default config (#603)
- Access control with v1.4 (#612)
- Move only owned scalars from `Variant` (#615)
- Strip empty array sentinel from `Variant` data pointer (#617)

## [0.18.0] - 2025-04-08

### Added

- Free `toString` function for all native/wrapper types (#520)
- Optionally move ownership of `ValueCallback` and `DataSource` to `Server` (#544)
- `Guid::parse` (#545)
- `NodeId::parse`, `ExpandedNodeId::parse` (#546)
- Load nodesets from `NodeSet2.xml` files at runtime 🔥 (#551)
- Construct `Client`/`Server` from native instances (#558)
- `DataTypeMember` wrapper (#564)
- `ApplicationType` enum (#566)
- `ApplicationDescription` constructor (#566)
- `ClientConfig::addCustomDataTypes` and `ServerConfig::addCustomDataTypes` (#569)

### Changed

- Remove ostream overloads for `Guid` and `XmlElement` (#560)
- Remove `ByteString` <-> `std::string_view` equality operator overloads (#561)

### Fixed

- Unreachable code in `NodeId::identifierIf` (#540)
- Deallocation failure in read/write requests (#552)
- `Node::writeValueArray` for `std::vector<bool>` (#553)
- Check if server runs before calling `UA_Server_run_shutdown` (#556)
- Store `typeName` and `memberName` in `DataType` wrapper (#562, #563)

## [0.17.0] - 2025-01-18

### Added

- [`IntegerId`](https://reference.opcfoundation.org/Core/Part4/v105/docs/7.19) alias for `uint32_t` (#446)
- Add DataAccess wrapper types (#451)
- Define a subscription inactivity callback via `Client::onSubscriptionInactive` (#456)
- Update open62541 to v1.3.15 (#463)
- `NumericRange` as wrapper type (#481)
- Support string conversions with custom traits and allocators (#501)
- `Span::at` for element access with bounds checking (#502)
- `ExtensionObject::encodedBinary` and `ExtensionObject::encodedXml` (#506)
- `String`/`XmlElement` assignment operator for `const char*` and `string_view` (#507)
- Mark `Result` class as `nodiscard` (#513)
- `Session::context` to access session context (#531)

### Changed

- Simplified `Variant` interface:

  - Universal `Variant` constructor for scalars/arrays (#384, #495, #497)
  - Universal `Variant::assign` function for scalars/arrays, replacing `Variant::setScalar*`/`Variant::setArray*` functions (#496, #510)
  - Assignment operator for scalars/arrays (#503)
  - `Variant::to<T>` function to retrieve/convert scalars/arrays replacing `Variant::getScalar*`/`Variant::getArray*` functions (#492)

  ```cpp
  int value = 11;

  opcua::Variant var(value);   // copy
  opcua::Variant var(&value);  // no copy

  var.assign(value);           // copy
  var.assign(&value);          // no copy
  var = value;                 // copy
  var = &value;                // no copy

  int& valueRef = var.scalar<int>();
  int valueCopy = var.to<int>();
  ```

- Remove `crypto` namespace (#445)
- Move types and NodeIds of http://opcfoundation.org/UA/ (namespace zero) into inline namespace `ua` (#454)
- Remove `maxReferences` parameter from `services::browseAll` (#457)
- Improved `NodeId` identifier getter (#447)
- Remove `get` prefix of getter member functions (#459)
- Rename `isEmpty` member functions to `empty` (#499)
- Universal `ExtensionObject` constructor (#504)
- `ValueCallback` as abstract base class exposing all parameters (#522)
- `DataSource` as abstract base class exposing all parameters (#525)
- Create `Subscription` via constructor (#533)

### Fixed

- Config include in `deprecated.hpp` (#452)
- `Result` constructor with value and bad status code (#460)
- Thread-safe session registry (#486)
- Avoid redefinition of `NOMINMAX` macro for Windows systems (#523)
- Fix undefined behavior in `detail::copyArray` (#532)

## [0.16.0] - 2024-11-13

### Added

- Update open62541 to v1.3.14 (#375)
- `ClientConfig` and `ServerConfig` wrapper classes that can be passed to `Client`/`Server` constructor (#377)
- `asWrapper` overloads for `UA_Client*` and `UA_Server*` (#379)
- `ServerConfig::setBuildInfo` (#386)
- Async support for Subscription service set (#419)
- Async support for MonitoredItem service set (#421)
- Async `Node` functions (#434)
- `Client::connectAsync`/`Client::disconnectAsync` (#429)
- Example `client_async` (#430)

### Changed

- Deprecate config setters in `Client` and `Server` class, use member function `config()` instead (#377)
- Don't revise subscription parameters (#392)
- Don't revise monitoring parameters (#393)
- Return raw responses or results from functions in the `services` namespace for further diagnostics:
  - Return `CreateSubscriptionResponse` from `services::createSubscription` overloads (#394)
  - Return `ModifySubscriptionResponse` from `services::modifySubscription` overloads (#394)
  - Return `CallMethodResult` from `services::call(Async)`/`Node::call(Async)` (#395)
  - Return `MonitoredItemCreateResult` from `services::createMonitoredItemDataChange`/`*Event` (#401)
  - Return `MonitoredItemModifyResult` from `services::modifyMonitoredItem` (#404)
  - Return `BrowseResult` from `services::browse(Async)`/`services::browseNext(Async)` overloads (#406)
  - Return `BrowsePathResult` from `services::translateBrowsePathToNodeIds(Async)`/`services::browseSimplifiedBrowsePath(Async)` (#407)
- Return `StatusCode` instead of `Result<void>` (#410)
- Remove default function arguments in `services` namespace because it doesn't work well with the async functions (completion token as last argument) (#424)

### Fixed

- Move constructor and move assignment of `Client` and `Server` (#373)
- Build with minimal open62541 options (#433)

## [0.15.0] - 2024-10-01

### Added

- Read `DataTypeDefinition` attribute from data type nodes (#145)
- Client inactivity callback (#304)
- Create subscription with `StatusChangeNotificationCallback` (#309)
- Raw Subscription services (#310)
- Raw MonitoredItem services (#312)
- Version macros `UAPP_VERSION`, `UAPP_VERSION_MAJOR`, `UAPP_VERSION_MINOR`, `UAPP_VERSION_PATCH` (#316)
- `Client::setUserIdentityToken` (#325)
- STL-compatible interface for `String`, `ByteString`, `XmlElement` (#330)
- Construct `Guid` from `std::array<uint8_t, 16>` (#332)
- Compatibility for open62541 v1.4 (#341)
- Update open62541 to v1.3.12 (#342)
- Copyable/movable `Node`, `Session`, `Event`, `Subscription`, `MonitoredItem` (#344)

### Changed

- Remove all deprecated functions/types (#305, 308)
- Remove special `Server` overloads in MonitoredItem service set (#306)
- Deprecate `Client::get*Node`, `Server::get*Node`, use `Node` constructor instead (#320)
- Deprecate `Server::createEvent`, use `Event` constructor instead (#321)
- Deprecate `Client::connect` with `Login`, use `Client::setUserIdentityToken` instead (#326)
- Deprecate `String::get()`, `ByteString::get()` and `XmlElement::get()`, use `static_cast<std::string_view>(...)` instead (#328, #329, #335)
- Rename `NumericRange::get` -> `NumericRange::dimensions`
- Remove `ByteString::fromFile`, `ByteString::toFile` (#337)
- Rename `Logger` -> `LogFunction` (#355, #360)
- Remove `log(...)` overloads (#356)
- New header file structure (#348, #353): The header file structure became quite complex and tangled over time. The new structure is similar to open62541. All headers are now snake case with the extension `.hpp`. When coming from open62541, usually you just have to make a small adjustment in your includes:
  ```diff
  - #include <open62541/server.h>
  + #include <open62541pp/server.hpp>
  - #include <open62541/plugin/accesscontrol_default.h>
  + #include <open62541pp/plugin/accesscontrol_default.hpp>
  ```
  We tried to keep the migration as smooth as possible. The old headers are generated via CMake, include the new header and raise a compiler warning.
  The Python script [`tools/update_deprecated_includes.py`](https://github.com/open62541pp/open62541pp/blob/master/tools/update_deprecated_includes.py) can be used to automatically replace the includes in your project.
  <details>
    <summary>Summary of the <code>#include</code> changes</summary>
    <table>
      <thead>
        <tr>
          <th>Old Header File(s)</th>
          <th>New Header File(s)</th>
        </tr>
      </thead>
      <tbody>
        <tr>
          <td><code>open62541pp/plugins/PluginAdapter.h</code></td>
          <td><code>open62541pp/plugin/pluginadapter.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/services/Attribute_highlevel.h</code></td>
          <td><code>open62541pp/services/attribute_highlevel.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/services/Attribute.h</code></td>
          <td><code>open62541pp/services/attribute.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/services/Method.h</code></td>
          <td><code>open62541pp/services/method.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/services/MonitoredItem.h</code></td>
          <td><code>open62541pp/services/monitoreditem.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/services/NodeManagement.h</code></td>
          <td><code>open62541pp/services/nodemanagement.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/services/Subscription.h</code></td>
          <td><code>open62541pp/services/subscription.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/services/View.h</code></td>
          <td><code>open62541pp/services/view.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/services/services.h</code></td>
          <td><code>open62541pp/services/services.hpp</code></td>
        </tr>
        <tr>
          <td>
            <code>open62541pp/types/Builtin.h</code><br>
            <code>open62541pp/types/DataValue.h</code><br>
            <code>open62541pp/types/DateTime.h</code><br>
            <code>open62541pp/types/ExtensionObject.h</code><br>
            <code>open62541pp/types/NodeId.h</code><br>
            <code>open62541pp/types/Variant.h</code>
          </td>
          <td><code>open62541pp/types.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/types/Composed.h</code></td>
          <td><code>open62541pp/types_composed.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/AccessControl.h</code></td>
          <td>
            <code>open62541pp/plugin/accesscontrol.hpp</code>
            <code>open62541pp/plugin/accesscontrol_default.hpp</code>
          </td>
        </tr>
        <tr>
          <td><code>open62541pp/async.h</code></td>
          <td><code>open62541pp/async.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/Bitmask.h</code></td>
          <td><code>open62541pp/bitmask.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/Client.h</code></td>
          <td><code>open62541pp/client.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/Common.h</code></td>
          <td><code>open62541pp/common.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/Config.h</code></td>
          <td><code>open62541pp/config.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/Crypto.h</code></td>
          <td><code>open62541pp/plugin/create_certificate.hpp</code></td>
        </tr>
        <tr>
          <td>
            <code>open62541pp/DataType.h</code><br>
            <code>open62541pp/DataTypeBuilder.h</code>
          </td>
          <td><code>open62541pp/datatype.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/ErrorHandling.h</code></td>
          <td><code>open62541pp/exception.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/Event.h</code></td>
          <td><code>open62541pp/event.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/Logger.h</code></td>
          <td>
            <code>open62541pp/plugin/log.hpp</code><br>
            <code>open62541pp/plugin/log_default.hpp</code>
          </td>
        </tr>
        <tr>
          <td><code>open62541pp/MonitoredItem.h</code></td>
          <td><code>open62541pp/monitoreditem.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/Node.h</code></td>
          <td><code>open62541pp/node.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/NodeIds.h</code></td>
          <td><code>open62541pp/nodeids.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/open62541pp.h</code></td>
          <td><code>open62541pp/open62541pp.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/Result.h</code></td>
          <td><code>open62541pp/result.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/Server.h</code></td>
          <td><code>open62541pp/server.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/Session.h</code></td>
          <td><code>open62541pp/session.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/Span.h</code></td>
          <td><code>open62541pp/span.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/Subscription.h</code></td>
          <td><code>open62541pp/subscription.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/TypeConverter.h</code></td>
          <td><code>open62541pp/typeconverter.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/TypeRegistry.h</code></td>
          <td><code>open62541pp/typeregistry.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/TypeRegistryNative.h</code></td>
          <td><code>open62541pp/typeregistry_generated.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/TypeWrapper.h</code></td>
          <td><code>open62541pp/typewrapper.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/ValueBackend.h</code></td>
          <td><code>open62541pp/plugin/nodestore.hpp</code></td>
        </tr>
        <tr>
          <td><code>open62541pp/Wrapper.h</code></td>
          <td><code>open62541pp/wrapper.hpp</code></td>
        </tr>
      </tbody>
    </table>
  </details>

### Fixed

- Type error in highlevel attribute async read functions (#303)
- Add missing `<exception>` include for `std::exception_ptr` (#317)
- Clang-tidy v18 errors (#343)

## [0.14.0] - 2024-07-18

### Added

- Update open62541 to v1.3.11 (#294)
- Define `enum` as `Bitmask` by function overload (#295)

### Fixed

- Trigger value change in `server_datasource` example (#282)
- Linking shared library on Windows (#290)
- Stack overflow in session registry when running from different translation units (#292)

## [0.13.0] - 2024-04-27

### Added

- Async functions in `services` namespace (client only), adapting the well-proven [asynchronous model of (Boost) Asio](https://think-async.com/asio/asio-1.28.0/doc/asio/overview/model.html) (#111)
  - Example `client_method_async`
- `Result<T>` class to encapsulate both the status code and the result of type `T`, similar to `std::expected` (#207, #264, #276)
- Raw `services::call` functions (#215)
- `NamespaceIndex` alias for `uint16_t` (#219)
- Symmetric implementation of `services::createMonitoredItemDataChange`, `services::deleteMonitoredItem` for `Server` and `Client` (#238)
- Deprecate high-level `DataChangeCallback<T>` and `EventCallback<T>`, use `DataChangeNotificationCallback` and `EventNotificationCallback` instead and create `MonitoredItem<T>` on demand (#245)
- Subscription service request/response wrapper classes (#270)
- Update open62541 to v1.3.10 (#278)

### Changed

- Move-only `Client` and `Server` (#211)
- Rename `MonitoringParameters` -> `MonitoringParametersEx` to avoid shadowing of OPC UA type [`MonitoringParameter`](https://reference.opcfoundation.org/Core/Part4/v104/docs/7.16) (#214)
- Remove logger utility functions `getLogLevelName`, `getLogCategoryName` (#221)
- Change return type of `AccessControlBase::getUserTokenPolicies()` to `Span<Span<UserTokenPolicy>` (#223)
- Deprecate `TypeWrapper::TypeWrapperBase` alias, use `TypeWrapper::TypeWrapper` instead (#253)
- Return `ExtensionObject` encoded members by pointer, not optional (#254)
- Strip `get` prefix of simple getters, deprecate old member functions (#267)
- Use `Result<T>` return type for all functions in `services` namespace (#275)

### Fixed

- Error handling in `client_subscription` example (#203)
- Compare `DataType` only by `typeId` (#217)
- `Variant::isArray` check (#274)

## [0.12.0] - 2024-02-10

### Added

- Raw `NodeManagement` functions in `opcua::services` (#121, #124)
- Raw `View` functions in `opcua::services` (#125)
- Pass `NodeId` identifier by value in constructors, mark as noexcept (#133)
- `TypeRegistry<T>` to derive the corresponding `UA_DataType` object from template types.
  Custom data types can be registered with template specializations. (#136)
- Check `Variant` data type by template type, e.g. `var.isType<int>()` (#139)
- Update open62541 to v1.3.9 (#140)
- Pass custom logger to constructor of `Server` and `Client` (#150, #155)
- New function `void throwIfBad(UA_StatusCode)` (#153)
- New function `UA_DataType& getDataType<T>()` (#154)
- `AccessLevel`, `WriteMask` and `EventNotifier` enum classes (#163)
- `Bitmask<T>` type to allow both enum classes and native enums/ints to define bitmasks (#163)
- Propagate callback exceptions to event loop run method (#179)
- Policy template parameter for `Variant` factory functions `Variant::fromScalar`, `Variant::fromArray` (#174)
  - `VariantPolicy::Copy`: Store copy of scalar/array inside the variant (**default**)
  - `VariantPolicy::Reference`: Store reference to scalar/array inside the variant
  - `VariantPolicy::ReferenceIfPossible`: Favor referencing but fall back to copying if necessary

### Changed

- Deprecate `TypeIndexList` (`TypeConverter::ValidTypes`) because it's not required anymore.
  Just remove `TypeConverter<T>::ValidTypes` from your template specializations.
  The `UA_DataType` is retrieved from the `TypeRegistry<NativeType>` specialization. (#136)
- Passing an empty function to `setLogger` does nothing (#150)
- Deprecate `Type` enum (#157)
- Remove implicit conversion from `XmlElement` to `std::string_view` (#159)
- Remove deprecated functions to read/write attributes (#166):
  - `Node::readDataValue(DataValue&)`, use `Node::readDataValue()` instead
  - `Node::readValue(DataValue&)`, use `Node::readValue()` instead
  - `Node::readScalar<T>()`, use `Node::readValueScalar<T>()` instead
  - `Node::readArray<T>()`, use `Node::readValueArray<T>()` instead
  - `services::readDataValue(T&, const NodeId&, DataValue&)`, use `services::readDataValue(T&, const NodeId&)` instead
  - `services::readValue(T&, const NodeId&, Variant&)`, use `services::readValue(T&, const NodeId&)` instead
- Use `Bitmask<Enum>` instead of integers (#163)
  - Use `Bitmask<AccessLevel>` instead of `uint8_t` with implicit conversions to/from `uint8_t`
  - Use `Bitmask<WriteMask>` instead of `uint32_t` with implicit conversions to/from `uint32_t`
  - Virtual function `AccessControlBase::getUserRightsMask` returns `Bitmask<WriteMask>` instead of `uint32_t`
  - Virtual function `AccessControlBase::getUserAccessLevel` returns `Bitmask<AccessLevel>` instead of `uint8_t`
  - Implicit conversion from `Bitmask<T>` to underlying integer are deprecated and will be made explicit in the future.
    Please migrate to `Bitmask<T>::get()`.
- Rename DataValue methods from has/get/setStatusCode to has/get/setStatus (#177)
- `Variant` factory functions `Variant::fromScalar` and `Variant::fromArray` will create copies by default.
  Choose other policies, `VariantPolicy::Reference` or `VariantPolicy::ReferenceIfPossible`, if needed. (#174)

### Fixed

- `setLogger` memory leak (#127)
- `DataTypeBuilder::createEnum` (#143)
- Underlying data type of enums (#152)
- Add missing `noexcept` specifiers (#160)
- Internal deletion of native arrays (#181)

## [0.11.0] - 2023-11-01

### Changed

- Return output `NodeId` from node creation functions (#118):
  - `services::addObject`
  - `services::addFolder`
  - `services::addVariable`
  - `services::addProperty`
  - `services::addMethod`
  - `services::addObjectType`
  - `services::addVariableType`
  - `services::addReferenceType`
  - `services::addDataType`
  - `services::addView`

### Fixed

- Return _true_ output `NodeId` from `Node::add*` methods to allow random node ids with e.g. `NodeId(1, 0)` (#118)

## [0.10.0] - 2023-10-28

### Added

- Event filter (#101)
  - Add filter wrapper types: `ContentFilterElement`, `ContentFilter`, `DataChangeFilter`, `EventFilter`, `AggregateFilter`
  - Example how to filter events client-side: `client_eventfilter`
- Bundle multiple read/write requests (#103)
  - Add `DiagnosticInfo `wrapper
  - Add missing request/response wrapper
  - Add `services::read` function overload for bundled read requests
  - Add `services::write` function overload for bundled write requests
- Non-const getter for composed types (#106)
- GCC 7 support (#110)
- Update open62541 to v1.3.8 (#115)

## [0.9.0] - 2023-08-29

### Added

- `Span` class to pass and return open62541 arrays without copy and use them with the standard library algorithms (#86)
  - `Span` objects just hold two members: the pointer to `T` and the size, so they are lightweight and trivially copyable
  - Spans are views of contiguous sequences of objects, similar to [`std::span`](https://en.cppreference.com/w/cpp/container/span) in C++20. Differences to `std::span`:
    - Constructor with `std::initializer_list<T>`
    - Explicit conversion to `std::vector<T>`
- `Node::exists` method with most efficient implementation to check node existence (#92)
- Conversions from `string_view`, `const char*`, `char[N]` to `UA_String` (#98)
- `Event` class to create and trigger events (#99)
- `Server::createEvent` method (#99)
- Equality operator overloads for `Session`
- Add examples:
  - `events/server_events` (#99)

### Changed

- Use `Span` to represent open62541 arrays (#86)
  - `EventNotificationCallback` signature:
    - `eventFields`: `Span<const Variant>` instead of `const std::vector<Variant>&`
  - `MethodCallback` signature:
    - `input`: `Span<const Variant>` instead of `const std::vector<Variant>&`
    - `output`: `Span<Variant>` instead of `std::vector<Variant>&`
  - Return (no-copy) array as `Span<T>` from `Variant` methods:
    - `Span<const uint32_t> getArrayDimensions() const`
    - `Span<T> getArray()`
    - `Span<const T> getArray() const`
  - Composed wrapper return `Span<T>` instead of `std::vector<T>` for array attributes:
    - `ApplicationDescription::getDiscoveryUrls()`
    - `EndpointDescription::getUserIdentityTokens()`
    - `BrowseResult::getReferences()`
    - `RelativePath::getElements()`
    - `Argument::getArrayDimensions()`
  - Remove getters for array size, e.g.:
    - `DataType::getMembersSize()`, use `DataType::getMembers().size()` instead
- Deprecate type-erased versions of `Variant::getScalar()` and `Variant::getArray()`, use `Variant::data()` instead (#86)
- Remove existence check from `Node` constructor (#92)

## [0.8.0] - 2023-08-21

### Added

- `services::browseRecursive` as a wrapper for `UA_Server_browseRecursive` (#91)
- Implicit conversion from `String` / `XmlElement` to `std::string_view` (#93)
- Convenience functions to read/write object properties (#96):
  - `Node::readObjectProperty`
  - `Node::writeObjectProperty`
- Delete reference (#97):
  - `services::deleteReference`
  - `Node::deleteReference`

### Changed

- Return `Variant`/`DataValue` by value from read functions (no performance penalty), deprecate old functions (#94):

  - `void services::readDataValue(T&, const NodeId&, DataValue&)` -> `DataValue services::readDataValue(T&, const NodeId&)`
  - `void services::readValue(T&, const NodeId&, Variant&)` -> `Variant services::readValue(T&, const NodeId&)`
  - `void Node::readDataValue(DataValue&)` -> `DataValue Node::readDataValue()`
  - `void Node::readValue(Variant&)` -> `Variant Node::readValue()`

- Specific naming of methods to read/write scalar/array values, deprecate old methods (#95):
  - `Node::readScalar` -> `Node::readValueScalar`
  - `Node::readArray` -> `Node::readValueArray`
  - `Node::writeScalar` -> `Node::writeValueScalar`
  - `Node::writeArray` -> `Node::writeValueArray`

## [0.7.0] - 2023-08-15

### Added

- Set variable node value data source with `Server::setVariableNodeValueBackend` (#65)
- Missing node management functions (#77):
  - `Node::addReferenceType` / `services::addReferenceType`
  - `Node::addDataType` / `services::addDataType`
  - `Node::addView` / `services::addView`
- `Variant` methods for custom types (#78):
  - `Variant::getDataType`
  - `Variant::getScalar` without template type (returns `void*`)
  - `Variant::getArray` without template type (returns `void*`)
  - `Variant::setScalar` overload with custom `UA_DataType`
  - `Variant::setScalarCopy` overload with custom `UA_DataType`
  - `Variant::setArray` overload with custom `UA_DataType`
  - `Variant::setArrayCopy` overload with custom `UA_DataType`
- `Variant::fromScalar` / `Variant::fromArray` overloads with custom data type (#78)
- `StatusCode` class (#79)
- Integration of custom data types (#76):
  - `DataType` wrapper class
  - `DataTypeBuilder` to generate `UA_DataType` definition for custom types
  - `Variant::isType(const UA_DataType&)` overload
  - `Server::setCustomDataTypes` / `Client::setCustomDataTypes`
  - `Variant::set*` and `Variant::from*` methods with custom data type parameter
- `NodeId::isNull` method
- `ExpandedNodeId::hash` method
- Custom access control (#74):
  - `Session` class
  - `AccessControlBase` base class and `AccessControlDefault` implementation
  - `Server` methods:
    - `Server::setAccessControl`
    - `Server::getSessions`
- Deduce data type from template type (#84).
  Provide overloads to deduce the data type id of Variable or VariableType nodes from the template type `T`:
  - `Node::writeDataType<T>()`
  - `VariableAttributes::setDataType<T>()`
  - `VariableTypeAttributes::setDataType<T>()`
- `std::hash` specialization for `NodeId` and `ExpandedNodeId` (#90)
- Add examples:
  - `typeconversion`
  - `custom_datatypes/*` (#76)
  - `server_accesscontrol` (#74)

### Changed

- Remove `type`/`typeIndex` template parameters (#83).
  Infer data type in function body. Ambiguous types have to be specified in overloaded methods with `UA_DataType` parameter
- Remove `Server::setLogin` method (#74).
  Use `Server::setAccessControl` instead with `AccessControlDefault(bool allowAnonymous = true, std::vector<Login> logins = {})` instead.

### Fixed

- Catch exceptions in callbacks
- Add include guard to `Crypto.h` (#80)
- Clang-tidy fixes (#82)
- MSVC warnings

## [0.6.0] - 2023-07-29

### Added

- Variable node value callbacks with `Server::setVariableNodeValueCallback` (#63)
- Encryption (#64)
  - `Client` constructor with `certificate`, `privateKey`, `trustList` and `revocationList`
  - `Server` constructor with `certificate`, `privateKey`, `trustList`, `issuerList` and `revocationList`
  - `crypto::createCertificate` function to create private keys and certificates
  - `ByteString::fromFile` / `ByteString::toFile` to load and save certificates or private keys
- `NumericRange` type (#73)
- Equality overloads for `String`/`ByteString` and `std::string_view`
- Fluent `Node` interface (#75)
- Examples
  - `server_valuecallback` (#63)

### Changed

- Hide `TypeWrapper::getDataType` method
- Initial node attributes as first default parameter (usually after `browseName`) (#61):
  - `Node::addFolder` / `services::addFolder`
  - `Node::addObject` / `services::addObject`
  - `Node::addVariable` / `services::addVariable`
  - `Node::addObjectType` / `services::addObjectType`
  - `Node::addVariableType` / `services::addVariableType`
  - `Node::addMethod` / `services::addMethod`
  - `Node::addFolder` / `services::addFolder`
  - `Node::addFolder` / `services::addFolder`
- `std::string` only convertible to/from `opcua::String`

### Fixed

- Typos (#67)
- CMake code in README (#72)

## [0.5.0] - 2023-07-14

### Added

- `asNative` function to cast wrapper to native objects
- Add method nodes (#55)
  - `services::addMethod`
  - `Node::addMethod`
- Call method (#55)
  - `services::call`
  - `Node::callMethod`
- Set client timeout with `Client::setTimeout` (#56)
- `Variant`:
  - Get/set wrapper types without copy (`Variant::getScalar`, `Variant::getArray`, `Variant::setScalar`, `Variant::setArray`)
  - `const` version of `Variant::getScalar`, `Variant::getArray`
- `log` function to generate log message with server's or client's logger
- Examples
  - `server_method` (#55)
  - `client_method` (#55)

### Fixed

- Build with `UA_ENABLE_SUBSCRIPTIONS` or `UA_ENABLE_METHODCALLS` disabled
- Export symbols of shared library on windows (#58)

## [0.4.1] - 2023-05-30

### Fixed

- Enforce new session in `client_subscription` example (#51)
- Incorrect `MonitoredItem` passed to data change and event callback (#53)

## [0.4.0] - 2023-05-17

### Added

- Subscription and MonitoredItem service set as free functions in namespace services (#45)
- High-level `Subscription<T>` and `MonitoredItem<T>` classes (#45)
- `Server`/`Client` methods to create and list subscriptions (#45):
  - `Server::createSubscription`
  - `Client::createSubscription`
  - `Client::getSubscriptions`
- Client methods:
  - `Client::isConnected`
  - `Client::runIterate` (#45)
  - `Client::run`
  - `Client::stop`
  - `Client::isRunning`
- Client state callbacks: `Client::onConnected`, `Client::onDisconnected`, `Client::onSessionActivated`, `Client::onSessionClosed` (#50)
- `ReadValueId` wrapper class (#45)
- `ExtensionObject` wrapper class (#48)
- `ByteString::fromBase64` and `ByteString::toBase64` methods
- `NodeId::toString`/`ExpandedNodeId::toString` method to encode NodeIds as strings, e.g. `ns=1;s=SomeNode`
- `BadDisconnect` exception for simpler handling of disconnects (#50)
- Examples:
  - `client_subscription` (#45, #50)

### Changed

- Update open62541 to v1.3.6 (#49)
- Use scoped enum `TimestampsToReturn` instead of `UA_TimestampsToReturn`
- Use scoped enum `AttributeId` instead of `UA_AttributeId`
- Don't return std::optional from `DataValue` getters
- Add `DataValue::has*` methods
- Remove `DataValue::getValuePtr` method

### Fixed

- Amalgamation support (#47)

## [0.3.0] - 2023-04-29

### Added

- Basic `Client` implementation (#33)
- Attribute service set with generic read/write functions:
  - `services::readAttribute`
  - `services::writeAttribute`
- View/browse service set:
  - `services::browse`
  - `services::browseNext`
  - `services::browseAll`
  - `services::translateBrowsePathToNodeIds`
  - `services::browseSimplifiedBrowsePath`
- `Node` methods for browsing:
  - `browseReferences`
  - `browseReferencedNodes`
  - `browseChildren`
  - `browseChild`
  - `browseParent`
- Missing functions to read/write attributes `UserWriteMask`, `IsAbstract`, `Symmetric`, `InverseName`, `UserAccessLevel` and `MinimumSamplingInterval`
- `asWrapper` function to cast native refs to wrapper refs (#30, #31)
- `DataValue::getValuePtr` method
- Static methods `DataValue::fromScalar` and `DataValue::fromArray`
- `DateTime::format` method
- `Server::getNamespaceArray` method
- `Server::runIterate` method, e.g. to run server in existing event loop
- `Guid::toString` and `Guid::random` method
- `DateTime::localTimeUtcOffset` and `DateTime::format` method
- `ostream` overloads for `String`, `Guid`, `XmlElement`
- CMake install target and config files (#38)
- Examples:
  - `client_minimal`
  - `client_connect`
  - `client_find_servers`
  - `client_browse`
  - `server_minimal`

### Changed

- Templated `Node` class (`Node<Server>`, `Node<Client>`) for client implementation (#32)
- Return reference from `ExpandedNodeId::getNodeId`
- Pass `LocalizedText` instead of members (`locale`, `text`) to `Node::writeDisplayName`, `Node::writeDescription`, `services::writeDisplayName`, `services::writeDescription` (#29)
- Remove `Server::getConfig` method
- Rename `Node::writeModellingRule` -> `Node::addModellingRule`
- Rename `Node::getChild` -> `Node::browseChild`
- Remove `TypeWrapper::getType` method
- Use `TypeIndex` instead of `Type` enum for `TypeConverter` to allow conversions of non-builtin types
- Return `Qualified` name from `readBrowseName` instead of `std::string`
- Rename `ReferenceType` enum -> `ReferenceTypeId` (#44)
- Use `NodeId` for `referenceType` function parameters (instead of enum to allow arbitrary references) (#44)
- Remove get methods of nested nodes from `Client`/`Server` class, use `getNode` instead (#44)
  - `Server::getObjectTypesNode()` -> `Server::getNode(ObjectId::ObjectTypesFolder)`
  - `Server::getVariableTypesNode()` -> `Server::getNode(ObjectId::VariableTypesFolder)`
  - `Server::getDataTypesNode()` -> `Server::getNode(ObjectId::DataTypesFolder)`
  - `Server::getReferenceTypesNode()` -> `Server::getNode(ObjectId::ReferenceTypesFolder)`
  - `Server::getBaseObjectTypeNode()` -> `Server::getNode(ObjectTypeId::BaseObjectType)`
  - `Server::getBaseDataVariableTypeNode()` -> `Server::getNode(VariableTypeId::BaseDataVariableType)`

### Fixed

- `TypeConverter::toNative` specialization for wrapper types
- `ModellingRule::Optional` enum value (#44)

## [0.2.0] - 2023-04-12

### Added

- Generic type conversions with `TypeConverter` struct specializations
- Free functions in services namespace as alternative to `Node` interface (#22)
- `Server` constructors with custom port and certificate
- Custom logger with `Server::setLogger`
- Example `server_instantiation`
- Missing core reference types to `ReferenceType` enum
- `Node` methods:
  - `Node::getNodeClass`
  - `Node::addReference`
  - `Node::getChild`
  - `Node::readValueRang`, `Node::writeValueRang`
  - `Node::readArrayDimensions`, `Node::writeArrayDimensions`
  - `Node::readDataValue`, `Node::writeDataValue`
  - `Node::writeModellingRule`
- New wrapper classes:
  - `XmlElement`
  - `ExpandedNodeId`
  - `DateTime`
  - `DataValue`
- Static methods `Variant::from*` for create variants from scalars and arrays (#21)

### Changed

- Update open62541 to v1.3.5 (#17, #19)
- Use generic `Node` class, remove specific node classes:
  `ObjectNode`, `VariableNode`, `MethodNode`, `ViewNode`, `DataTypeNode`, `ObjectTypeNode`, `VariableTypeNode`, `ReferenceTypeNode`, `EventTypeNode`
- Rename methods `Node::get*` -> `Node::read*` and `Node::set*` -> `Node::write*` (consistent with open62541 naming)
- Remove `type` argument for `Node::addVariable`, `Node::addProperty`, `Node::addVariableType`
- Rename `Node` methods to write scalars:
  - `Node::read` -> `Node::readScalar`
  - `Node::write` -> `Node::writeScalar`
- Make `TypeWrapper` destructor non-virtual (**caution**: don't implement destructors in derived classes)
- `NodeId` `namespaceIndex` as first constructor argument (**breaking change**)
- `LocalizedText` `locale` as first constructor argument (**breaking change**)
- Return `LocalizedText` from `Node::readDisplayName`, `Node::readDescription` methods
- Remove default `locale` parameter for `LocalizedText`
- Rename `Exception` -> `BadStatus`
- Use default open62541 attributes for `Node::add*`
- Use `ReferenceType::HasComponent` as default reference for child variable and object nodes
- Optional check if `Node` exists in constructor (#24)

### Removed

- `Variant` constructors to set scalars and arrays directly

### Fixed

- Dereference in `Variant::getArray` (#25)

## [0.1.0] - 2022-04-24

Initial public release

[unreleased]: https://github.com/open62541pp/open62541pp/compare/v0.19.0...HEAD
[0.19.0]: https://github.com/open62541pp/open62541pp/releases/tag/v0.19.0
[0.18.0]: https://github.com/open62541pp/open62541pp/releases/tag/v0.18.0
[0.17.0]: https://github.com/open62541pp/open62541pp/releases/tag/v0.17.0
[0.16.0]: https://github.com/open62541pp/open62541pp/releases/tag/v0.16.0
[0.15.0]: https://github.com/open62541pp/open62541pp/releases/tag/v0.15.0
[0.14.0]: https://github.com/open62541pp/open62541pp/releases/tag/v0.14.0
[0.13.0]: https://github.com/open62541pp/open62541pp/releases/tag/v0.13.0
[0.12.0]: https://github.com/open62541pp/open62541pp/releases/tag/v0.12.0
[0.11.0]: https://github.com/open62541pp/open62541pp/releases/tag/v0.11.0
[0.10.0]: https://github.com/open62541pp/open62541pp/releases/tag/v0.10.0
[0.9.0]: https://github.com/open62541pp/open62541pp/releases/tag/v0.9.0
[0.8.0]: https://github.com/open62541pp/open62541pp/releases/tag/v0.8.0
[0.7.0]: https://github.com/open62541pp/open62541pp/releases/tag/v0.7.0
[0.6.0]: https://github.com/open62541pp/open62541pp/releases/tag/v0.6.0
[0.5.0]: https://github.com/open62541pp/open62541pp/releases/tag/v0.5.0
[0.4.1]: https://github.com/open62541pp/open62541pp/releases/tag/v0.4.1
[0.4.0]: https://github.com/open62541pp/open62541pp/releases/tag/v0.4.0
[0.3.0]: https://github.com/open62541pp/open62541pp/releases/tag/v0.3.0
[0.2.0]: https://github.com/open62541pp/open62541pp/releases/tag/v0.2.0
[0.1.0]: https://github.com/open62541pp/open62541pp/releases/tag/v0.1.0
