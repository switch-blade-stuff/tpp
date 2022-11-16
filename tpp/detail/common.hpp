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

#endif

/* Select `if consteval` alternative if possible when it is not supported. */
#if defined(__cpp_if_consteval) && __cpp_if_consteval >= 202106L
#define TPP_IF_CONSTEVAL(t, f) if consteval { t; } else { f; }
#elif defined(__cpp_lib_is_constant_evaluated)
#define TPP_IF_CONSTEVAL(t, f) if (std::is_constant_evaluated()) { t; } else { f; }
#elif defined(__GNUC__) || defined(__clang__) || defined(_MSC_VER)
#define TPP_IF_CONSTEVAL(t, f) if (__builtin_is_constant_evaluated()) { t; } else { f; }
#endif

#if __cplusplus >= 202002L
#define TPP_REQUIRES(cnd) requires cnd
#else
#define TPP_REQUIRES(cnd)
#endif