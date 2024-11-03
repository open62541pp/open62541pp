/* ---------------------------------------------------------------------------------------------- */
/*                                   Generated - do not modify!                                   */
/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "open62541pp/services/attribute.hpp"

namespace opcua::services {

/**
 * Read the AttributeId::NodeId attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
Result<NodeId> readNodeId(T& connection, const NodeId& id) noexcept {
    return detail::readAttributeImpl<AttributeId::NodeId>(connection, id);
}

/**
 * @copydoc readNodeId
 * @param token @completiontoken{void(Result<NodeId>&)}
 * @return @asyncresult{Result<NodeId>}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readNodeIdAsync(
    Client& connection, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::NodeId>(
        connection, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::NodeClass attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
Result<NodeClass> readNodeClass(T& connection, const NodeId& id) noexcept {
    return detail::readAttributeImpl<AttributeId::NodeClass>(connection, id);
}

/**
 * @copydoc readNodeClass
 * @param token @completiontoken{void(Result<NodeClass>)}
 * @return @asyncresult{Result<NodeClass>}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readNodeClassAsync(
    Client& connection, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::NodeClass>(
        connection, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::BrowseName attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
Result<QualifiedName> readBrowseName(T& connection, const NodeId& id) noexcept {
    return detail::readAttributeImpl<AttributeId::BrowseName>(connection, id);
}

/**
 * @copydoc readBrowseName
 * @param token @completiontoken{void(Result<QualifiedName>&)}
 * @return @asyncresult{Result<QualifiedName>}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readBrowseNameAsync(
    Client& connection, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::BrowseName>(
        connection, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::BrowseName attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to write
 * @param browseName Value to write
 * @ingroup Write
 */
template <typename T>
StatusCode writeBrowseName(
    T& connection, const NodeId& id, const QualifiedName& browseName
) noexcept {
    return detail::writeAttributeImpl<AttributeId::BrowseName>(connection, id, browseName);
}

/**
 * @copydoc writeBrowseName
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeBrowseNameAsync(
    Client& connection,
    const NodeId& id,
    const QualifiedName& browseName,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::writeAttributeAsyncImpl<AttributeId::BrowseName>(
        connection, id, browseName, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::DisplayName attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
Result<LocalizedText> readDisplayName(T& connection, const NodeId& id) noexcept {
    return detail::readAttributeImpl<AttributeId::DisplayName>(connection, id);
}

/**
 * @copydoc readDisplayName
 * @param token @completiontoken{void(Result<LocalizedText>&)}
 * @return @asyncresult{Result<LocalizedText>}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readDisplayNameAsync(
    Client& connection, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::DisplayName>(
        connection, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::DisplayName attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to write
 * @param displayName Value to write
 * @ingroup Write
 */
template <typename T>
StatusCode writeDisplayName(
    T& connection, const NodeId& id, const LocalizedText& displayName
) noexcept {
    return detail::writeAttributeImpl<AttributeId::DisplayName>(connection, id, displayName);
}

/**
 * @copydoc writeDisplayName
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeDisplayNameAsync(
    Client& connection,
    const NodeId& id,
    const LocalizedText& displayName,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::writeAttributeAsyncImpl<AttributeId::DisplayName>(
        connection, id, displayName, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::Description attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
Result<LocalizedText> readDescription(T& connection, const NodeId& id) noexcept {
    return detail::readAttributeImpl<AttributeId::Description>(connection, id);
}

/**
 * @copydoc readDescription
 * @param token @completiontoken{void(Result<LocalizedText>&)}
 * @return @asyncresult{Result<LocalizedText>}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readDescriptionAsync(
    Client& connection, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::Description>(
        connection, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::Description attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to write
 * @param description Value to write
 * @ingroup Write
 */
template <typename T>
StatusCode writeDescription(
    T& connection, const NodeId& id, const LocalizedText& description
) noexcept {
    return detail::writeAttributeImpl<AttributeId::Description>(connection, id, description);
}

/**
 * @copydoc writeDescription
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeDescriptionAsync(
    Client& connection,
    const NodeId& id,
    const LocalizedText& description,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::writeAttributeAsyncImpl<AttributeId::Description>(
        connection, id, description, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::WriteMask attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
Result<Bitmask<WriteMask>> readWriteMask(T& connection, const NodeId& id) noexcept {
    return detail::readAttributeImpl<AttributeId::WriteMask>(connection, id);
}

/**
 * @copydoc readWriteMask
 * @param token @completiontoken{void(Result<Bitmask<WriteMask>>)}
 * @return @asyncresult{Result<Bitmask<WriteMask>>}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readWriteMaskAsync(
    Client& connection, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::WriteMask>(
        connection, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::WriteMask attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to write
 * @param writeMask Value to write
 * @ingroup Write
 */
template <typename T>
StatusCode writeWriteMask(T& connection, const NodeId& id, Bitmask<WriteMask> writeMask) noexcept {
    return detail::writeAttributeImpl<AttributeId::WriteMask>(connection, id, writeMask);
}

/**
 * @copydoc writeWriteMask
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeWriteMaskAsync(
    Client& connection,
    const NodeId& id,
    Bitmask<WriteMask> writeMask,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::writeAttributeAsyncImpl<AttributeId::WriteMask>(
        connection, id, writeMask, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::UserWriteMask attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
Result<Bitmask<WriteMask>> readUserWriteMask(T& connection, const NodeId& id) noexcept {
    return detail::readAttributeImpl<AttributeId::UserWriteMask>(connection, id);
}

/**
 * @copydoc readUserWriteMask
 * @param token @completiontoken{void(Result<Bitmask<WriteMask>>)}
 * @return @asyncresult{Result<Bitmask<WriteMask>>}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readUserWriteMaskAsync(
    Client& connection, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::UserWriteMask>(
        connection, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::UserWriteMask attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to write
 * @param userWriteMask Value to write
 * @ingroup Write
 */
template <typename T>
StatusCode writeUserWriteMask(
    T& connection, const NodeId& id, Bitmask<WriteMask> userWriteMask
) noexcept {
    return detail::writeAttributeImpl<AttributeId::UserWriteMask>(connection, id, userWriteMask);
}

/**
 * @copydoc writeUserWriteMask
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeUserWriteMaskAsync(
    Client& connection,
    const NodeId& id,
    Bitmask<WriteMask> userWriteMask,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::writeAttributeAsyncImpl<AttributeId::UserWriteMask>(
        connection, id, userWriteMask, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::IsAbstract attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
Result<bool> readIsAbstract(T& connection, const NodeId& id) noexcept {
    return detail::readAttributeImpl<AttributeId::IsAbstract>(connection, id);
}

/**
 * @copydoc readIsAbstract
 * @param token @completiontoken{void(Result<bool>)}
 * @return @asyncresult{Result<bool>}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readIsAbstractAsync(
    Client& connection, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::IsAbstract>(
        connection, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::IsAbstract attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to write
 * @param isAbstract Value to write
 * @ingroup Write
 */
template <typename T>
StatusCode writeIsAbstract(T& connection, const NodeId& id, bool isAbstract) noexcept {
    return detail::writeAttributeImpl<AttributeId::IsAbstract>(connection, id, isAbstract);
}

/**
 * @copydoc writeIsAbstract
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeIsAbstractAsync(
    Client& connection,
    const NodeId& id,
    bool isAbstract,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::writeAttributeAsyncImpl<AttributeId::IsAbstract>(
        connection, id, isAbstract, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::Symmetric attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
Result<bool> readSymmetric(T& connection, const NodeId& id) noexcept {
    return detail::readAttributeImpl<AttributeId::Symmetric>(connection, id);
}

/**
 * @copydoc readSymmetric
 * @param token @completiontoken{void(Result<bool>)}
 * @return @asyncresult{Result<bool>}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readSymmetricAsync(
    Client& connection, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::Symmetric>(
        connection, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::Symmetric attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to write
 * @param symmetric Value to write
 * @ingroup Write
 */
template <typename T>
StatusCode writeSymmetric(T& connection, const NodeId& id, bool symmetric) noexcept {
    return detail::writeAttributeImpl<AttributeId::Symmetric>(connection, id, symmetric);
}

/**
 * @copydoc writeSymmetric
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeSymmetricAsync(
    Client& connection,
    const NodeId& id,
    bool symmetric,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::writeAttributeAsyncImpl<AttributeId::Symmetric>(
        connection, id, symmetric, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::InverseName attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
Result<LocalizedText> readInverseName(T& connection, const NodeId& id) noexcept {
    return detail::readAttributeImpl<AttributeId::InverseName>(connection, id);
}

/**
 * @copydoc readInverseName
 * @param token @completiontoken{void(Result<LocalizedText>&)}
 * @return @asyncresult{Result<LocalizedText>}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readInverseNameAsync(
    Client& connection, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::InverseName>(
        connection, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::InverseName attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to write
 * @param inverseName Value to write
 * @ingroup Write
 */
template <typename T>
StatusCode writeInverseName(
    T& connection, const NodeId& id, const LocalizedText& inverseName
) noexcept {
    return detail::writeAttributeImpl<AttributeId::InverseName>(connection, id, inverseName);
}

/**
 * @copydoc writeInverseName
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeInverseNameAsync(
    Client& connection,
    const NodeId& id,
    const LocalizedText& inverseName,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::writeAttributeAsyncImpl<AttributeId::InverseName>(
        connection, id, inverseName, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::ContainsNoLoops attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
Result<bool> readContainsNoLoops(T& connection, const NodeId& id) noexcept {
    return detail::readAttributeImpl<AttributeId::ContainsNoLoops>(connection, id);
}

/**
 * @copydoc readContainsNoLoops
 * @param token @completiontoken{void(Result<bool>&)}
 * @return @asyncresult{Result<bool>}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readContainsNoLoopsAsync(
    Client& connection, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::ContainsNoLoops>(
        connection, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::ContainsNoLoops attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to write
 * @param containsNoLoops Value to write
 * @ingroup Write
 */
template <typename T>
StatusCode writeContainsNoLoops(
    T& connection, const NodeId& id, const bool& containsNoLoops
) noexcept {
    return detail::writeAttributeImpl<AttributeId::ContainsNoLoops>(
        connection, id, containsNoLoops
    );
}

/**
 * @copydoc writeContainsNoLoops
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeContainsNoLoopsAsync(
    Client& connection,
    const NodeId& id,
    const bool& containsNoLoops,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::writeAttributeAsyncImpl<AttributeId::ContainsNoLoops>(
        connection, id, containsNoLoops, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::EventNotifier attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
Result<Bitmask<EventNotifier>> readEventNotifier(T& connection, const NodeId& id) noexcept {
    return detail::readAttributeImpl<AttributeId::EventNotifier>(connection, id);
}

/**
 * @copydoc readEventNotifier
 * @param token @completiontoken{void(Result<Bitmask<EventNotifier>>)}
 * @return @asyncresult{Result<Bitmask<EventNotifier>>}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readEventNotifierAsync(
    Client& connection, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::EventNotifier>(
        connection, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::EventNotifier attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to write
 * @param eventNotifier Value to write
 * @ingroup Write
 */
template <typename T>
StatusCode writeEventNotifier(
    T& connection, const NodeId& id, Bitmask<EventNotifier> eventNotifier
) noexcept {
    return detail::writeAttributeImpl<AttributeId::EventNotifier>(connection, id, eventNotifier);
}

/**
 * @copydoc writeEventNotifier
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeEventNotifierAsync(
    Client& connection,
    const NodeId& id,
    Bitmask<EventNotifier> eventNotifier,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::writeAttributeAsyncImpl<AttributeId::EventNotifier>(
        connection, id, eventNotifier, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::Value attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
Result<Variant> readValue(T& connection, const NodeId& id) noexcept {
    return detail::readAttributeImpl<AttributeId::Value>(connection, id);
}

/**
 * @copydoc readValue
 * @param token @completiontoken{void(Result<Variant>&)}
 * @return @asyncresult{Result<Variant>}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readValueAsync(
    Client& connection, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::Value>(
        connection, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::Value attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to write
 * @param value Value to write
 * @ingroup Write
 */
template <typename T>
StatusCode writeValue(T& connection, const NodeId& id, const Variant& value) noexcept {
    return detail::writeAttributeImpl<AttributeId::Value>(connection, id, value);
}

/**
 * @copydoc writeValue
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeValueAsync(
    Client& connection,
    const NodeId& id,
    const Variant& value,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::writeAttributeAsyncImpl<AttributeId::Value>(
        connection, id, value, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::DataType attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
Result<NodeId> readDataType(T& connection, const NodeId& id) noexcept {
    return detail::readAttributeImpl<AttributeId::DataType>(connection, id);
}

/**
 * @copydoc readDataType
 * @param token @completiontoken{void(Result<NodeId>&)}
 * @return @asyncresult{Result<NodeId>}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readDataTypeAsync(
    Client& connection, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::DataType>(
        connection, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::DataType attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to write
 * @param dataType Value to write
 * @ingroup Write
 */
template <typename T>
StatusCode writeDataType(T& connection, const NodeId& id, const NodeId& dataType) noexcept {
    return detail::writeAttributeImpl<AttributeId::DataType>(connection, id, dataType);
}

/**
 * @copydoc writeDataType
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeDataTypeAsync(
    Client& connection,
    const NodeId& id,
    const NodeId& dataType,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::writeAttributeAsyncImpl<AttributeId::DataType>(
        connection, id, dataType, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::ValueRank attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
Result<ValueRank> readValueRank(T& connection, const NodeId& id) noexcept {
    return detail::readAttributeImpl<AttributeId::ValueRank>(connection, id);
}

/**
 * @copydoc readValueRank
 * @param token @completiontoken{void(Result<ValueRank>)}
 * @return @asyncresult{Result<ValueRank>}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readValueRankAsync(
    Client& connection, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::ValueRank>(
        connection, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::ValueRank attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to write
 * @param valueRank Value to write
 * @ingroup Write
 */
template <typename T>
StatusCode writeValueRank(T& connection, const NodeId& id, ValueRank valueRank) noexcept {
    return detail::writeAttributeImpl<AttributeId::ValueRank>(connection, id, valueRank);
}

/**
 * @copydoc writeValueRank
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeValueRankAsync(
    Client& connection,
    const NodeId& id,
    ValueRank valueRank,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::writeAttributeAsyncImpl<AttributeId::ValueRank>(
        connection, id, valueRank, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::ArrayDimensions attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
Result<std::vector<uint32_t>> readArrayDimensions(T& connection, const NodeId& id) noexcept {
    return detail::readAttributeImpl<AttributeId::ArrayDimensions>(connection, id);
}

/**
 * @copydoc readArrayDimensions
 * @param token @completiontoken{void(Result<std::vector<uint32_t>>&)}
 * @return @asyncresult{Result<std::vector<uint32_t>>}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readArrayDimensionsAsync(
    Client& connection, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::ArrayDimensions>(
        connection, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::ArrayDimensions attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to write
 * @param arrayDimensions Value to write
 * @ingroup Write
 */
template <typename T>
StatusCode writeArrayDimensions(
    T& connection, const NodeId& id, Span<const uint32_t> arrayDimensions
) noexcept {
    return detail::writeAttributeImpl<AttributeId::ArrayDimensions>(
        connection, id, arrayDimensions
    );
}

/**
 * @copydoc writeArrayDimensions
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeArrayDimensionsAsync(
    Client& connection,
    const NodeId& id,
    Span<const uint32_t> arrayDimensions,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::writeAttributeAsyncImpl<AttributeId::ArrayDimensions>(
        connection, id, arrayDimensions, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::AccessLevel attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
Result<Bitmask<AccessLevel>> readAccessLevel(T& connection, const NodeId& id) noexcept {
    return detail::readAttributeImpl<AttributeId::AccessLevel>(connection, id);
}

/**
 * @copydoc readAccessLevel
 * @param token @completiontoken{void(Result<Bitmask<AccessLevel>>)}
 * @return @asyncresult{Result<Bitmask<AccessLevel>>}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readAccessLevelAsync(
    Client& connection, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::AccessLevel>(
        connection, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::AccessLevel attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to write
 * @param accessLevel Value to write
 * @ingroup Write
 */
template <typename T>
StatusCode writeAccessLevel(
    T& connection, const NodeId& id, Bitmask<AccessLevel> accessLevel
) noexcept {
    return detail::writeAttributeImpl<AttributeId::AccessLevel>(connection, id, accessLevel);
}

/**
 * @copydoc writeAccessLevel
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeAccessLevelAsync(
    Client& connection,
    const NodeId& id,
    Bitmask<AccessLevel> accessLevel,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::writeAttributeAsyncImpl<AttributeId::AccessLevel>(
        connection, id, accessLevel, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::UserAccessLevel attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
Result<Bitmask<AccessLevel>> readUserAccessLevel(T& connection, const NodeId& id) noexcept {
    return detail::readAttributeImpl<AttributeId::UserAccessLevel>(connection, id);
}

/**
 * @copydoc readUserAccessLevel
 * @param token @completiontoken{void(Result<Bitmask<AccessLevel>>)}
 * @return @asyncresult{Result<Bitmask<AccessLevel>>}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readUserAccessLevelAsync(
    Client& connection, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::UserAccessLevel>(
        connection, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::UserAccessLevel attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to write
 * @param userAccessLevel Value to write
 * @ingroup Write
 */
template <typename T>
StatusCode writeUserAccessLevel(
    T& connection, const NodeId& id, Bitmask<AccessLevel> userAccessLevel
) noexcept {
    return detail::writeAttributeImpl<AttributeId::UserAccessLevel>(
        connection, id, userAccessLevel
    );
}

/**
 * @copydoc writeUserAccessLevel
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeUserAccessLevelAsync(
    Client& connection,
    const NodeId& id,
    Bitmask<AccessLevel> userAccessLevel,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::writeAttributeAsyncImpl<AttributeId::UserAccessLevel>(
        connection, id, userAccessLevel, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::MinimumSamplingInterval attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
Result<double> readMinimumSamplingInterval(T& connection, const NodeId& id) noexcept {
    return detail::readAttributeImpl<AttributeId::MinimumSamplingInterval>(connection, id);
}

/**
 * @copydoc readMinimumSamplingInterval
 * @param token @completiontoken{void(Result<double>)}
 * @return @asyncresult{Result<double>}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readMinimumSamplingIntervalAsync(
    Client& connection, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::MinimumSamplingInterval>(
        connection, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::MinimumSamplingInterval attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to write
 * @param minimumSamplingInterval Value to write
 * @ingroup Write
 */
template <typename T>
StatusCode writeMinimumSamplingInterval(
    T& connection, const NodeId& id, double minimumSamplingInterval
) noexcept {
    return detail::writeAttributeImpl<AttributeId::MinimumSamplingInterval>(
        connection, id, minimumSamplingInterval
    );
}

/**
 * @copydoc writeMinimumSamplingInterval
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeMinimumSamplingIntervalAsync(
    Client& connection,
    const NodeId& id,
    double minimumSamplingInterval,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::writeAttributeAsyncImpl<AttributeId::MinimumSamplingInterval>(
        connection, id, minimumSamplingInterval, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::Historizing attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
Result<bool> readHistorizing(T& connection, const NodeId& id) noexcept {
    return detail::readAttributeImpl<AttributeId::Historizing>(connection, id);
}

/**
 * @copydoc readHistorizing
 * @param token @completiontoken{void(Result<bool>)}
 * @return @asyncresult{Result<bool>}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readHistorizingAsync(
    Client& connection, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::Historizing>(
        connection, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::Historizing attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to write
 * @param historizing Value to write
 * @ingroup Write
 */
template <typename T>
StatusCode writeHistorizing(T& connection, const NodeId& id, bool historizing) noexcept {
    return detail::writeAttributeImpl<AttributeId::Historizing>(connection, id, historizing);
}

/**
 * @copydoc writeHistorizing
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeHistorizingAsync(
    Client& connection,
    const NodeId& id,
    bool historizing,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::writeAttributeAsyncImpl<AttributeId::Historizing>(
        connection, id, historizing, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::Executable attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
Result<bool> readExecutable(T& connection, const NodeId& id) noexcept {
    return detail::readAttributeImpl<AttributeId::Executable>(connection, id);
}

/**
 * @copydoc readExecutable
 * @param token @completiontoken{void(Result<bool>)}
 * @return @asyncresult{Result<bool>}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readExecutableAsync(
    Client& connection, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::Executable>(
        connection, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::Executable attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to write
 * @param executable Value to write
 * @ingroup Write
 */
template <typename T>
StatusCode writeExecutable(T& connection, const NodeId& id, bool executable) noexcept {
    return detail::writeAttributeImpl<AttributeId::Executable>(connection, id, executable);
}

/**
 * @copydoc writeExecutable
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeExecutableAsync(
    Client& connection,
    const NodeId& id,
    bool executable,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::writeAttributeAsyncImpl<AttributeId::Executable>(
        connection, id, executable, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::UserExecutable attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
Result<bool> readUserExecutable(T& connection, const NodeId& id) noexcept {
    return detail::readAttributeImpl<AttributeId::UserExecutable>(connection, id);
}

/**
 * @copydoc readUserExecutable
 * @param token @completiontoken{void(Result<bool>)}
 * @return @asyncresult{Result<bool>}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readUserExecutableAsync(
    Client& connection, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::UserExecutable>(
        connection, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::UserExecutable attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to write
 * @param userExecutable Value to write
 * @ingroup Write
 */
template <typename T>
StatusCode writeUserExecutable(T& connection, const NodeId& id, bool userExecutable) noexcept {
    return detail::writeAttributeImpl<AttributeId::UserExecutable>(connection, id, userExecutable);
}

/**
 * @copydoc writeUserExecutable
 * @param token @completiontoken{void(StatusCode)}
 * @return @asyncresult{StatusCode}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
auto writeUserExecutableAsync(
    Client& connection,
    const NodeId& id,
    bool userExecutable,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::writeAttributeAsyncImpl<AttributeId::UserExecutable>(
        connection, id, userExecutable, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::DataTypeDefinition attribute of a node.
 * The attribute value can be of type EnumDefinition or StructureDefinition.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
Result<Variant> readDataTypeDefinition(T& connection, const NodeId& id) noexcept {
    return detail::readAttributeImpl<AttributeId::DataTypeDefinition>(connection, id);
}

/**
 * @copydoc readDataTypeDefinition
 * @param token @completiontoken{void(Result<Variant>&)}
 * @return @asyncresult{Result<Variant>}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
auto readDataTypeDefinitionAsync(
    Client& connection, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::DataTypeDefinition>(
        connection, id, std::forward<CompletionToken>(token)
    );
}

}  // namespace opcua::services
