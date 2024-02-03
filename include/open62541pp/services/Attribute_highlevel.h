/* ---------------------------------------------------------------------------------------------- */
/*                                   Generated - do not modify!                                   */
/* ---------------------------------------------------------------------------------------------- */

#pragma once

#include "open62541pp/services/Attribute.h"

namespace opcua::services {

/**
 * @addtogroup Attribute
 * @{
 */

/**
 * Read the AttributeId::NodeId attribute of a node.
 */
template <typename T>
inline NodeId readNodeId(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::NodeId>(serverOrClient, id);
}

/**
 * Asynchronously read the AttributeId::NodeId attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode, opcua::NodeId&)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readNodeIdAsync(
    Client& client, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::NodeId>(
        client, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::NodeClass attribute of a node.
 */
template <typename T>
inline NodeClass readNodeClass(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::NodeClass>(serverOrClient, id);
}

/**
 * Asynchronously read the AttributeId::NodeClass attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode, opcua::NodeClass)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readNodeClassAsync(
    Client& client, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::NodeClass>(
        client, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::BrowseName attribute of a node.
 */
template <typename T>
inline QualifiedName readBrowseName(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::BrowseName>(serverOrClient, id);
}

/**
 * Asynchronously read the AttributeId::BrowseName attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode, opcua::QualifiedName&)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readBrowseNameAsync(
    Client& client, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::BrowseName>(
        client, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::BrowseName attribute of a node.
 */
template <typename T>
inline void writeBrowseName(T& serverOrClient, const NodeId& id, const QualifiedName& browseName) {
    detail::writeAttributeImpl<AttributeId::BrowseName>(serverOrClient, id, browseName);
}

/**
 * Asynchronously write the AttributeId::BrowseName attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeBrowseNameAsync(
    Client& client, const NodeId& id, const QualifiedName& browseName, CompletionToken&& token
) {
    detail::writeAttributeAsyncImpl<AttributeId::BrowseName>(
        client, id, browseName, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::DisplayName attribute of a node.
 */
template <typename T>
inline LocalizedText readDisplayName(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::DisplayName>(serverOrClient, id);
}

/**
 * Asynchronously read the AttributeId::DisplayName attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode, opcua::LocalizedText&)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readDisplayNameAsync(
    Client& client, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::DisplayName>(
        client, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::DisplayName attribute of a node.
 */
template <typename T>
inline void writeDisplayName(
    T& serverOrClient, const NodeId& id, const LocalizedText& displayName
) {
    detail::writeAttributeImpl<AttributeId::DisplayName>(serverOrClient, id, displayName);
}

/**
 * Asynchronously write the AttributeId::DisplayName attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeDisplayNameAsync(
    Client& client, const NodeId& id, const LocalizedText& displayName, CompletionToken&& token
) {
    detail::writeAttributeAsyncImpl<AttributeId::DisplayName>(
        client, id, displayName, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::Description attribute of a node.
 */
template <typename T>
inline LocalizedText readDescription(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::Description>(serverOrClient, id);
}

/**
 * Asynchronously read the AttributeId::Description attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode, opcua::LocalizedText&)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readDescriptionAsync(
    Client& client, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::Description>(
        client, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::Description attribute of a node.
 */
template <typename T>
inline void writeDescription(
    T& serverOrClient, const NodeId& id, const LocalizedText& description
) {
    detail::writeAttributeImpl<AttributeId::Description>(serverOrClient, id, description);
}

/**
 * Asynchronously write the AttributeId::Description attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeDescriptionAsync(
    Client& client, const NodeId& id, const LocalizedText& description, CompletionToken&& token
) {
    detail::writeAttributeAsyncImpl<AttributeId::Description>(
        client, id, description, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::WriteMask attribute of a node.
 */
template <typename T>
inline Bitmask<WriteMask> readWriteMask(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::WriteMask>(serverOrClient, id);
}

/**
 * Asynchronously read the AttributeId::WriteMask attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode, opcua::Bitmask<WriteMask>)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readWriteMaskAsync(
    Client& client, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::WriteMask>(
        client, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::WriteMask attribute of a node.
 */
template <typename T>
inline void writeWriteMask(T& serverOrClient, const NodeId& id, Bitmask<WriteMask> writeMask) {
    detail::writeAttributeImpl<AttributeId::WriteMask>(serverOrClient, id, writeMask);
}

/**
 * Asynchronously write the AttributeId::WriteMask attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeWriteMaskAsync(
    Client& client, const NodeId& id, Bitmask<WriteMask> writeMask, CompletionToken&& token
) {
    detail::writeAttributeAsyncImpl<AttributeId::WriteMask>(
        client, id, writeMask, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::UserWriteMask attribute of a node.
 */
template <typename T>
inline Bitmask<WriteMask> readUserWriteMask(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::UserWriteMask>(serverOrClient, id);
}

/**
 * Asynchronously read the AttributeId::UserWriteMask attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode, opcua::Bitmask<WriteMask>)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readUserWriteMaskAsync(
    Client& client, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::UserWriteMask>(
        client, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::UserWriteMask attribute of a node.
 */
template <typename T>
inline void writeUserWriteMask(
    T& serverOrClient, const NodeId& id, Bitmask<WriteMask> userWriteMask
) {
    detail::writeAttributeImpl<AttributeId::UserWriteMask>(serverOrClient, id, userWriteMask);
}

/**
 * Asynchronously write the AttributeId::UserWriteMask attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeUserWriteMaskAsync(
    Client& client, const NodeId& id, Bitmask<WriteMask> userWriteMask, CompletionToken&& token
) {
    detail::writeAttributeAsyncImpl<AttributeId::UserWriteMask>(
        client, id, userWriteMask, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::IsAbstract attribute of a node.
 */
template <typename T>
inline bool readIsAbstract(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::IsAbstract>(serverOrClient, id);
}

/**
 * Asynchronously read the AttributeId::IsAbstract attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode, bool)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readIsAbstractAsync(
    Client& client, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::IsAbstract>(
        client, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::IsAbstract attribute of a node.
 */
template <typename T>
inline void writeIsAbstract(T& serverOrClient, const NodeId& id, bool isAbstract) {
    detail::writeAttributeImpl<AttributeId::IsAbstract>(serverOrClient, id, isAbstract);
}

/**
 * Asynchronously write the AttributeId::IsAbstract attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeIsAbstractAsync(
    Client& client, const NodeId& id, bool isAbstract, CompletionToken&& token
) {
    detail::writeAttributeAsyncImpl<AttributeId::IsAbstract>(
        client, id, isAbstract, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::Symmetric attribute of a node.
 */
template <typename T>
inline bool readSymmetric(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::Symmetric>(serverOrClient, id);
}

/**
 * Asynchronously read the AttributeId::Symmetric attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode, bool)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readSymmetricAsync(
    Client& client, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::Symmetric>(
        client, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::Symmetric attribute of a node.
 */
template <typename T>
inline void writeSymmetric(T& serverOrClient, const NodeId& id, bool symmetric) {
    detail::writeAttributeImpl<AttributeId::Symmetric>(serverOrClient, id, symmetric);
}

/**
 * Asynchronously write the AttributeId::Symmetric attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeSymmetricAsync(
    Client& client, const NodeId& id, bool symmetric, CompletionToken&& token
) {
    detail::writeAttributeAsyncImpl<AttributeId::Symmetric>(
        client, id, symmetric, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::InverseName attribute of a node.
 */
template <typename T>
inline LocalizedText readInverseName(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::InverseName>(serverOrClient, id);
}

/**
 * Asynchronously read the AttributeId::InverseName attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode, opcua::LocalizedText&)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readInverseNameAsync(
    Client& client, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::InverseName>(
        client, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::InverseName attribute of a node.
 */
template <typename T>
inline void writeInverseName(
    T& serverOrClient, const NodeId& id, const LocalizedText& inverseName
) {
    detail::writeAttributeImpl<AttributeId::InverseName>(serverOrClient, id, inverseName);
}

/**
 * Asynchronously write the AttributeId::InverseName attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeInverseNameAsync(
    Client& client, const NodeId& id, const LocalizedText& inverseName, CompletionToken&& token
) {
    detail::writeAttributeAsyncImpl<AttributeId::InverseName>(
        client, id, inverseName, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::ContainsNoLoops attribute of a node.
 */
template <typename T>
inline bool readContainsNoLoops(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::ContainsNoLoops>(serverOrClient, id);
}

/**
 * Asynchronously read the AttributeId::ContainsNoLoops attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode, bool&)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readContainsNoLoopsAsync(
    Client& client, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::ContainsNoLoops>(
        client, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::ContainsNoLoops attribute of a node.
 */
template <typename T>
inline void writeContainsNoLoops(T& serverOrClient, const NodeId& id, const bool& containsNoLoops) {
    detail::writeAttributeImpl<AttributeId::ContainsNoLoops>(serverOrClient, id, containsNoLoops);
}

/**
 * Asynchronously write the AttributeId::ContainsNoLoops attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeContainsNoLoopsAsync(
    Client& client, const NodeId& id, const bool& containsNoLoops, CompletionToken&& token
) {
    detail::writeAttributeAsyncImpl<AttributeId::ContainsNoLoops>(
        client, id, containsNoLoops, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::EventNotifier attribute of a node.
 */
template <typename T>
inline Bitmask<EventNotifier> readEventNotifier(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::EventNotifier>(serverOrClient, id);
}

/**
 * Asynchronously read the AttributeId::EventNotifier attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode, opcua::Bitmask<EventNotifier>)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readEventNotifierAsync(
    Client& client, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::EventNotifier>(
        client, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::EventNotifier attribute of a node.
 */
template <typename T>
inline void writeEventNotifier(
    T& serverOrClient, const NodeId& id, Bitmask<EventNotifier> eventNotifier
) {
    detail::writeAttributeImpl<AttributeId::EventNotifier>(serverOrClient, id, eventNotifier);
}

/**
 * Asynchronously write the AttributeId::EventNotifier attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeEventNotifierAsync(
    Client& client, const NodeId& id, Bitmask<EventNotifier> eventNotifier, CompletionToken&& token
) {
    detail::writeAttributeAsyncImpl<AttributeId::EventNotifier>(
        client, id, eventNotifier, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::Value attribute of a node.
 */
template <typename T>
inline Variant readValue(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::Value>(serverOrClient, id);
}

/**
 * Asynchronously read the AttributeId::Value attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode, opcua::Variant&)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readValueAsync(
    Client& client, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::Value>(
        client, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::Value attribute of a node.
 */
template <typename T>
inline void writeValue(T& serverOrClient, const NodeId& id, const Variant& value) {
    detail::writeAttributeImpl<AttributeId::Value>(serverOrClient, id, value);
}

/**
 * Asynchronously write the AttributeId::Value attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeValueAsync(
    Client& client, const NodeId& id, const Variant& value, CompletionToken&& token
) {
    detail::writeAttributeAsyncImpl<AttributeId::Value>(
        client, id, value, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::DataType attribute of a node.
 */
template <typename T>
inline NodeId readDataType(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::DataType>(serverOrClient, id);
}

/**
 * Asynchronously read the AttributeId::DataType attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode, opcua::NodeId&)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readDataTypeAsync(
    Client& client, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::DataType>(
        client, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::DataType attribute of a node.
 */
template <typename T>
inline void writeDataType(T& serverOrClient, const NodeId& id, const NodeId& dataType) {
    detail::writeAttributeImpl<AttributeId::DataType>(serverOrClient, id, dataType);
}

/**
 * Asynchronously write the AttributeId::DataType attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeDataTypeAsync(
    Client& client, const NodeId& id, const NodeId& dataType, CompletionToken&& token
) {
    detail::writeAttributeAsyncImpl<AttributeId::DataType>(
        client, id, dataType, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::ValueRank attribute of a node.
 */
template <typename T>
inline ValueRank readValueRank(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::ValueRank>(serverOrClient, id);
}

/**
 * Asynchronously read the AttributeId::ValueRank attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode, opcua::ValueRank)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readValueRankAsync(
    Client& client, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::ValueRank>(
        client, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::ValueRank attribute of a node.
 */
template <typename T>
inline void writeValueRank(T& serverOrClient, const NodeId& id, ValueRank valueRank) {
    detail::writeAttributeImpl<AttributeId::ValueRank>(serverOrClient, id, valueRank);
}

/**
 * Asynchronously write the AttributeId::ValueRank attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeValueRankAsync(
    Client& client, const NodeId& id, ValueRank valueRank, CompletionToken&& token
) {
    detail::writeAttributeAsyncImpl<AttributeId::ValueRank>(
        client, id, valueRank, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::ArrayDimensions attribute of a node.
 */
template <typename T>
inline std::vector<uint32_t> readArrayDimensions(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::ArrayDimensions>(serverOrClient, id);
}

/**
 * Asynchronously read the AttributeId::ArrayDimensions attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode, std::vector<uint32_t>&)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readArrayDimensionsAsync(
    Client& client, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::ArrayDimensions>(
        client, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::ArrayDimensions attribute of a node.
 */
template <typename T>
inline void writeArrayDimensions(
    T& serverOrClient, const NodeId& id, Span<const uint32_t> arrayDimensions
) {
    detail::writeAttributeImpl<AttributeId::ArrayDimensions>(serverOrClient, id, arrayDimensions);
}

/**
 * Asynchronously write the AttributeId::ArrayDimensions attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeArrayDimensionsAsync(
    Client& client, const NodeId& id, Span<const uint32_t> arrayDimensions, CompletionToken&& token
) {
    detail::writeAttributeAsyncImpl<AttributeId::ArrayDimensions>(
        client, id, arrayDimensions, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::AccessLevel attribute of a node.
 */
template <typename T>
inline Bitmask<AccessLevel> readAccessLevel(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::AccessLevel>(serverOrClient, id);
}

/**
 * Asynchronously read the AttributeId::AccessLevel attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode, opcua::Bitmask<AccessLevel>)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readAccessLevelAsync(
    Client& client, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::AccessLevel>(
        client, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::AccessLevel attribute of a node.
 */
template <typename T>
inline void writeAccessLevel(
    T& serverOrClient, const NodeId& id, Bitmask<AccessLevel> accessLevel
) {
    detail::writeAttributeImpl<AttributeId::AccessLevel>(serverOrClient, id, accessLevel);
}

/**
 * Asynchronously write the AttributeId::AccessLevel attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeAccessLevelAsync(
    Client& client, const NodeId& id, Bitmask<AccessLevel> accessLevel, CompletionToken&& token
) {
    detail::writeAttributeAsyncImpl<AttributeId::AccessLevel>(
        client, id, accessLevel, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::UserAccessLevel attribute of a node.
 */
template <typename T>
inline Bitmask<AccessLevel> readUserAccessLevel(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::UserAccessLevel>(serverOrClient, id);
}

/**
 * Asynchronously read the AttributeId::UserAccessLevel attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode, opcua::Bitmask<AccessLevel>)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readUserAccessLevelAsync(
    Client& client, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::UserAccessLevel>(
        client, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::UserAccessLevel attribute of a node.
 */
template <typename T>
inline void writeUserAccessLevel(
    T& serverOrClient, const NodeId& id, Bitmask<AccessLevel> userAccessLevel
) {
    detail::writeAttributeImpl<AttributeId::UserAccessLevel>(serverOrClient, id, userAccessLevel);
}

/**
 * Asynchronously write the AttributeId::UserAccessLevel attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeUserAccessLevelAsync(
    Client& client, const NodeId& id, Bitmask<AccessLevel> userAccessLevel, CompletionToken&& token
) {
    detail::writeAttributeAsyncImpl<AttributeId::UserAccessLevel>(
        client, id, userAccessLevel, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::MinimumSamplingInterval attribute of a node.
 */
template <typename T>
inline double readMinimumSamplingInterval(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::MinimumSamplingInterval>(serverOrClient, id);
}

/**
 * Asynchronously read the AttributeId::MinimumSamplingInterval attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode, double)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readMinimumSamplingIntervalAsync(
    Client& client, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::MinimumSamplingInterval>(
        client, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::MinimumSamplingInterval attribute of a node.
 */
template <typename T>
inline void writeMinimumSamplingInterval(
    T& serverOrClient, const NodeId& id, double minimumSamplingInterval
) {
    detail::writeAttributeImpl<AttributeId::MinimumSamplingInterval>(
        serverOrClient, id, minimumSamplingInterval
    );
}

/**
 * Asynchronously write the AttributeId::MinimumSamplingInterval attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeMinimumSamplingIntervalAsync(
    Client& client, const NodeId& id, double minimumSamplingInterval, CompletionToken&& token
) {
    detail::writeAttributeAsyncImpl<AttributeId::MinimumSamplingInterval>(
        client, id, minimumSamplingInterval, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::Historizing attribute of a node.
 */
template <typename T>
inline bool readHistorizing(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::Historizing>(serverOrClient, id);
}

/**
 * Asynchronously read the AttributeId::Historizing attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode, bool)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readHistorizingAsync(
    Client& client, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::Historizing>(
        client, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::Historizing attribute of a node.
 */
template <typename T>
inline void writeHistorizing(T& serverOrClient, const NodeId& id, bool historizing) {
    detail::writeAttributeImpl<AttributeId::Historizing>(serverOrClient, id, historizing);
}

/**
 * Asynchronously write the AttributeId::Historizing attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeHistorizingAsync(
    Client& client, const NodeId& id, bool historizing, CompletionToken&& token
) {
    detail::writeAttributeAsyncImpl<AttributeId::Historizing>(
        client, id, historizing, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::Executable attribute of a node.
 */
template <typename T>
inline bool readExecutable(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::Executable>(serverOrClient, id);
}

/**
 * Asynchronously read the AttributeId::Executable attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode, bool)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readExecutableAsync(
    Client& client, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::Executable>(
        client, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::Executable attribute of a node.
 */
template <typename T>
inline void writeExecutable(T& serverOrClient, const NodeId& id, bool executable) {
    detail::writeAttributeImpl<AttributeId::Executable>(serverOrClient, id, executable);
}

/**
 * Asynchronously write the AttributeId::Executable attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeExecutableAsync(
    Client& client, const NodeId& id, bool executable, CompletionToken&& token
) {
    detail::writeAttributeAsyncImpl<AttributeId::Executable>(
        client, id, executable, std::forward<CompletionToken>(token)
    );
}

/**
 * Read the AttributeId::UserExecutable attribute of a node.
 */
template <typename T>
inline bool readUserExecutable(T& serverOrClient, const NodeId& id) {
    return detail::readAttributeImpl<AttributeId::UserExecutable>(serverOrClient, id);
}

/**
 * Asynchronously read the AttributeId::UserExecutable attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode, bool)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto readUserExecutableAsync(
    Client& client, const NodeId& id, CompletionToken&& token = DefaultCompletionToken()
) {
    return detail::readAttributeAsyncImpl<AttributeId::UserExecutable>(
        client, id, std::forward<CompletionToken>(token)
    );
}

/**
 * Write the AttributeId::UserExecutable attribute of a node.
 */
template <typename T>
inline void writeUserExecutable(T& serverOrClient, const NodeId& id, bool userExecutable) {
    detail::writeAttributeImpl<AttributeId::UserExecutable>(serverOrClient, id, userExecutable);
}

/**
 * Asynchronously write the AttributeId::UserExecutable attribute of a node.
 * @param token @completiontoken{void(opcua::StatusCode)}
 */
template <typename CompletionToken = DefaultCompletionToken>
inline auto writeUserExecutableAsync(
    Client& client, const NodeId& id, bool userExecutable, CompletionToken&& token
) {
    detail::writeAttributeAsyncImpl<AttributeId::UserExecutable>(
        client, id, userExecutable, std::forward<CompletionToken>(token)
    );
}

/**
 * @}
 */

}  // namespace opcua::services
