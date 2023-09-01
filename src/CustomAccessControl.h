#pragma once

#include <memory>
#include <set>
#include <variant>
#include <vector>

#include "open62541pp/types/Composed.h"
#include "open62541pp/types/NodeId.h"

// forward declare
struct UA_AccessControl;

namespace opcua {

namespace detail {

void clearUaAccessControl(UA_AccessControl& ac) noexcept;

}  // namespace detail

// forward declare
class AccessControlBase;
class Server;
class Session;

class CustomAccessControl {
public:
    CustomAccessControl(Server& server);

    /// Apply custom access control (after UA_ServerConfig was changed).
    void setAccessControl();

    /// Set and apply custom access control.
    void setAccessControl(AccessControlBase& accessControl);
    /// Set and apply custom access control (transfer ownership).
    void setAccessControl(std::unique_ptr<AccessControlBase> accessControl);

    void onSessionActivated(const NodeId& sessionId);
    void onSessionClosed(const NodeId& sessionId);

    /// Get active sessions.
    std::vector<Session> getSessions() const;

    Server& getServer() noexcept;
    AccessControlBase* getAccessControl() noexcept;

private:
    Server& server_;
    std::variant<AccessControlBase*, std::unique_ptr<AccessControlBase>> accessControl_;
    std::vector<UserTokenPolicy> userTokenPolicies_;
    std::set<NodeId> sessionIds_;
};

}  // namespace opcua
