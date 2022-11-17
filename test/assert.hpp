/*
 * Created by switchblade on 23/09/22
 */

#pragma once

#include <tpp/detail/common.hpp>
#include <cstdio>

#if defined(__has_builtin) && !defined(__ibmxl__)
#if __has_builtin(__builtin_debugtrap)
#define TEST_DEBUG_TRAP() __builtin_debugtrap()
#elif __has_builtin(__debugbreak)
#define TEST_DEBUG_TRAP() __debugbreak()
#endif
#elif defined(_MSC_VER) || defined(__INTEL_COMPILER)
#define TEST_DEBUG_TRAP() __debugbreak()
#elif defined(__ARMCC_VERSION)
#define TEST_DEBUG_TRAP() __breakpoint(42)
#elif defined(__ibmxl__) || defined(__xlC__)
#include <builtins.h>
#define TEST_DEBUG_TRAP() __trap(42)
#elif defined(__DMC__) && defined(_M_IX86)
#define TEST_DEBUG_TRAP() (__asm int 3h)
#elif defined(__i386__) || defined(__x86_64__)
#define TEST_DEBUG_TRAP() (__asm__ __volatile__("int3"))
#elif defined(__STDC_HOSTED__) && (__STDC_HOSTED__ == 0) && defined(__GNUC__)
#define TEST_DEBUG_TRAP() __builtin_trap()
#endif

#ifndef TEST_DEBUG_TRAP
#ifndef TPP_USE_IMPORT

#include <csignal>

#endif
#if defined(SIGTRAP)
#define TEST_DEBUG_TRAP() raise(SIGTRAP)
#else
#define TEST_DEBUG_TRAP() raise(SIGABRT)
#endif
#endif

#if defined(_MSC_VER) || defined(__CYGWIN__)
#define PRETTY_FUNC __FUNCSIG__
#elif defined(__clang__) || defined(__GNUC__)
#define PRETTY_FUNC __PRETTY_FUNCTION__
#endif

static inline void test_assert(bool cnd, const char *file, std::size_t line, const char *func, const char *cstr) noexcept
{
	if (!cnd)
	{
		fprintf(stderr, "Assertion (%s) failed at '%s:%lu' in '%s'\n", cstr, file, line, func);
		TEST_DEBUG_TRAP();
		TPP_UNREACHABLE();
	}
}

#define TEST_ASSERT(cnd) test_assert((cnd), (__FILE__), (__LINE__), (PRETTY_FUNC), (#cnd))