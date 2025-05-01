#include "open62541pp/server.hpp"

#include <atomic>
#include <cassert>
#include <mutex>
#include <utility>  // move

#include "open62541pp/datatype.hpp"
#include "open62541pp/detail/result_utils.hpp"  // tryInvoke
#include "open62541pp/detail/server_context.hpp"
#include "open62541pp/event.hpp"
#include "open62541pp/exception.hpp"
#include "open62541pp/node.hpp"
#include "open62541pp/services/attribute_highlevel.hpp"
#include "open62541pp/session.hpp"
#include "open62541pp/types.hpp"
#include "open62541pp/ua/types.hpp"
#include "open62541pp/wrapper.hpp"  // asWrapper

namespace opcua {

/* ---------------------------------------- ServerConfig ---------------------------------------- */

ServerConfig::ServerConfig() {
    throwIfBad(UA_ServerConfig_setDefault(handle()));
}

ServerConfig::ServerConfig(uint16_t port, const ByteString& certificate) {
    throwIfBad(UA_ServerConfig_setMinimal(
        handle(), port, certificate.empty() ? nullptr : certificate.handle()
    ));
}

#ifdef UA_ENABLE_ENCRYPTION
ServerConfig::ServerConfig(
    uint16_t port,
    const ByteString& certificate,
    const ByteString& privateKey,
    Span<const ByteString> trustList,
    Span<const ByteString> issuerList,
    Span<const ByteString> revocationList
) {
    throwIfBad(UA_ServerConfig_setDefaultWithSecurityPolicies(
        handle(),
        port,
        certificate.handle(),
        privateKey.handle(),
        asNative(trustList.data()),
        trustList.size(),
        asNative(issuerList.data()),
        issuerList.size(),
        asNative(revocationList.data()),
        revocationList.size()
    ));
}
#endif

// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
ServerConfig::ServerConfig(UA_ServerConfig&& native)
    : Wrapper{std::exchange(native, {})} {}

ServerConfig::~ServerConfig() {
    detail::deallocate(native().customDataTypes);
    native().customDataTypes = nullptr;
    UA_ServerConfig_clean(handle());
}

// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
ServerConfig::ServerConfig(ServerConfig&& other) noexcept
    : Wrapper{std::exchange(other.native(), {})} {}

ServerConfig& ServerConfig::operator=(ServerConfig&& other) noexcept {
    if (this != &other) {
        // UA_ServerConfig_clean(handle());  // TODO
        native() = std::exchange(other.native(), {});
    }
    return *this;
}

void ServerConfig::setLogger(LogFunction func) {
    if (func) {
        auto adapter = std::make_unique<LoggerDefault>(std::move(func));
        auto* logger = detail::getLogger(handle());
        assert(logger != nullptr);
        detail::clear(*logger);
        *logger = adapter.release()->create(true);
    }
}

void ServerConfig::setBuildInfo(BuildInfo buildInfo) {
    asWrapper<BuildInfo>(native().buildInfo) = std::move(buildInfo);
}

static ApplicationDescription& getApplicationDescription(UA_ServerConfig& config) noexcept {
    return asWrapper<ApplicationDescription>(config.applicationDescription);
}

// copy to endpoints needed, see: https://github.com/open62541/open62541/issues/1175
static void copyApplicationDescriptionToEndpoints(UA_ServerConfig& config) {
    auto endpoints = Span(asWrapper<EndpointDescription>(config.endpoints), config.endpointsSize);
    for (auto& endpoint : endpoints) {
        endpoint.server() = getApplicationDescription(config);
    }
}

void ServerConfig::setApplicationUri(std::string_view uri) {
    getApplicationDescription(native()).applicationUri() = String{uri};
    copyApplicationDescriptionToEndpoints(native());
}

void ServerConfig::setProductUri(std::string_view uri) {
    getApplicationDescription(native()).productUri() = String{uri};
    copyApplicationDescriptionToEndpoints(native());
}

void ServerConfig::setApplicationName(std::string_view name) {
    getApplicationDescription(native()).applicationName() = LocalizedText{"", name};
    copyApplicationDescriptionToEndpoints(native());
}

void ServerConfig::addCustomDataTypes(Span<const DataType> types) {
    detail::addDataTypes(native().customDataTypes, types);
}

static void copyUserTokenPoliciesToEndpoints(UA_ServerConfig& config) {
    // copy config.accessControl.userTokenPolicies -> config.endpoints[i].userIdentityTokens
    auto& ac = config.accessControl;
    for (auto& endpoint : Span(config.endpoints, config.endpointsSize)) {
        detail::deallocateArray(
            endpoint.userIdentityTokens,
            endpoint.userIdentityTokensSize,
            UA_TYPES[UA_TYPES_USERTOKENPOLICY]
        );
        endpoint.userIdentityTokens = detail::copyArray(
            ac.userTokenPolicies, ac.userTokenPoliciesSize, UA_TYPES[UA_TYPES_USERTOKENPOLICY]
        );
        endpoint.userIdentityTokensSize = ac.userTokenPoliciesSize;
    }
}

static void setHighestSecurityPolicyForUserTokenTransfer(UA_ServerConfig& config) {
    auto& ac = config.accessControl;
    const Span securityPolicies{config.securityPolicies, config.securityPoliciesSize};
    Span userTokenPolicies(
        asWrapper<UserTokenPolicy>(ac.userTokenPolicies), ac.userTokenPoliciesSize
    );
    if (!securityPolicies.empty()) {
        const auto& highestSecurityPoliciyUri = securityPolicies.back().policyUri;
        for (auto& userTokenPolicy : userTokenPolicies) {
            if (userTokenPolicy.tokenType() != UserTokenType::Anonymous &&
                userTokenPolicy.securityPolicyUri().empty()) {
                userTokenPolicy.securityPolicyUri() = String{highestSecurityPoliciyUri};
            }
        }
    }
}

void ServerConfig::setAccessControl(AccessControlBase& accessControl) {
    detail::clear(native().accessControl);
    native().accessControl = accessControl.create(false);
    setHighestSecurityPolicyForUserTokenTransfer(native());
    copyUserTokenPoliciesToEndpoints(native());
}

// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
void ServerConfig::setAccessControl(std::unique_ptr<AccessControlBase>&& accessControl) {
    if (accessControl != nullptr) {
        detail::clear(native().accessControl);
        native().accessControl = accessControl.release()->create(true);
        setHighestSecurityPolicyForUserTokenTransfer(native());
        copyUserTokenPoliciesToEndpoints(native());
    }
}

/* ------------------------------------------- Server ------------------------------------------- */

static UA_StatusCode activateSession(
    UA_Server* server,
    UA_AccessControl* ac,
    const UA_EndpointDescription* endpointDescription,
    const UA_ByteString* secureChannelRemoteCertificate,
    const UA_NodeId* sessionId,
    const UA_ExtensionObject* userIdentityToken,
    void** sessionContext
) {
    auto* context = detail::getContext(server);
    if (context == nullptr || context->sessionRegistry.activateSessionUser == nullptr) {
        return UA_STATUSCODE_BADINTERNALERROR;
    }
    // call user-defined function
    auto status = context->sessionRegistry.activateSessionUser(
        server,
        ac,
        endpointDescription,
        secureChannelRemoteCertificate,
        sessionId,
        userIdentityToken,
        sessionContext
    );
    if (detail::isGood(status) && sessionId != nullptr) {
        const std::scoped_lock lock(context->sessionRegistry.mutex);
        context->sessionRegistry.sessions.insert_or_assign(
            asWrapper<NodeId>(*sessionId), *sessionContext
        );
    }
    return status;
}

static void closeSession(
    UA_Server* server, UA_AccessControl* ac, const UA_NodeId* sessionId, void* sessionContext
) {
    auto* context = detail::getContext(server);
    if (context == nullptr || context->sessionRegistry.closeSessionUser == nullptr) {
        return;
    }
    // call user-defined function
    context->sessionRegistry.closeSessionUser(server, ac, sessionId, sessionContext);
    if (sessionId != nullptr) {
        const std::scoped_lock lock(context->sessionRegistry.mutex);
        context->sessionRegistry.sessions.erase(asWrapper<NodeId>(*sessionId));
    }
}

static void applySessionRegistry(UA_ServerConfig& config, detail::ServerContext& context) {
    // Make sure to call this function only once after access control is initialized or changed.
    // The function pointers to activateSession / closeSession might not be unique and the
    // the pointer comparison might fail resulting in stack overflows:
    // - https://github.com/open62541pp/open62541pp/issues/285
    // - https://stackoverflow.com/questions/31209693/static-library-linked-two-times
    if (config.accessControl.activateSession != &activateSession) {
        context.sessionRegistry.activateSessionUser = config.accessControl.activateSession;
        config.accessControl.activateSession = &activateSession;
    }
    if (config.accessControl.closeSession != &closeSession) {
        context.sessionRegistry.closeSessionUser = config.accessControl.closeSession;
        config.accessControl.closeSession = &closeSession;
    }
}

static void updateLoggerStackPointer([[maybe_unused]] UA_ServerConfig& config) noexcept {
#if UAPP_OPEN62541_VER_LE(1, 2)
    for (auto& layer : Span(config.networkLayers, config.networkLayersSize)) {
        // https://github.com/open62541/open62541/blob/v1.0.6/arch/network_tcp.c#L556-L563
        *static_cast<UA_Logger**>(layer.handle) = &config.logger;
    }
    for (auto& policy : Span(config.securityPolicies, config.securityPoliciesSize)) {
        policy.logger = &config.logger;
    }
#endif
}

static void setWrapperAsContextPointer(Server& server) noexcept {
#if UAPP_OPEN62541_VER_GE(1, 3)
    server.config()->context = &server;
#else
    const auto status = UA_Server_setNodeContext(
        server.handle(), UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER), &server
    );
    assert(status == UA_STATUSCODE_GOOD);
#endif
}

