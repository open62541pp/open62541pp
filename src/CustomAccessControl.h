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

void clear(UA_AccessControl& ac) noexcept;

}  // namespace detail

// forward declare
class AccessControlBase;
class Server;
class Session;

class CustomAccessControl {
public:
    /// Set and apply custom access control.
    void setAccessControl(UA_AccessControl& native, AccessControlBase& accessControl);
    /// Set and apply custom access control (transfer ownership).
    void setAccessControl(UA_AccessControl& native, std::unique_ptr<AccessControlBase> accessControl);

    void onSessionActivated(const NodeId& sessionId);
    void onSessionClosed(const NodeId& sessionId);

    /// Get active sessions.
    std::vector<Session> getSessions() const;

    void setServer(Server& server) noexcept;
    Server& getServer() noexcept;
    AccessControlBase* getAccessControl() noexcept;

private:
    void setAccessControl(UA_AccessControl& ac);

    Server* server_{nullptr};
    std::variant<AccessControlBase*, std::unique_ptr<AccessControlBase>> accessControl_{nullptr};
    std::vector<UserTokenPolicy> userTokenPolicies_;
    std::set<NodeId> sessionIds_;
};

}  // namespace opcua
