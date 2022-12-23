/*
 * Created by switchblade on 12/22/22.
 */

#pragma once

#include "define.hpp"

#ifndef TPP_USE_IMPORT

#include <cstring>

#if (__cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L))

#include <bit>

#endif

#endif

#if defined(TPP_DEBUG) || !defined(NDEBUG)

#ifndef TPP_USE_IMPORT

#include <algorithm>
#include <iterator>
#include <cstdlib>
#include <cstdio>
#include <array>

#endif

#if defined(__has_builtin) && !defined(__ibmxl__)
#if __has_builtin(__builtin_debugtrap)
#define TPP_DEBUG_TRAP() __builtin_debugtrap()
#elif __has_builtin(__debugbreak)
#define TPP_DEBUG_TRAP() __debugbreak()
#endif
#elif defined(_MSC_VER) || defined(__INTEL_COMPILER)
#define TPP_DEBUG_TRAP() __debugbreak()
#elif defined(__ARMCC_VERSION)
#define TPP_DEBUG_TRAP() __breakpoint(42)
#elif defined(__ibmxl__) || defined(__xlC__)
#include <builtins.h>
#define TPP_DEBUG_TRAP() __trap(42)
#elif defined(__DMC__) && defined(_M_IX86)
#define TPP_DEBUG_TRAP() (__asm int 3h)
#elif defined(__i386__) || defined(__x86_64__)
#define TPP_DEBUG_TRAP() (__asm__ __volatile__("int3"))
#elif defined(__STDC_HOSTED__) && (__STDC_HOSTED__ == 0) && defined(__GNUC__)
#define TPP_DEBUG_TRAP() __builtin_trap()
#endif

#ifndef TPP_DEBUG_TRAP
#ifndef TPP_USE_IMPORT

#ifndef TPP_USE_IMPORT

#include <csignal>

#endif

#endif
#if defined(SIGTRAP)
#define TPP_DEBUG_TRAP() raise(SIGTRAP)
#else
#define TPP_DEBUG_TRAP() raise(SIGABRT)
#endif
#endif

#if defined(_MSC_VER) || defined(__CYGWIN__)
#define TPP_PRETTY_FUNC __FUNCSIG__
#elif defined(__clang__) || defined(__GNUC__)
#define TPP_PRETTY_FUNC __PRETTY_FUNCTION__
#endif

#endif