Server::Server()
    : Server{ServerConfig{}} {}

// NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved)
Server::Server(ServerConfig&& config)
    : context_{std::make_unique<detail::ServerContext>()},
      server_{UA_Server_newWithConfig(config.handle())} {
    if (handle() == nullptr) {
        throw BadStatus(UA_STATUSCODE_BADOUTOFMEMORY);
    }
    config = {};
#if UAPP_OPEN62541_VER_GE(1, 2)
    this->config()->allowEmptyVariables = UA_RULEHANDLING_ACCEPT;  // allow empty variables
#endif
    updateLoggerStackPointer(this->config());
    setWrapperAsContextPointer(*this);
}

Server::Server(UA_Server* native)
    : context_{std::make_unique<detail::ServerContext>()},
      server_{native} {
    if (handle() == nullptr) {
        throw BadStatus(UA_STATUSCODE_BADOUTOFMEMORY);
    }
    setWrapperAsContextPointer(*this);
}

Server::~Server() = default;

Server::Server(Server&& other) noexcept
    : context_{std::move(other.context_)},
      server_{std::move(other.server_)} {
    setWrapperAsContextPointer(*this);
}

Server& Server::operator=(Server&& other) noexcept {
    if (this != &other) {
        context_ = std::move(other.context_);
        server_ = std::move(other.server_);
        setWrapperAsContextPointer(*this);
    }
    return *this;
}

