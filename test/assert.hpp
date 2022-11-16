/*
 * Created by switchblade on 23/09/22
 */

#pragma once

#include <tpp/detail/common.hpp>
#include <exception>
#include <utility>
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

inline void test_assert(bool cnd, const char *file, std::size_t line, const char *func, const char *cstr, const char *msg) noexcept
{
	if (!cnd)[[unlikely]]
	{
		fprintf(stderr, "Assertion ");
		if (cstr) fprintf(stderr, "(%s) ", cstr);

		fprintf(stderr, "failed at '%s:%lu' in '%s'", file, line, func);
		if (msg) fprintf(stderr, ": %s", msg);
		fputc('\n', stderr);

		TEST_DEBUG_TRAP();
		std::terminate();
	}
}

#define TEST_ASSERT_2(cnd, msg) test_assert((cnd), (__FILE__), (__LINE__), (PRETTY_FUNC), (#cnd), (msg))
#define TEST_ASSERT_1(cnd) TEST_ASSERT_2(cnd, nullptr)

#define GET_MACRO_2(_1, _2, MACRO, ...) MACRO
#define TEST_ASSERT(...) GET_MACRO_2(__VA_ARGS__, TEST_ASSERT_2, TEST_ASSERT_1)(__VA_ARGS__)