namespace tpp::detail
{
#if defined(__cpp_lib_bit_cast) && __cpp_lib_bit_cast >= 201806L
	template<typename To, typename From>
	[[nodiscard]] constexpr TPP_FORCEINLINE To bit_cast(const From &from) noexcept { return std::bit_cast<To>(from); }
#elif defined(__has_builtin) && __has_builtin(__builtin_bit_cast)
	template<typename To, typename From>
	[[nodiscard]] constexpr TPP_FORCEINLINE To bit_cast(const From &from) noexcept { return __builtin_bit_cast(To, from); }
#else
	template<typename To, typename From>
	[[nodiscard]] inline TPP_FORCEINLINE To bit_cast(const From &from) noexcept
	{
		To to;
		std::memcpy(&to, &from, sizeof(To));
		return to;
	}
#endif
#ifdef TPP_ARCH_X86 /* On x86 unaligned MOV is allowed. */
	template<typename T>
	[[nodiscard]] inline TPP_FORCEINLINE std::enable_if_t<std::is_integral_v<T>, T> read_unaligned(const void *data) noexcept
	{
		return *static_cast<const T *>(data);
	}
#else /* On non-x86 platforms, unaligned access must be emulated */
	template<typename T>
	[[nodiscard]] inline TPP_FORCEINLINE std::enable_if_t<std::is_integral_v<T> && sizeof(T) == 1, T> read_unaligned(const void *data) noexcept
	{
		return *static_cast<const std::uint8_t *>(data);
	}
#if TPP_BYTE_ORDER == TPP_BYTE_ORDER_LE
	template<typename T>
	[[nodiscard]] inline TPP_FORCEINLINE std::enable_if_t<std::is_integral_v<T> && sizeof(T) == 8, T> read_unaligned(const void *data) noexcept
	{
		const auto *bytes = static_cast<const std::uint8_t *>(data);
		return (static_cast<T>(bytes[7]) << 56) |
			   (static_cast<T>(bytes[6]) << 48) |
			   (static_cast<T>(bytes[5]) << 40) |
			   (static_cast<T>(bytes[4]) << 32) |
			   (static_cast<T>(bytes[3]) << 24) |
			   (static_cast<T>(bytes[2]) << 16) |
			   (static_cast<T>(bytes[1]) << 8) |
			   (static_cast<T>(bytes[0]));
	}
	template<typename T>
	[[nodiscard]] inline TPP_FORCEINLINE std::enable_if_t<std::is_integral_v<T> && sizeof(T) == 4, T> read_unaligned(const void *data) noexcept
	{
		const auto *bytes = static_cast<const std::uint8_t *>(data);
		return (static_cast<T>(bytes[3]) << 24) |
			   (static_cast<T>(bytes[2]) << 16) |
			   (static_cast<T>(bytes[1]) << 8) |
			   (static_cast<T>(bytes[0]));
	}
	template<typename T>
	[[nodiscard]] inline TPP_FORCEINLINE std::enable_if_t<std::is_integral_v<T> && sizeof(T) == 2, T> read_unaligned(const void *data) noexcept
	{
		const auto *bytes = static_cast<const std::uint8_t *>(data);
		return (static_cast<T>(bytes[1]) << 8) |
			   (static_cast<T>(bytes[0]));
	}
#else
	template<typename T>
	[[nodiscard]] inline TPP_FORCEINLINE std::enable_if_t<std::is_integral_v<T> && sizeof(T) == 8, T> read_unaligned(const void *data) noexcept
	{
		const auto *bytes = static_cast<const std::uint8_t *>(data);
		return (static_cast<T>(bytes[0]) << 56) |
			   (static_cast<T>(bytes[1]) << 48) |
			   (static_cast<T>(bytes[2]) << 40) |
			   (static_cast<T>(bytes[3]) << 32) |
			   (static_cast<T>(bytes[4]) << 24) |
			   (static_cast<T>(bytes[5]) << 16) |
			   (static_cast<T>(bytes[6]) << 8) |
			   (static_cast<T>(bytes[7]));
	}
	template<typename T>
	[[nodiscard]] inline TPP_FORCEINLINE std::enable_if_t<std::is_integral_v<T> && sizeof(T) == 4, T> read_unaligned(const void *data) noexcept
	{
		const auto *bytes = static_cast<const std::uint8_t *>(data);
		return (static_cast<T>(bytes[0]) << 24) |
			   (static_cast<T>(bytes[1]) << 16) |
			   (static_cast<T>(bytes[2]) << 8) |
			   (static_cast<T>(bytes[3]));
	}
	template<typename T>
	[[nodiscard]] inline TPP_FORCEINLINE std::enable_if_t<std::is_integral_v<T> && sizeof(T) == 2, T> read_unaligned(const void *data) noexcept
	{
		const auto *bytes = static_cast<const std::uint8_t *>(data);
		return (static_cast<T>(bytes[0]) << 8) |
			   (static_cast<T>(bytes[1]));
	}
#endif
#endif

#if TPP_BYTE_ORDER == TPP_BYTE_ORDER_LE
	template<typename T>
	[[nodiscard]] inline TPP_FORCEINLINE std::enable_if_t<std::is_integral_v<T> && sizeof(T) == 8, T> read_buffer(const void *data, std::size_t n) noexcept
	{
		switch (const auto *bytes = static_cast<const std::uint8_t *>(data); n)
		{
			case 0: return 0;
			case 1: return static_cast<T>(read_unaligned<std::uint8_t>(bytes));
			case 2: return static_cast<T>(read_unaligned<std::uint16_t>(bytes));
			case 3:
			{
				const auto h = static_cast<T>(read_unaligned<std::uint8_t>(bytes + 2));
				const auto l = static_cast<T>(read_unaligned<std::uint16_t>(bytes));
				return l | (h << 16);
			}
			case 4: return static_cast<T>(read_unaligned<std::uint32_t>(bytes));
			case 5:
			{
				const auto h = static_cast<T>(read_unaligned<std::uint8_t>(bytes + 4));
				const auto l = static_cast<T>(read_unaligned<std::uint32_t>(bytes));
				return l | (h << 32);
			}
			case 6:
			{
				const auto h = static_cast<T>(read_unaligned<std::uint16_t>(bytes + 4));
				const auto l = static_cast<T>(read_unaligned<std::uint32_t>(bytes));
				return l | (h << 32);
			}
			case 7:
			{
				const auto a = static_cast<T>(read_unaligned<std::uint16_t>(bytes + 4));
				const auto b = static_cast<T>(read_unaligned<std::uint8_t>(bytes + 6));
				const auto c = static_cast<T>(read_unaligned<std::uint32_t>(bytes));
				return c | (a << 32) | (b << 48);
			}
			case 8: return read_unaligned<T>(bytes);
			default: TPP_UNREACHABLE();
		}
	}
	template<typename T>
	[[nodiscard]] inline TPP_FORCEINLINE std::enable_if_t<std::is_integral_v<T> && sizeof(T) == 4, T> read_buffer(const void *data, std::size_t n) noexcept
	{
		switch (const auto *bytes = static_cast<const std::uint8_t *>(data); n)
		{
			case 0: return 0;
			case 1: return static_cast<T>(read_unaligned<std::uint8_t>(bytes));
			case 2: return static_cast<T>(read_unaligned<std::uint16_t>(bytes));
			case 3:
			{
				const auto h = static_cast<T>(read_unaligned<std::uint8_t>(bytes + 2));
				const auto l = static_cast<T>(read_unaligned<std::uint16_t>(bytes));
				return l | (h << 16);
			}
			case 4: return read_unaligned<T>(bytes);
			default: TPP_UNREACHABLE();
		}
	}
#else
	template<typename T>
	[[nodiscard]] inline TPP_FORCEINLINE std::enable_if_t<std::is_integral_v<T> && sizeof(T) == 8, T> read_buffer(const void *data, std::size_t n) noexcept
	{
		switch (const auto *bytes = static_cast<const std::uint8_t *>(data); n)
		{
			case 0: return 0;
			case 1: return static_cast<T>(read_unaligned<std::uint8_t>(bytes));
			case 2: return static_cast<T>(read_unaligned<std::uint16_t>(bytes));
			case 3:
			{
				const auto l = static_cast<T>(read_unaligned<std::uint8_t>(bytes + 2));
				const auto h = static_cast<T>(read_unaligned<std::uint16_t>(bytes));
				return l | (h << 8);
			}
			case 4: return static_cast<T>(read_unaligned<std::uint32_t>(bytes));
			case 5:
			{
				const auto l = static_cast<T>(read_unaligned<std::uint8_t>(bytes + 4));
				const auto h = static_cast<T>(read_unaligned<std::uint32_t>(bytes));
				return l | (h << 8);
			}
			case 6:
			{
				const auto l = static_cast<T>(read_unaligned<std::uint16_t>(bytes + 4));
				const auto h = static_cast<T>(read_unaligned<std::uint32_t>(bytes));
				return l | (h << 16);
			}
			case 7:
			{
				const auto a = static_cast<T>(read_unaligned<std::uint16_t>(bytes + 4));
				const auto b = static_cast<T>(read_unaligned<std::uint8_t>(bytes + 6));
				const auto c = static_cast<T>(read_unaligned<std::uint32_t>(bytes));
				return b | (a << 8) | (c << 24);
			}
			case 8: return read_unaligned<T>(data);
			default: TPP_UNREACHABLE();
		}
	}
	template<typename T>
	[[nodiscard]] inline TPP_FORCEINLINE std::enable_if_t<std::is_integral_v<T> && sizeof(T) == 4, T> read_buffer(const void *data, std::size_t n) noexcept
	{
		switch (const auto *bytes = static_cast<const std::uint8_t *>(data); n)
		{
			case 0: return 0;
			case 1: return static_cast<T>(read_unaligned<std::uint8_t>(bytes));
			case 2: return static_cast<T>(read_unaligned<std::uint16_t>(bytes));
			case 3:
			{
				const auto l = static_cast<T>(read_unaligned<std::uint8_t>(bytes + 2));
				const auto h = static_cast<T>(read_unaligned<std::uint16_t>(bytes));
				return l | (h << 8);
			}
			case 4: return read_unaligned<T>(data);
			default: TPP_UNREACHABLE();
		}
	}
#endif

#if defined(TPP_DEBUG) || !defined(NDEBUG)
	[[maybe_unused]] inline void assert_msg(bool cnd, const char *cstr, const char *file, std::size_t line, const char *func, const char *msg) noexcept
	{
		if (!cnd)
		{
			printf("Assertion (%s) failed at '%s:%zu' in '%s'", cstr, file, line, func);
			if (msg) printf("%s", msg);
			TPP_DEBUG_TRAP();
			std::abort();
		}
	}
#endif

