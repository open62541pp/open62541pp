/* ---------------------------------------------------------------------------------------------- */
/*                                   Generated - do not modify!                                   */
/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "open62541pp/services/Attribute.h"

namespace opcua::services {

/**
 * Read the AttributeId::NodeId attribute of a node.
 * @param connection Instance of type Client (or Server)
 * @param id Node to read
 * @ingroup Read
 */
template <typename T>
inline Result<NodeId> readNodeId(T& connection, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::NodeId>(connection, id);
}

/**
 * Asynchronously read the AttributeId::NodeId attribute of a node.
 * @copydetails readNodeId
 * @param token @completiontoken{void(Result<NodeId>&)}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readNodeIdAsync(
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
inline Result<NodeClass> readNodeClass(T& connection, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::NodeClass>(connection, id);
}

/**
 * Asynchronously read the AttributeId::NodeClass attribute of a node.
 * @copydetails readNodeClass
 * @param token @completiontoken{void(Result<NodeClass>)}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readNodeClassAsync(
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
inline Result<QualifiedName> readBrowseName(T& connection, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::BrowseName>(connection, id);
}

/**
 * Asynchronously read the AttributeId::BrowseName attribute of a node.
 * @copydetails readBrowseName
 * @param token @completiontoken{void(Result<QualifiedName>&)}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readBrowseNameAsync(
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
inline Result<void> writeBrowseName(
    T& connection, const NodeId& id, const QualifiedName& browseName
) {
    return detail::writeAttributeImpl<AttributeId::BrowseName>(connection, id, browseName);
}

/**
 * Asynchronously write the AttributeId::BrowseName attribute of a node.
 * @copydetails writeBrowseName
 * @param token @completiontoken{void(Result<void>)}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeBrowseNameAsync(
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
inline Result<LocalizedText> readDisplayName(T& connection, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::DisplayName>(connection, id);
}

/**
 * Asynchronously read the AttributeId::DisplayName attribute of a node.
 * @copydetails readDisplayName
 * @param token @completiontoken{void(Result<LocalizedText>&)}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readDisplayNameAsync(
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
inline Result<void> writeDisplayName(
    T& connection, const NodeId& id, const LocalizedText& displayName
) {
    return detail::writeAttributeImpl<AttributeId::DisplayName>(connection, id, displayName);
}

/**
 * Asynchronously write the AttributeId::DisplayName attribute of a node.
 * @copydetails writeDisplayName
 * @param token @completiontoken{void(Result<void>)}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeDisplayNameAsync(
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
inline Result<LocalizedText> readDescription(T& connection, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::Description>(connection, id);
}

/**
 * Asynchronously read the AttributeId::Description attribute of a node.
 * @copydetails readDescription
 * @param token @completiontoken{void(Result<LocalizedText>&)}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readDescriptionAsync(
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
inline Result<void> writeDescription(
    T& connection, const NodeId& id, const LocalizedText& description
) {
    return detail::writeAttributeImpl<AttributeId::Description>(connection, id, description);
}

/**
 * Asynchronously write the AttributeId::Description attribute of a node.
 * @copydetails writeDescription
 * @param token @completiontoken{void(Result<void>)}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeDescriptionAsync(
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
inline Result<Bitmask<WriteMask>> readWriteMask(T& connection, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::WriteMask>(connection, id);
}

/**
 * Asynchronously read the AttributeId::WriteMask attribute of a node.
 * @copydetails readWriteMask
 * @param token @completiontoken{void(Result<Bitmask<WriteMask>>)}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readWriteMaskAsync(
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
inline Result<void> writeWriteMask(T& connection, const NodeId& id, Bitmask<WriteMask> writeMask) {
    return detail::writeAttributeImpl<AttributeId::WriteMask>(connection, id, writeMask);
}

/**
 * Asynchronously write the AttributeId::WriteMask attribute of a node.
 * @copydetails writeWriteMask
 * @param token @completiontoken{void(Result<void>)}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeWriteMaskAsync(
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
inline Result<Bitmask<WriteMask>> readUserWriteMask(T& connection, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::UserWriteMask>(connection, id);
}

/**
 * Asynchronously read the AttributeId::UserWriteMask attribute of a node.
 * @copydetails readUserWriteMask
 * @param token @completiontoken{void(Result<Bitmask<WriteMask>>)}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readUserWriteMaskAsync(
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
inline Result<void> writeUserWriteMask(
    T& connection, const NodeId& id, Bitmask<WriteMask> userWriteMask
) {
    return detail::writeAttributeImpl<AttributeId::UserWriteMask>(connection, id, userWriteMask);
}

/**
 * Asynchronously write the AttributeId::UserWriteMask attribute of a node.
 * @copydetails writeUserWriteMask
 * @param token @completiontoken{void(Result<void>)}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeUserWriteMaskAsync(
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
inline Result<bool> readIsAbstract(T& connection, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::IsAbstract>(connection, id);
}

/**
 * Asynchronously read the AttributeId::IsAbstract attribute of a node.
 * @copydetails readIsAbstract
 * @param token @completiontoken{void(Result<bool>)}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readIsAbstractAsync(
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
inline Result<void> writeIsAbstract(T& connection, const NodeId& id, bool isAbstract) {
    return detail::writeAttributeImpl<AttributeId::IsAbstract>(connection, id, isAbstract);
}

/**
 * Asynchronously write the AttributeId::IsAbstract attribute of a node.
 * @copydetails writeIsAbstract
 * @param token @completiontoken{void(Result<void>)}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeIsAbstractAsync(
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
inline Result<bool> readSymmetric(T& connection, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::Symmetric>(connection, id);
}

/**
 * Asynchronously read the AttributeId::Symmetric attribute of a node.
 * @copydetails readSymmetric
 * @param token @completiontoken{void(Result<bool>)}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readSymmetricAsync(
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
inline Result<void> writeSymmetric(T& connection, const NodeId& id, bool symmetric) {
    return detail::writeAttributeImpl<AttributeId::Symmetric>(connection, id, symmetric);
}

/**
 * Asynchronously write the AttributeId::Symmetric attribute of a node.
 * @copydetails writeSymmetric
 * @param token @completiontoken{void(Result<void>)}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeSymmetricAsync(
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
inline Result<LocalizedText> readInverseName(T& connection, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::InverseName>(connection, id);
}

/**
 * Asynchronously read the AttributeId::InverseName attribute of a node.
 * @copydetails readInverseName
 * @param token @completiontoken{void(Result<LocalizedText>&)}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readInverseNameAsync(
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
inline Result<void> writeInverseName(
    T& connection, const NodeId& id, const LocalizedText& inverseName
) {
    return detail::writeAttributeImpl<AttributeId::InverseName>(connection, id, inverseName);
}

/**
 * Asynchronously write the AttributeId::InverseName attribute of a node.
 * @copydetails writeInverseName
 * @param token @completiontoken{void(Result<void>)}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeInverseNameAsync(
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
inline Result<bool> readContainsNoLoops(T& connection, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::ContainsNoLoops>(connection, id);
}

/**
 * Asynchronously read the AttributeId::ContainsNoLoops attribute of a node.
 * @copydetails readContainsNoLoops
 * @param token @completiontoken{void(Result<bool>&)}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readContainsNoLoopsAsync(
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
inline Result<void> writeContainsNoLoops(
    T& connection, const NodeId& id, const bool& containsNoLoops
) {
    return detail::writeAttributeImpl<AttributeId::ContainsNoLoops>(
        connection, id, containsNoLoops
    );
}

/**
 * Asynchronously write the AttributeId::ContainsNoLoops attribute of a node.
 * @copydetails writeContainsNoLoops
 * @param token @completiontoken{void(Result<void>)}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeContainsNoLoopsAsync(
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
inline Result<Bitmask<EventNotifier>> readEventNotifier(T& connection, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::EventNotifier>(connection, id);
}

/**
 * Asynchronously read the AttributeId::EventNotifier attribute of a node.
 * @copydetails readEventNotifier
 * @param token @completiontoken{void(Result<Bitmask<EventNotifier>>)}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readEventNotifierAsync(
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
inline Result<void> writeEventNotifier(
    T& connection, const NodeId& id, Bitmask<EventNotifier> eventNotifier
) {
    return detail::writeAttributeImpl<AttributeId::EventNotifier>(connection, id, eventNotifier);
}

/**
 * Asynchronously write the AttributeId::EventNotifier attribute of a node.
 * @copydetails writeEventNotifier
 * @param token @completiontoken{void(Result<void>)}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeEventNotifierAsync(
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
inline Result<Variant> readValue(T& connection, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::Value>(connection, id);
}

/**
 * Asynchronously read the AttributeId::Value attribute of a node.
 * @copydetails readValue
 * @param token @completiontoken{void(Result<Variant>&)}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readValueAsync(
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
inline Result<void> writeValue(T& connection, const NodeId& id, const Variant& value) {
    return detail::writeAttributeImpl<AttributeId::Value>(connection, id, value);
}

/**
 * Asynchronously write the AttributeId::Value attribute of a node.
 * @copydetails writeValue
 * @param token @completiontoken{void(Result<void>)}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeValueAsync(
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
inline Result<NodeId> readDataType(T& connection, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::DataType>(connection, id);
}

/**
 * Asynchronously read the AttributeId::DataType attribute of a node.
 * @copydetails readDataType
 * @param token @completiontoken{void(Result<NodeId>&)}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readDataTypeAsync(
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
inline Result<void> writeDataType(T& connection, const NodeId& id, const NodeId& dataType) {
    return detail::writeAttributeImpl<AttributeId::DataType>(connection, id, dataType);
}

/**
 * Asynchronously write the AttributeId::DataType attribute of a node.
 * @copydetails writeDataType
 * @param token @completiontoken{void(Result<void>)}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeDataTypeAsync(
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
inline Result<ValueRank> readValueRank(T& connection, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::ValueRank>(connection, id);
}

/**
 * Asynchronously read the AttributeId::ValueRank attribute of a node.
 * @copydetails readValueRank
 * @param token @completiontoken{void(Result<ValueRank>)}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readValueRankAsync(
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
inline Result<void> writeValueRank(T& connection, const NodeId& id, ValueRank valueRank) {
    return detail::writeAttributeImpl<AttributeId::ValueRank>(connection, id, valueRank);
}

/**
 * Asynchronously write the AttributeId::ValueRank attribute of a node.
 * @copydetails writeValueRank
 * @param token @completiontoken{void(Result<void>)}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeValueRankAsync(
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
inline Result<std::vector<uint32_t>> readArrayDimensions(T& connection, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::ArrayDimensions>(connection, id);
}

/**
 * Asynchronously read the AttributeId::ArrayDimensions attribute of a node.
 * @copydetails readArrayDimensions
 * @param token @completiontoken{void(Result<std::vector<uint32_t>>&)}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readArrayDimensionsAsync(
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
inline Result<void> writeArrayDimensions(
    T& connection, const NodeId& id, Span<const uint32_t> arrayDimensions
) {
    return detail::writeAttributeImpl<AttributeId::ArrayDimensions>(
        connection, id, arrayDimensions
    );
}

/**
 * Asynchronously write the AttributeId::ArrayDimensions attribute of a node.
 * @copydetails writeArrayDimensions
 * @param token @completiontoken{void(Result<void>)}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeArrayDimensionsAsync(
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
inline Result<Bitmask<AccessLevel>> readAccessLevel(T& connection, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::AccessLevel>(connection, id);
}

/**
 * Asynchronously read the AttributeId::AccessLevel attribute of a node.
 * @copydetails readAccessLevel
 * @param token @completiontoken{void(Result<Bitmask<AccessLevel>>)}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readAccessLevelAsync(
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
inline Result<void> writeAccessLevel(
    T& connection, const NodeId& id, Bitmask<AccessLevel> accessLevel
) {
    return detail::writeAttributeImpl<AttributeId::AccessLevel>(connection, id, accessLevel);
}

/**
 * Asynchronously write the AttributeId::AccessLevel attribute of a node.
 * @copydetails writeAccessLevel
 * @param token @completiontoken{void(Result<void>)}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeAccessLevelAsync(
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
inline Result<Bitmask<AccessLevel>> readUserAccessLevel(T& connection, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::UserAccessLevel>(connection, id);
}

/**
 * Asynchronously read the AttributeId::UserAccessLevel attribute of a node.
 * @copydetails readUserAccessLevel
 * @param token @completiontoken{void(Result<Bitmask<AccessLevel>>)}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readUserAccessLevelAsync(
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
inline Result<void> writeUserAccessLevel(
    T& connection, const NodeId& id, Bitmask<AccessLevel> userAccessLevel
) {
    return detail::writeAttributeImpl<AttributeId::UserAccessLevel>(
        connection, id, userAccessLevel
    );
}

/**
 * Asynchronously write the AttributeId::UserAccessLevel attribute of a node.
 * @copydetails writeUserAccessLevel
 * @param token @completiontoken{void(Result<void>)}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeUserAccessLevelAsync(
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
inline Result<double> readMinimumSamplingInterval(T& connection, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::MinimumSamplingInterval>(connection, id);
}

/**
 * Asynchronously read the AttributeId::MinimumSamplingInterval attribute of a node.
 * @copydetails readMinimumSamplingInterval
 * @param token @completiontoken{void(Result<double>)}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readMinimumSamplingIntervalAsync(
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
inline Result<void> writeMinimumSamplingInterval(
    T& connection, const NodeId& id, double minimumSamplingInterval
) {
    return detail::writeAttributeImpl<AttributeId::MinimumSamplingInterval>(
        connection, id, minimumSamplingInterval
    );
}

/**
 * Asynchronously write the AttributeId::MinimumSamplingInterval attribute of a node.
 * @copydetails writeMinimumSamplingInterval
 * @param token @completiontoken{void(Result<void>)}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeMinimumSamplingIntervalAsync(
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
inline Result<bool> readHistorizing(T& connection, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::Historizing>(connection, id);
}

/**
 * Asynchronously read the AttributeId::Historizing attribute of a node.
 * @copydetails readHistorizing
 * @param token @completiontoken{void(Result<bool>)}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readHistorizingAsync(
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
inline Result<void> writeHistorizing(T& connection, const NodeId& id, bool historizing) {
    return detail::writeAttributeImpl<AttributeId::Historizing>(connection, id, historizing);
}

/**
 * Asynchronously write the AttributeId::Historizing attribute of a node.
 * @copydetails writeHistorizing
 * @param token @completiontoken{void(Result<void>)}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeHistorizingAsync(
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
inline Result<bool> readExecutable(T& connection, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::Executable>(connection, id);
}

/**
 * Asynchronously read the AttributeId::Executable attribute of a node.
 * @copydetails readExecutable
 * @param token @completiontoken{void(Result<bool>)}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readExecutableAsync(
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
inline Result<void> writeExecutable(T& connection, const NodeId& id, bool executable) {
    return detail::writeAttributeImpl<AttributeId::Executable>(connection, id, executable);
}

/**
 * Asynchronously write the AttributeId::Executable attribute of a node.
 * @copydetails writeExecutable
 * @param token @completiontoken{void(Result<void>)}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeExecutableAsync(
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
inline Result<bool> readUserExecutable(T& connection, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::UserExecutable>(connection, id);
}

/**
 * Asynchronously read the AttributeId::UserExecutable attribute of a node.
 * @copydetails readUserExecutable
 * @param token @completiontoken{void(Result<bool>)}
 * @ingroup Read
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readUserExecutableAsync(
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
inline Result<void> writeUserExecutable(T& connection, const NodeId& id, bool userExecutable) {
    return detail::writeAttributeImpl<AttributeId::UserExecutable>(connection, id, userExecutable);
}

/**
 * Asynchronously write the AttributeId::UserExecutable attribute of a node.
 * @copydetails writeUserExecutable
 * @param token @completiontoken{void(Result<void>)}
 * @ingroup Write
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeUserExecutableAsync(
    Client& connection,
    const NodeId& id,
    bool userExecutable,
    CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::writeAttributeAsyncImpl<AttributeId::UserExecutable>(
        connection, id, userExecutable, std::forward<CompletionToken>(token)
    );
}

}  // namespace opcua::services
