# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- `asWrapper` function to cast native refs to wrapper refs (#30, #31)
- `DataValue::getValuePtr` method
- `Server::getNamespaceArray` method

### Changed

- Templated `Node` class (`Node<Server>`, `Node<Client>`) as preparation for client implementation (#32)
- Return reference from `ExpandedNodeId::getNodeId`
- Pass `LocalizedText` instead of members (`locale`, `text`) to `Node::writeDisplayName`, `Node::writeDescription`, `services::writeDisplayName`, `services::writeDescription` (#29)
- Remove `Server::getConfig` method

### Fixed

- `TypeConverter::toNative` specialization for wrapper types

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

[unreleased]: https://github.com/open62541pp/open62541pp/compare/v0.2.0...HEAD
[0.2.0]: https://github.com/open62541pp/open62541pp/releases/tag/v0.2.0
[0.1.0]: https://github.com/open62541pp/open62541pp/releases/tag/v0.1.0
