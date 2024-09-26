#include "open62541pp/plugin/create_certificate.hpp"

#ifdef UA_ENABLE_ENCRYPTION

#include <optional>
#include <string>
#include <string_view>

#include "open62541pp/config.hpp"
#include "open62541pp/detail/open62541/common.h"
#include "open62541pp/exception.hpp"
#include "open62541pp/plugin/log.hpp"
#include "open62541pp/typewrapper.hpp"

#include "log_default.hpp"

namespace opcua::crypto {

#ifdef UAPP_CREATE_CERTIFICATE

static_assert(static_cast<int>(CertificateFormat::DER) == UA_CERTIFICATEFORMAT_DER);
static_assert(static_cast<int>(CertificateFormat::PEM) == UA_CERTIFICATEFORMAT_PEM);

CreateCertificateResult createCertificate(
    Span<const String> subject,
    Span<const String> subjectAltName,
    CertificateFormat certificateFormat
) {
    if (subject.empty() || subjectAltName.empty()) {
        throw CreateCertificateError("Argument subject or subjectAltName is empty");
    }

    // OpenSSL errors will generate a generic UA_STATUSCODE_BADINTERNALERROR status code
    // detailed errors are reported through error log messages -> capture log messages
    std::optional<std::string> error;
    LoggerAdapter loggerAdapter(
        [&](LogLevel level, [[maybe_unused]] LogCategory category, std::string_view msg) {
            if (level >= LogLevel::Error && !error /* keep first error */) {
                error = msg;
            }
        }
    );
    const UA_Logger logger = loggerAdapter.create();

    CreateCertificateResult result;
    const auto status = UA_CreateCertificate(
        &logger,
        asNative(subject.data()),
        subject.size(),
        asNative(subjectAltName.data()),
        subjectAltName.size(),
#if UAPP_OPEN62541_VER_LE(1, 3)
        0,  // keySizeBits, 0 = maximum
#endif
        static_cast<UA_CertificateFormat>(certificateFormat),
#if UAPP_OPEN62541_VER_GE(1, 4)
        nullptr,  // key value map with optional parameters
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

#endif  // ifdef UAPP_CREATE_CERTIFICATE

}  // namespace opcua::crypto

#endif  // ifdef UA_ENABLE_ENCRYPTION
