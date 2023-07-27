#pragma once

#include "open62541pp/open62541.h"

// expose open62541 compile options defined in open62541/config.h

#ifndef UA_ENABLE_NODEMANAGEMENT
#warning "open62541 should be compiled with UA_ENABLE_NODEMANAGEMENT"
#endif

// open62541pp specific macros/defines

// NOLINTNEXTLINE
#define UAPP_OPEN62541_VER_GE(MAJOR, MINOR)                                                        \
    (UA_OPEN62541_VER_MAJOR >= (MAJOR)) && (UA_OPEN62541_VER_MINOR >= (MINOR))

// NOLINTNEXTLINE
#define UAPP_OPEN62541_VER_LE(MAJOR, MINOR)                                                        \
    (UA_OPEN62541_VER_MAJOR <= (MAJOR)) && (UA_OPEN62541_VER_MINOR <= (MINOR))

#if UAPP_OPEN62541_VER_GE(1, 3) &&                                                                 \
    (defined(UA_ENABLE_ENCRYPTION_OPENSSL) || defined(UA_ENABLE_ENCRYPTION_LIBRESSL))
#define UAPP_CREATE_CERTIFICATE
#endif
