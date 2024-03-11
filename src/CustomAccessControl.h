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
class AccessControlBase;
}  // namespace opcua

namespace opcua {

namespace detail {

void clear(UA_AccessControl& ac) noexcept;

}  // namespace detail

class CustomAccessControl {
public:
    /// Set and apply custom access control.
    void setAccessControl(UA_AccessControl& native, AccessControlBase& accessControl);
    /// Set and apply custom access control (transfer ownership).
    void setAccessControl(
        UA_AccessControl& native, std::unique_ptr<AccessControlBase> accessControl
    );

    AccessControlBase* getAccessControl() noexcept;

private:
    void setAccessControl(UA_AccessControl& ac);

    std::variant<AccessControlBase*, std::unique_ptr<AccessControlBase>> accessControl_{nullptr};
    std::vector<UserTokenPolicy> userTokenPolicies_;
};

}  // namespace opcua
