#pragma once

#include <memory>
#include <vector>

// forward declare
struct UA_AccessControl;

namespace opcua {

// forward declare
class AccessControlBase;
class Server;
class UserTokenPolicy;

class CustomAccessControl {
public:
    CustomAccessControl(Server& server, UA_AccessControl& native);

    /// Apply custom access control (after UA_ServerConfig was changed).
    void setAccessControl();

    /// Set and apply custom access control.
    void setAccessControl(AccessControlBase& accessControl);

    // Data behind the UA_AccessControl context pointer.
    struct Context {
        Server& server;
        AccessControlBase* accessControl;
        std::vector<UserTokenPolicy> userTokenPolicies;
    };

private:
    UA_AccessControl& native_;
    Context context_;
};

}  // namespace opcua
