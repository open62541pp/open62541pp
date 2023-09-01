#pragma once

#include <cstddef>

#include "open62541pp/Config.h"
#include "open62541pp/Span.h"
#include "open62541pp/types/Builtin.h"

#ifdef UA_ENABLE_ENCRYPTION

namespace opcua::crypto {

#ifdef UAPP_CREATE_CERTIFICATE

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
 * recognized across several executions, e.g. with ByteString::toFile, ByteString::fromFile.
 *
 * @note Only available with open62541 >= v1.3 and OpenSSL/LibreSSL
 *
 * @param subject Elements for the subject,
 *        e.g. {"C=DE", "O=SampleOrganization", "CN=Open62541Server@localhost"}
 * @param subjectAltName Elements for SubjectAltName,
 *        e.g. {"DNS:localhost", "URI:urn:open62541.server.application"}
 * @param keySizeBits Size of the generated key in bits. If set to 0, the maximum key size is used.
 *        Possible values are: 0, 1024 (deprecated), 2048, 4096
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
    size_t keySizeBits = 2048,
    CertificateFormat certificateFormat = CertificateFormat::DER
);

#endif  // ifdef UAPP_CREATE_CERTIFICATE

}  // namespace opcua::crypto

#endif  // ifdef UA_ENABLE_ENCRYPTION
