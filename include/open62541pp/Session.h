#pragma once

#include <open62541pp/types/NodeId.h>

namespace opcua {

// forward declare
class QualifiedName;
class Server;
class Variant;

/**
 * High-level session class to manage client sessions.
 *
 * Sessions are identified by a server-assigned `sessionId` (of type NodeId).
 * A session carries attributes in a key-value list. Custom attributes/meta-data can be attached to
 * a session as key-value pairs of QualifiedName and Variant.
 *
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.6
 */
class Session {
public:
    Session(Server& server, NodeId sessionId) noexcept;

    /// Get the server instance.
    Server& getConnection() noexcept;
    /// Get the server instance.
    const Server& getConnection() const noexcept;

    /// Get the session identifier.
    const NodeId& getSessionId() const noexcept;

    /// Get a session attribute by its key.
    /// @note Only supported since open62541 v1.3
    Variant getSessionAttribute(const QualifiedName& key);

    /// Attach a session attribute as a key-value pair.
    /// @note Only supported since open62541 v1.3
    void setSessionAttribute(const QualifiedName& key, const Variant& value);

    /// Delete a session attribute by its key.
    /// @note Only supported since open62541 v1.3
    void deleteSessionAttribute(const QualifiedName& key);

    /// Manually close this session.
    /// @note Only supported since open62541 v1.3
    void close();

private:
    Server& connection_;
    NodeId sessionId_;
};

bool operator==(const Session& lhs, const Session& rhs) noexcept;
bool operator!=(const Session& lhs, const Session& rhs) noexcept;

}  // namespace opcua
