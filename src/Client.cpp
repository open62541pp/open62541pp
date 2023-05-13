#include "open62541pp/Client.h"

#include <atomic>
#include <string>
#include <utility>  // move

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Node.h"
#include "open62541pp/TypeConverter.h"
#include "open62541pp/services/Subscription.h"

#include "ClientContext.h"
#include "CustomLogger.h"
#include "open62541_impl.h"
#include "version.h"

namespace opcua {

/* ------------------------------------------- Helper ------------------------------------------- */

inline static UA_ClientConfig* getConfig(UA_Client* client) noexcept {
    return UA_Client_getConfig(client);
}

inline static UA_ClientConfig* getConfig(Client* client) noexcept {
    return UA_Client_getConfig(client->handle());
}

/* ----------------------------------------- Connection ----------------------------------------- */

class Client::Connection {
public:
    Connection()
        : client_(UA_Client_new()),
          logger_(getConfig(client_)->logger) {
        setContext(client_, context_);
    }

    ~Connection() {
        UA_Client_disconnect(handle());
        UA_Client_delete(handle());
    }

    // prevent copy & move
    Connection(const Connection&) = delete;
    Connection(Connection&&) noexcept = delete;
    Connection& operator=(const Connection&) = delete;
    Connection& operator=(Connection&&) noexcept = delete;

    void setLogger(Logger logger) {
        logger_.setLogger(std::move(logger));
    }

    void runIterate(uint16_t timeoutMilliseconds) {
        const auto status = UA_Client_run_iterate(handle(), timeoutMilliseconds);
        detail::throwOnBadStatus(status);
    }

    void run() {
        if (running_) {
            return;
        }
        running_ = true;
        while (running_) {
            runIterate(1000);
        }
    }

    void stop() {
        running_ = false;
    }

    bool isRunning() const noexcept {
        return running_;
    }

    UA_Client* handle() noexcept {
        return client_;
    }

    ClientContext& getContext() noexcept {
        return context_;
    }

private:
    UA_Client* client_;
    ClientContext context_;
    CustomLogger logger_;
    std::atomic<bool> running_{false};
};

/* ------------------------------------------- Client ------------------------------------------- */

Client::Client()
    : connection_(std::make_shared<Connection>()) {
    const auto status = UA_ClientConfig_setDefault(getConfig(this));
    detail::throwOnBadStatus(status);
    setContext(handle(), getContext());  // overwritten by UA_ClientConfig_setDefault
}

std::vector<ApplicationDescription> Client::findServers(std::string_view serverUrl) {
    UA_ApplicationDescription* array = nullptr;
    size_t arraySize = 0;
    const auto status = UA_Client_findServers(
        handle(),
        std::string(serverUrl).c_str(),  // serverUrl
        0,  // serverUrisSize
        nullptr,  // serverUris
        0,  // localeIdsSize
        nullptr,  // localeIds
        &arraySize,  // registeredServersSize
        &array  // registeredServers
    );
    auto result = detail::fromNativeArray<ApplicationDescription>(array, arraySize);
    UA_Array_delete(array, arraySize, &UA_TYPES[UA_TYPES_APPLICATIONDESCRIPTION]);
    detail::throwOnBadStatus(status);
    return result;
}

std::vector<EndpointDescription> Client::getEndpoints(std::string_view serverUrl) {
    UA_EndpointDescription* array = nullptr;
    size_t arraySize = 0;
    const auto status = UA_Client_getEndpoints(
        handle(),
        std::string(serverUrl).c_str(),  // serverUrl
        &arraySize,  // endpointDescriptionsSize,
        &array  // endpointDescriptions
    );
    auto result = detail::fromNativeArray<EndpointDescription>(array, arraySize);
    UA_Array_delete(array, arraySize, &UA_TYPES[UA_TYPES_ENDPOINTDESCRIPTION]);
    detail::throwOnBadStatus(status);
    return result;
}

void Client::setLogger(Logger logger) {
    connection_->setLogger(std::move(logger));
}

void Client::connect(std::string_view endpointUrl) {
    const auto status = UA_Client_connect(handle(), std::string(endpointUrl).c_str());
    detail::throwOnBadStatus(status);
}

void Client::connect(std::string_view endpointUrl, const Login& login) {
#if UAPP_OPEN62541_VER_LE(1, 0)
    const auto func = UA_Client_connect_username;
#else
    const auto func = UA_Client_connectUsername;
#endif
    const auto status = func(
        handle(), std::string(endpointUrl).c_str(), login.username.c_str(), login.password.c_str()
    );
    detail::throwOnBadStatus(status);
}

void Client::disconnect() noexcept {
    UA_Client_disconnect(handle());
}

bool Client::isConnected() noexcept {
#if UAPP_OPEN62541_VER_LE(1, 0)
    return (UA_Client_getState(handle()) >= UA_CLIENTSTATE_CONNECTED);
#else
    UA_SecureChannelState channelState{};
    UA_Client_getState(handle(), &channelState, nullptr, nullptr);
    return (channelState == UA_SECURECHANNELSTATE_OPEN);
#endif
}

std::vector<std::string> Client::getNamespaceArray() {
    Variant variant;
    services::readValue(*this, {0, UA_NS0ID_SERVER_NAMESPACEARRAY}, variant);
    return variant.getArrayCopy<std::string>();
}

Subscription<Client> Client::createSubscription() {
    SubscriptionParameters parameters{};
    return createSubscription(parameters);
}

Subscription<Client> Client::createSubscription(SubscriptionParameters& parameters) {
    const uint32_t subscriptionId = services::createSubscription(*this, parameters, true);
    return {*this, subscriptionId};
}

std::vector<Subscription<Client>> Client::getSubscriptions() {
    const auto& subscriptions = getContext().subscriptions;
    std::vector<Subscription<Client>> result;
    result.reserve(subscriptions.size());
    for (const auto& [subId, _] : subscriptions) {
        result.emplace_back(*this, subId);
    }
    return result;
}

void Client::runIterate(uint16_t timeoutMilliseconds) {
    connection_->runIterate(timeoutMilliseconds);
}

void Client::run() {
    connection_->run();
}

void Client::stop() {
    connection_->stop();
}

bool Client::isRunning() const noexcept {
    return connection_->isRunning();
}

Node<Client> Client::getNode(const NodeId& id) {
    return {*this, id, true};
}

Node<Client> Client::getRootNode() {
    return {*this, {0, UA_NS0ID_ROOTFOLDER}, false};
}

Node<Client> Client::getObjectsNode() {
    return {*this, {0, UA_NS0ID_OBJECTSFOLDER}, false};
}

Node<Client> Client::getTypesNode() {
    return {*this, {0, UA_NS0ID_TYPESFOLDER}, false};
}

Node<Client> Client::getViewsNode() {
    return {*this, {0, UA_NS0ID_VIEWSFOLDER}, false};
}

UA_Client* Client::handle() noexcept {
    return connection_->handle();
}

const UA_Client* Client::handle() const noexcept {
    return connection_->handle();
}

ClientContext& Client::getContext() noexcept {
    return connection_->getContext();
}

/* ---------------------------------------------------------------------------------------------- */

bool operator==(const Client& left, const Client& right) noexcept {
    return (left.handle() == right.handle());
}

bool operator!=(const Client& left, const Client& right) noexcept {
    return !(left == right);
}

}  // namespace opcua