ServerConfig& Server::config() noexcept {
    return asWrapper<ServerConfig>(*detail::getConfig(handle()));
}

const ServerConfig& Server::config() const noexcept {
    return const_cast<Server*>(this)->config();  // NOLINT
}

std::vector<Session> Server::sessions() {
    std::vector<Session> result;
    const std::scoped_lock lock(context().sessionRegistry.mutex);
    for (auto& [id, context] : context().sessionRegistry.sessions) {
        result.emplace_back(*this, id, context);
    }
    return result;
}

std::vector<std::string> Server::namespaceArray() {
    return services::readValue(*this, {0, UA_NS0ID_SERVER_NAMESPACEARRAY})
        .value()
        .to<std::vector<std::string>>();
}

NamespaceIndex Server::registerNamespace(std::string_view uri) {
    return UA_Server_addNamespace(handle(), std::string{uri}.c_str());
}

static void setVariableNodeValueCallbackImpl(
    Server& server, const NodeId& id, detail::UniqueOrRawPtr<ValueCallbackBase>&& callback
) {
    auto* nodeContext = detail::getContext(server).nodeContexts[id];
    nodeContext->valueCallback = std::move(callback);
    throwIfBad(UA_Server_setNodeContext(server.handle(), id, nodeContext));
    throwIfBad(UA_Server_setVariableNode_valueCallback(
        server.handle(), id, nodeContext->valueCallback->create(false)
    ));
}

