/*
 * Created by switchblade on 2022-11-14.
 */

#pragma once

#include "arch.hpp"

#if !defined(NDEBUG) && !defined(TPP_DEBUG)
#define TPP_DEBUG
#endif

#include <type_traits>
#include <utility>
#include <cstdint>
#include <cstddef>

#if (__cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L))
#include <version>
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

#if defined(_MSC_VER)
#define TPP_FORCEINLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define TPP_FORCEINLINE __attribute__((always_inline))
#else
#define TPP_FORCEINLINE
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