	template<std::size_t, typename T>
	[[nodiscard]] constexpr decltype(auto) forward_n(T &&value) noexcept { return std::forward<T>(value); }

	template<std::size_t... Is, typename T>
	[[nodiscard]] inline std::array<T, sizeof...(Is)> make_array(std::index_sequence<Is...>, const T &value) { return {forward_n<Is>(value)...}; }
	template<std::size_t Size, typename T>
	[[nodiscard]] inline std::array<T, Size> make_array(const T &value) noexcept { return make_array(std::make_index_sequence<Size>{}, value); }

	template<typename Iter, typename Size>
	[[nodiscard]] inline Size max_distance_or_n(const Iter &first, const Iter &last, Size n) noexcept
	{
		if constexpr (std::is_base_of_v<std::random_access_iterator_tag, typename std::iterator_traits<Iter>::iterator_category>)
			return std::max(static_cast<Size>(std::distance(first, last)), n);
		else
			return n;
	}

	template<typename Alloc>
	[[nodiscard]] inline bool allocator_eq(const Alloc &a, const Alloc &b)
	{
		if constexpr (!std::allocator_traits<Alloc>::is_always_equal::value)
			return a == b;
		else
			return true;
	}

	template<typename A, typename T>
	inline void relocate(A &alloc_src, T *src, A &alloc_dst, T *dst)
	{
		std::allocator_traits<A>::construct(alloc_dst, dst, std::move(*src));
		std::allocator_traits<A>::destroy(alloc_src, src);
	}
	template<typename A, typename T, typename F>
	inline void relocate(A &alloc_src, T *src_first, T *src_last, A &alloc_dst, T *dst_first, F rel = relocate<A, T>)
	{
		while (src_first != src_last) rel(alloc_src, src_first++, alloc_dst, dst_first++);
	}

	template<typename A, typename T, typename SizeT = typename std::allocator_traits<A>::size_type>
	inline std::pair<T *, SizeT> realloc_buffer(A &alloc, std::pair<T *, SizeT> buff, SizeT new_size)
	{
		if (buff.second < new_size)
		{
			if (buff.first) std::allocator_traits<A>::deallocate(alloc, buff.first, buff.second);
			buff.first = std::allocator_traits<A>::allocate(alloc, buff.second = new_size);
		}
		return buff;
	}
	template<typename A, typename T, typename SizeT = typename std::allocator_traits<A>::size_type>
	inline void realloc_buffer(A &alloc, T *&buff, SizeT &size, SizeT new_size)
	{
		const auto new_buff = realloc_buffer(alloc, std::pair<T *, SizeT>{buff, size}, new_size);
		buff = new_buff.first;
		size = new_buff.second;
	}
}

#if defined(TPP_DEBUG) || !defined(NDEBUG)
#define TPP_ASSERT(cnd, msg) tpp::detail::assert_msg(cnd, (#cnd), (__FILE__), (__LINE__), (TPP_PRETTY_FUNC), (msg))
#else
#define TPP_ASSERT(cnd, msg) TPP_ASSUME(cnd)
#endif