void Server::setVariableNodeValueCallback(const NodeId& id, ValueCallbackBase& callback) {
    setVariableNodeValueCallbackImpl(*this, id, detail::UniqueOrRawPtr{&callback});
}

void Server::setVariableNodeValueCallback(
    const NodeId& id, std::unique_ptr<ValueCallbackBase>&& callback
) {
    setVariableNodeValueCallbackImpl(*this, id, detail::UniqueOrRawPtr{std::move(callback)});
}

static void setVariableNodeDataSourceImpl(
    Server& server, const NodeId& id, detail::UniqueOrRawPtr<DataSourceBase>&& source
) {
    auto* nodeContext = detail::getContext(server).nodeContexts[id];
    nodeContext->dataSource = std::move(source);
    throwIfBad(UA_Server_setNodeContext(server.handle(), id, nodeContext));
    throwIfBad(UA_Server_setVariableNode_dataSource(
        server.handle(), id, nodeContext->dataSource->create(false)
    ));
}

void Server::setVariableNodeDataSource(const NodeId& id, DataSourceBase& source) {
    setVariableNodeDataSourceImpl(*this, id, detail::UniqueOrRawPtr{&source});
}

void Server::setVariableNodeDataSource(const NodeId& id, std::unique_ptr<DataSourceBase>&& source) {
    setVariableNodeDataSourceImpl(*this, id, detail::UniqueOrRawPtr{std::move(source)});
}

#ifdef UA_ENABLE_SUBSCRIPTIONS
Subscription<Server> Server::createSubscription() noexcept {
    return {*this, 0U};
}
#endif

static void runStartup(Server& server, detail::ServerContext& context) {
    applySessionRegistry(server.config(), context);
    throwIfBad(UA_Server_run_startup(server.handle()));
    context.running = true;
}

uint16_t Server::runIterate() {
    if (!context().running) {
        runStartup(*this, context());
    }
    auto interval = UA_Server_run_iterate(handle(), false /* don't wait */);
    context().exceptionCatcher.rethrow();
    return interval;
}

void Server::run() {
    if (context().running) {
        return;
    }
    runStartup(*this, context());
    const std::lock_guard lock(context().mutexRun);
    try {
        while (context().running) {
            // https://github.com/open62541/open62541/blob/master/examples/server_mainloop.c
            UA_Server_run_iterate(handle(), true /* wait for messages in the networklayer */);
            context().exceptionCatcher.rethrow();
        }
    } catch (...) {
        context().running = false;
        throw;
    }
}

void Server::stop() {
    if (!context().running) {
        return;
    }
    context().running = false;
    // wait for run loop to complete
    const std::lock_guard<std::mutex> lock(context().mutexRun);
    throwIfBad(UA_Server_run_shutdown(handle()));
}

bool Server::isRunning() const noexcept {
    return context().running;
}

UA_Server* Server::handle() noexcept {
    return server_.get();
}

