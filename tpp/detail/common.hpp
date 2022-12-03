/*
 * Created by switchblade on 11/14/22.
 */

#pragma once

/* Define TPP_USE_IMPORT only if modules are enabled and supported by the compiler. */
#if defined(TPP_USE_MODULES) && defined(__cpp_modules)
#define TPP_USE_IMPORT
#endif

#ifdef TPP_USE_IMPORT

/* If we are not on MSVC or C++ version at least C++23 use `import std`. Otherwise, use `import std.core`. */
#ifdef _MSC_VER || __cplusplus <= 202002L
import std.core;
#else
import std;
#endif

#else

#include <type_traits>
#include <utility>

#endif

/* Select `if consteval` alternative if possible when it is not supported. */
#if defined(__cpp_if_consteval) && __cpp_if_consteval >= 202106L
#define TPP_IF_CONSTEVAL if consteval
#elif defined(__cpp_lib_is_constant_evaluated)
#define TPP_IF_CONSTEVAL if (std::is_constant_evaluated())
#elif defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER)
#define TPP_IF_CONSTEVAL if (__builtin_is_constant_evaluated())
#else
#define TPP_IF_CONSTEVAL if (true)
#endif

#if __cplusplus >= 202002L
#define TPP_REQUIRES(cnd) requires cnd
#else
#define TPP_REQUIRES(cnd)
#endif

#if defined(__has_cpp_attribute) && __has_cpp_attribute(assume)
#define TPP_ASSUME(x) [[assume(x)]]
#elif defined(_MSC_VER)
#define TPP_ASSUME(x) __assume(x)
#elif defined(__clang__)
#define TPP_ASSUME(x) __builtin_assume(x)
#elif defined(__GNUC__)
#define TPP_ASSUME(x) if (!(x)) __builtin_unreachable()
#else
#define TPP_ASSUME(x)
#endif

#if defined(_MSC_VER)
#define TPP_UNREACHABLE() __assume(false)
#elif defined(__GNUC__) || defined(__clang__)
#define TPP_UNREACHABLE() __builtin_unreachable()
#elif defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
#define TPP_UNREACHABLE() std::unreachable()
#else
#define TPP_UNREACHABLE() TPP_ASSUME(false)
#endif

#if defined(_MSC_VER)
#define TPP_FORCEINLINE __forceinline
#elif defined(__GNUC__) || defined(__clang__)
#define TPP_FORCEINLINE __attribute__((always_inline)) inline
#else
#define TPP_FORCEINLINE inline
#endif

#ifndef TPP_NO_SIMD

#ifdef __SSE__
#define TPP_HAS_SSE
#if __SSE2__
#define TPP_HAS_SSE2
#endif
#endif

#ifdef __SSSE3__
#define TPP_HAS_SSSE3
#endif

#if defined(_MSC_VER) && defined(_M_IX86_FP)

#if !defined(TPP_HAS_SSE) && (_M_IX86_FP >= 1 || defined(_M_AMD64) || defined(_M_X64))
#define TPP_HAS_SSE
#endif

#if !defined(TPP_HAS_SSE2) && defined(TPP_HAS_SSE) && (_M_IX86_FP >= 2 || defined(_M_AMD64) || defined(_M_X64))
#define TPP_HAS_SSE2
#endif

#endif
#endif