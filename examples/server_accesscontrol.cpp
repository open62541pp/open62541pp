#include <iostream>

#include <open62541pp/node.hpp>
#include <open62541pp/plugin/accesscontrol_default.hpp>
#include <open62541pp/server.hpp>

using namespace opcua;

// Custom access control based on AccessControlDefault.
// If a user logs in with the username "admin", a session attribute "isAdmin" is stored. As an
// example, the user "admin" has write access level to the created variable node. So admins can
// change the value of the created variable node, anonymous users and the user "user" can't.
// Session attributes are available since open62541 v1.3, so this example requires at least v1.3.
class AccessControlCustom : public AccessControlDefault {
public:
    using AccessControlDefault::AccessControlDefault;  // inherit constructors

    struct SessionContext {
        bool isAdmin;
    };

    StatusCode activateSession(
        Session& session,
        const EndpointDescription& endpointDescription,
        const ByteString& secureChannelRemoteCertificate,
        const ExtensionObject& userIdentityToken,
        void*& sessionContext
    ) override {
        // Grant admin rights if user is logged in as "admin"
        const auto* token = userIdentityToken.decodedData<UserNameIdentityToken>();
        const bool isAdmin = (token != nullptr && token->userName() == "admin");

        // Create session context (must be deleted in closeSession)
        sessionContext = new SessionContext{isAdmin};  // NOLINT(*memory)

        return AccessControlDefault::activateSession(
            session,
            endpointDescription,
            secureChannelRemoteCertificate,
            userIdentityToken,
            sessionContext
        );
    }

    void closeSession(Session& session) override {
        delete static_cast<SessionContext*>(session.context());  // NOLINT(*memory)
    }

    Bitmask<AccessLevel> getUserAccessLevel(Session& session, const NodeId& nodeId) override {
        const bool isAdmin = static_cast<SessionContext*>(session.context())->isAdmin;
        std::cout << "Get user access level of node id " << opcua::toString(nodeId) << std::endl;
        std::cout << "Admin rights granted: " << isAdmin << std::endl;
        return isAdmin
            ? AccessLevel::CurrentRead | AccessLevel::CurrentWrite
            : AccessLevel::CurrentRead;
    }
};

int main() {
    // Exchanging usernames/passwords without encryption as plain text is dangerous.
    // We are doing this just for demonstration, don't use it in production!
    AccessControlCustom accessControl{
        true,  // allow anonymous
        {
            Login{String{"admin"}, String{"admin"}},
            Login{String{"user"}, String{"user"}},
        }
    };

    ServerConfig config;
    config.setAccessControl(accessControl);
#if UAPP_OPEN62541_VER_GE(1, 4)
    config->allowNonePolicyPassword = true;
#endif

    Server server{std::move(config)};

    // Add variable node. Try to change its value as a client with different logins.
    Node{server, ObjectId::ObjectsFolder}.addVariable(
        {1, 1000},
        "Variable",
        VariableAttributes{}
            .setAccessLevel(AccessLevel::CurrentRead | AccessLevel::CurrentWrite)
            .setDataType(DataTypeId::Int32)
            .setValueRank(ValueRank::Scalar)
            .setValue(opcua::Variant{0})
    );

    server.run();
}