const UA_Server* Server::handle() const noexcept {
    return server_.get();
}

detail::ServerContext& Server::context() noexcept {
    return *context_;
}

const detail::ServerContext& Server::context() const noexcept {
    return *context_;
}

void Server::Deleter::operator()(UA_Server* server) noexcept {
    if (server == nullptr) {
        return;
    }
    if (detail::getContext(server)->running) {
        UA_Server_run_shutdown(server);
    }
    auto* config = UA_Server_getConfig(server);
    detail::deallocate(config->customDataTypes);
    config->customDataTypes = nullptr;
    UA_Server_delete(server);
}

Server* asWrapper(UA_Server* server) noexcept {
#if UAPP_OPEN62541_VER_GE(1, 3)
    auto* config = detail::getConfig(server);
    return config == nullptr ? nullptr : static_cast<Server*>(config->context);
#else
    if (server == nullptr) {
        return nullptr;
    }
    // use node context of server object as fallback
    void* nodeContext = nullptr;
    const auto status = UA_Server_getNodeContext(
        server, UA_NODEID_NUMERIC(0, UA_NS0ID_SERVER), &nodeContext
    );
    if (status != UA_STATUSCODE_GOOD) {
        return nullptr;
    }
    return static_cast<Server*>(nodeContext);
#endif
}

/* -------------------------------------- Utility functions ------------------------------------- */

namespace detail {

UA_ServerConfig* getConfig(UA_Server* server) noexcept {
    return UA_Server_getConfig(server);
}

UA_Logger* getLogger(UA_ServerConfig* config) noexcept {
#if UAPP_OPEN62541_VER_GE(1, 4)
    return config == nullptr ? nullptr : config->logging;
#else
    return config == nullptr ? nullptr : &config->logger;
#endif
}

ServerContext* getContext(UA_Server* server) noexcept {
    auto* wrapper = asWrapper(server);
    return wrapper == nullptr ? nullptr : &getContext(*wrapper);
}

ServerContext& getContext(Server& server) noexcept {
    return server.context();
}

ExceptionCatcher* getExceptionCatcher(UA_Server* server) noexcept {
    auto* context = getContext(server);
    return context == nullptr ? nullptr : &context->exceptionCatcher;
}

ExceptionCatcher& getExceptionCatcher(Server& server) noexcept {
    return getContext(server).exceptionCatcher;
}

UA_Server* getHandle(Server& server) noexcept {
    return server.handle();
}

}  // namespace detail

/* -------------------------------------- Async operations -------------------------------------- */

#if UAPP_HAS_ASYNC_OPERATIONS
void useAsyncOperation(Server& server, const NodeId& id, bool enabled) {
    throwIfBad(UA_Server_setMethodNodeAsync(server.handle(), id, enabled));
}

template <typename T>
static void setAsyncOperationResult(Server& server, const T& response, void* context) noexcept {
    static_assert(sizeof(T) <= sizeof(UA_AsyncOperationResponse));
    UA_Server_setAsyncOperationResult(
        server.handle(),
        reinterpret_cast<const UA_AsyncOperationResponse*>(&response),  // NOLINT
        context
    );
}

std::optional<AsyncOperation> getAsyncOperation(Server& server) noexcept {
    AsyncOperation operation{};
    const bool hasRequest = UA_Server_getAsyncOperationNonBlocking(
        server.handle(), &operation.type, &operation.request, &operation.context, nullptr
    );
    if (!hasRequest || operation.request == nullptr) {
        return std::nullopt;
    }
    return operation;
}

void runAsyncOperation(Server& server, const AsyncOperation& operation) {
    if (operation.request == nullptr) {
        return;
    }
    if (operation.type == UA_ASYNCOPERATIONTYPE_CALL) {
        const CallMethodResult response = UA_Server_call(
            server.handle(), &operation.request->callMethodRequest
        );
        setAsyncOperationResult(server, response, operation.context);
    }
}
#endif

}  // namespace opcua
