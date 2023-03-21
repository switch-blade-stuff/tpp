/*
 * Created by switchblade on 2022-11-14.
 */

#pragma once

#include "arch.hpp"

#if !defined(NDEBUG) && !defined(TPP_DEBUG)
#define TPP_DEBUG
#endif

/* Define TPP_USE_IMPORT only if modules are enabled and supported by the compiler. */
#if defined(TPP_USE_MODULES) && defined(__cpp_modules)
#define TPP_USE_IMPORT
#endif

#ifdef TPP_USE_IMPORT

/* If we are not on MSVC or C++ version at least C++23 use `import std`. Otherwise, use `import std.core`. */
#if defined(_MSC_VER) && (__cplusplus <= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG <= 202002L))

import std.core;

#else

import std;

#endif

#else

#include <type_traits>

#if (__cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L))

#include <version>

#endif

#include <utility>
#include <cstdint>
#include <cstddef>

#endif

#if (__cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L))
#define TPP_REQUIRES(cnd) requires cnd
#else
#define TPP_REQUIRES(cnd)
#endif

#if defined(__has_cpp_attribute) && __has_cpp_attribute(assume)
#define TPP_ASSUME(x) [[assume(x)]]
#elif defined(_MSC_VER)
#define TPP_ASSUME(x) __assume(x)
#elif defined(__clang__) && 0 /* CLang complains about side effects when __builtin_assume is used */
#define TPP_ASSUME(x) __builtin_assume(x)
#elif defined(__GNUC__)
#define TPP_ASSUME(x) if (!(x)) __builtin_unreachable()
#else
#define TPP_ASSUME(x)
#endif

#if defined(__GNUC__) || defined(__clang__)
#define TPP_UNREACHABLE() __builtin_unreachable()
#elif defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
#define TPP_UNREACHABLE() std::unreachable()
#else
#define TPP_UNREACHABLE() TPP_ASSUME(false)
#endif

#if defined(_MSC_VER)
#define TPP_FORCEINLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define TPP_FORCEINLINE __attribute__((always_inline))
#else
#define TPP_FORCEINLINE
#endif

#if defined(__GNUC__) || defined(__clang__)
#define TPP_PURE __attribute__((pure))
#else
#define TPP_PURE
#endif

#if (__cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L))

#define TPP_IF_LIKELY(x) if (x) [[likely]]
#define TPP_IF_UNLIKELY(x) if (x) [[unlikely]]

#elif defined(__GNUC__) || defined(__clang__)

#define TPP_IF_LIKELY(x) if (__builtin_expect(static_cast<bool>(x), true))
#define TPP_IF_UNLIKELY(x) if (__builtin_expect(static_cast<bool>(x), false))

#else

#define TPP_IF_LIKELY(x) if (x)
#define TPP_IF_UNLIKELY(x) if (x)

#endif