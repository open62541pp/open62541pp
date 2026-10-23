#include "open62541pp/plugin/create_certificate.hpp"

#if UAPP_HAS_CREATE_CERTIFICATE

#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "open62541pp/config.hpp"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/exception.hpp"
#include "open62541pp/plugin/log_default.hpp"

namespace opcua {

static_assert(static_cast<int>(CertificateFormat::DER) == UA_CERTIFICATEFORMAT_DER);
static_assert(static_cast<int>(CertificateFormat::PEM) == UA_CERTIFICATEFORMAT_PEM);

CreateCertificateResult createCertificate(
    Span<const String> subject,
    Span<const String> subjectAltName,
    CertificateFormat certificateFormat,
    uint16_t keySizeBits
) {
    if (subject.empty() || subjectAltName.empty()) {
        throw CreateCertificateError("Argument subject or subjectAltName is empty");
    }

    // OpenSSL errors will generate a generic UA_STATUSCODE_BADINTERNALERROR status code
    // detailed errors are reported through error log messages -> capture log messages
    std::optional<std::string> error;
    LoggerDefault loggerAdapter(
        [&](LogLevel level, [[maybe_unused]] LogCategory category, std::string_view msg) {
            if (level >= LogLevel::Error && !error /* keep first error */) {
                error = msg;
            }
        }
    );
    const UA_Logger logger = loggerAdapter.create(false);

#if UAPP_OPEN62541_VER_GE(1, 4)
    // optional parameters, only add key size if explicitly requested
    struct KeyValueMapDeleter {
        void operator()(UA_KeyValueMap* map) const noexcept {
            UA_KeyValueMap_delete(map);
        }
    };

    std::string keySizeBitsKey = "key-size-bits";
    std::unique_ptr<UA_KeyValueMap, KeyValueMapDeleter> params;
    if (keySizeBits != 0) {
        params.reset(UA_KeyValueMap_new());
        if (params == nullptr) {
            throw BadStatus(UA_STATUSCODE_BADOUTOFMEMORY);
        }
        throwIfBad(UA_KeyValueMap_setScalar(
            params.get(),
            UA_QUALIFIEDNAME(0, keySizeBitsKey.data()),
            &keySizeBits,
            &UA_TYPES[UA_TYPES_UINT16]
        ));
    }
#endif

    CreateCertificateResult result;
    const auto status = UA_CreateCertificate(
        &logger,
        asNative(subject.data()),
        subject.size(),
        asNative(subjectAltName.data()),
        subjectAltName.size(),
#if UAPP_OPEN62541_VER_LE(1, 3)
        keySizeBits,  // 0 = maximum
#endif
        static_cast<UA_CertificateFormat>(certificateFormat),
#if UAPP_OPEN62541_VER_GE(1, 4)
        params.get(),  // key value map with optional parameters
#endif
        result.privateKey.handle(),
        result.certificate.handle()
    );

    if (error) {
        throw CreateCertificateError(error.value());
    }
    // handle errors without error logging
    throwIfBad(status);

    return result;
}

}  // namespace opcua

#endif
