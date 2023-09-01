#include "open62541pp/Crypto.h"

#ifdef UA_ENABLE_ENCRYPTION

#include <string>
#include <string_view>
#include <vector>

#include "open62541pp/ErrorHandling.h"
#include "open62541pp/Logger.h"
#include "open62541pp/TypeWrapper.h"

#include "CustomLogger.h"
#include "open62541_impl.h"

namespace opcua::crypto {

#ifdef UAPP_CREATE_CERTIFICATE

static_assert(static_cast<int>(CertificateFormat::DER) == UA_CERTIFICATEFORMAT_DER);
static_assert(static_cast<int>(CertificateFormat::PEM) == UA_CERTIFICATEFORMAT_PEM);

CreateCertificateResult createCertificate(
    Span<const String> subject,
    Span<const String> subjectAltName,
    size_t keySizeBits,
    CertificateFormat certificateFormat
) {
    if (subject.empty() || subjectAltName.empty()) {
        throw CreateCertificateError("Argument subject or subjectAltName is empty");
    }

    // OpenSSL errors will generate a generic UA_STATUSCODE_BADINTERNALERROR status code
    // detailed errors are reported through error log messages -> capture log messages
    UA_Logger logger{};
    CustomLogger customLogger(logger);
    std::vector<std::string> errorMessages;
    customLogger.setLogger(
        [&](LogLevel level, [[maybe_unused]] LogCategory category, std::string_view msg) {
            if (level >= LogLevel::Error) {
                errorMessages.emplace_back(msg);
            }
        }
    );

    CreateCertificateResult result;
    const auto status = UA_CreateCertificate(
        &logger,
        asNative(subject.data()),
        subject.size(),
        asNative(subjectAltName.data()),
        subjectAltName.size(),
        keySizeBits,
        static_cast<UA_CertificateFormat>(certificateFormat),
        result.privateKey.handle(),
        result.certificate.handle()
    );

    if (!errorMessages.empty()) {
        throw CreateCertificateError(errorMessages.front());  // throw first error
    }
    // handle errors without error logging
    detail::throwOnBadStatus(status);

    return result;
}

#endif  // ifdef UAPP_CREATE_CERTIFICATE

}  // namespace opcua::crypto

#endif  // ifdef UA_ENABLE_ENCRYPTION
