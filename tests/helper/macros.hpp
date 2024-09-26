#pragma once

// mock __has_feature for other compilers than clang
#ifndef __has_feature
#define __has_feature(x) 0
#endif

#if __has_feature(thread_sanitizer) || defined(__SANITIZE_THREAD__)
#define UAPP_TSAN_ENABLED
#endif
