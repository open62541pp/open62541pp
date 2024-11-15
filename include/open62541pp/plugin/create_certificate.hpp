#pragma once

#include "open62541pp/config.hpp"
#include "open62541pp/span.hpp"
#include "open62541pp/types.hpp"

#if UAPP_HAS_CREATE_CERTIFICATE

namespace opcua {

enum class CertificateFormat {
    DER,
    PEM,
};

struct CreateCertificateResult {
    ByteString privateKey;
    ByteString certificate;
};

/**
 * Create a self-signed X.509 v3 certificate.
 *
 * It is recommended to store the generated certificate on disk for reuse, so the application can be
 * recognized across several executions.
 *
 * @note Only available with open62541 >= v1.3 and OpenSSL/LibreSSL
 *
 * @param subject Elements for the subject,
 *        e.g. {"C=DE", "O=SampleOrganization", "CN=Open62541Server@localhost"}
 * @param subjectAltName Elements for SubjectAltName,
 *        e.g. {"DNS:localhost", "URI:urn:open62541.server.application"}
 * @param certificateFormat Certificate format, either DER or PEM
 *
 * @exception BadStatus (BadOutOfMemory)
 * @exception CreateCertificateError
 *
 * @see UA_CreateCertificate
 */
CreateCertificateResult createCertificate(
    Span<const String> subject,
    Span<const String> subjectAltName,
    CertificateFormat certificateFormat = CertificateFormat::DER
);

}  // namespace opcua

#endif  // if UAPP_HAS_CREATE_CERTIFICATE
