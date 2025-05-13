#pragma once

#include <any> // std::any
#include <utility>  // move

#include "open62541pp/types.hpp"

namespace opcua {

class Server;

/**
 * High-level session class to manage client sessions.
 *
 * Sessions are identified by a server-assigned session id (of type NodeId).
 * A session carries attributes in a key-value list. Custom attributes/meta-data can be attached to
 * a session as key-value pairs of QualifiedName and Variant.
 *
 * @see https://reference.opcfoundation.org/Core/Part4/v105/docs/5.7
 */
class Session {
public:
    Session(Server& connection, NodeId sessionId, std::any* sessionContext) noexcept
        : connection_{&connection},
          id_{std::move(sessionId)},
          context_{sessionContext} {

    }

    /// Get the server instance.
    Server& connection() noexcept {
        return *connection_;
    }

    /// Get the server instance.
    const Server& connection() const noexcept {
        return *connection_;
    }

    /// Get the session identifier.
    const NodeId& id() const noexcept {
        return id_;
    }


    std::optional<std::reference_wrapper<std::any>> context() noexcept {
        if (context_) {
            return {*context_};
        }
        return std::nullopt;
    }

    std::optional<std::reference_wrapper<const std::any>> context() const noexcept {
        if (context_) {
            return {*context_};
        }
        return std::nullopt;
    }

    /// Get the session context
    template<typename ContextType>
    std::optional<std::reference_wrapper<ContextType>> context_as() {
        if (auto* result =  std::any_cast<ContextType>(context_)) {
            return {*result};
        }
        return std::nullopt;
    }

    /// Get the session context.
    template<typename ContextType>
    std::optional<std::reference_wrapper<const ContextType>> context_as() const {
        if (auto* result =  std::any_cast<ContextType>(context_)) {
            return {*result};
        }
        return std::nullopt;
    }

    /// Get a session attribute by its key.
    /// @note Supported since open62541 v1.3
    Variant getSessionAttribute(const QualifiedName& key);

    /// Attach a session attribute as a key-value pair.
    /// @note Supported since open62541 v1.3
    void setSessionAttribute(const QualifiedName& key, const Variant& value);

    /// Delete a session attribute by its key.
    /// @note Supported since open62541 v1.3
    void deleteSessionAttribute(const QualifiedName& key);

    /// Manually close this session.
    /// @note Supported since open62541 v1.3
    void close();

private:
    Server* connection_;
    NodeId id_;
    std::any* context_;
};

/// @relates Session
bool operator==(const Session& lhs, const Session& rhs) noexcept;

/// @relates Session
bool operator!=(const Session& lhs, const Session& rhs) noexcept;

}  // namespace opcua
