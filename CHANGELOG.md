# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

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

[unreleased]: https://github.com/open62541pp/open62541pp/compare/v0.3.0...HEAD
[0.3.0]: https://github.com/open62541pp/open62541pp/releases/tag/v0.3.0
[0.2.0]: https://github.com/open62541pp/open62541pp/releases/tag/v0.2.0
[0.1.0]: https://github.com/open62541pp/open62541pp/releases/tag/v0.1.0
