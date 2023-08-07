#pragma once

#include <memory>
#include <set>
#include <vector>

// forward declare
struct UA_AccessControl;

namespace opcua {

// forward declare
class AccessControlBase;
class NodeId;
class Server;
class Session;
class UserTokenPolicy;

class CustomAccessControl {
public:
    CustomAccessControl(Server& server, UA_AccessControl& native);

    /// Apply custom access control (after UA_ServerConfig was changed).
    void setAccessControl();

    /// Set and apply custom access control.
    void setAccessControl(AccessControlBase& accessControl);

    void onSessionActivated(const NodeId& sessionId);
    void onSessionClosed(const NodeId& sessionId);

    /// Get active sessions.
    std::vector<Session> getSessions() const;

    Server& getServer() noexcept;
    AccessControlBase* getAccessControl() noexcept;

private:
    Server& server_;
    UA_AccessControl& native_;
    AccessControlBase* accessControl_{};
    std::vector<UserTokenPolicy> userTokenPolicies_;
    std::set<NodeId> sessionIds_;
};

}  // namespace opcua
