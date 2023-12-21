#pragma once

// expose open62541 compile options defined in open62541/config.h
#include "open62541pp/open62541.h"

#ifndef UA_ENABLE_NODEMANAGEMENT
#warning "open62541 should be compiled with UA_ENABLE_NODEMANAGEMENT"
#endif

// open62541pp specific macros/defines

// constexpr destructor allowed since paper P0784R7
// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2019/p0784r7.html
#if __cpp_constexpr_dynamic_alloc >= 201907L
#define UAPP_CONSTEXPR_DTOR constexpr
#else
#define UAPP_CONSTEXPR_DTOR
#endif

// constexpr swap allowed since paper P0879R0
// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p0879r0.html
// https://cplusplus.github.io/LWG/issue3256
#if __cpp_lib_constexpr_algorithms >= 201806L
#define UAPP_CONSTEXPR_SWAP constexpr
#else
#define UAPP_CONSTEXPR_SWAP
#endif

// constexpr TypeWrapper needs constexpr destructor and std::is_constant_evaluated
#if (__cpp_constexpr_dynamic_alloc >= 201907L) && (__cpp_lib_is_constant_evaluated >= 201811L)
#define UAPP_CONSTEXPR_WRAPPER
#endif

// mock __has_feature for other compilers than clang
#ifndef __has_feature
#define __has_feature(x) 0
#endif

#if __has_feature(thread_sanitizer) || defined(__SANITIZE_THREAD__)
#define UAPP_TSAN_ENABLED
#endif

// NOLINTNEXTLINE
#define UAPP_OPEN62541_VER_EQ(MAJOR, MINOR)                                                        \
    (UA_OPEN62541_VER_MAJOR == (MAJOR)) && (UA_OPEN62541_VER_MINOR == (MINOR))

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
