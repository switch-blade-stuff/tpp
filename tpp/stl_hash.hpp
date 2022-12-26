/*
 * Created by switchblade on 11/7/22.
 */

#include "hash.hpp"

#ifndef TPP_NO_HASH

#if !defined(TPP_HAS_OPTIONAL_HASH) && (defined(TPP_STL_HASH_ALL) || defined(TPP_OPTIONAL_HASH))
#define TPP_HAS_OPTIONAL_HASH

#ifndef TPP_USE_IMPORT

#include <optional>

#endif

template<typename T, std::size_t (*Algo)(const void *, std::size_t)>
struct tpp::hash<std::optional<T>, Algo>
{
	using value_hash = hash<T, Algo>;

	/* Use a sentinel value for nullopt. Value taken from `https://github.com/gcc-mirror/gcc/blob/c838119946c9f75f1e42f4320275355822cc86fc/libstdc%2B%2B-v3/include/std/optional#L1480` */
	static constexpr auto nullopt_sentinel = static_cast<std::size_t>(-3333);

	[[nodiscard]] std::size_t operator()(const std::optional<T> &value) const noexcept(noexcept(value_hash{}(*value)))
	{
		return value ? value_hash{}(*value) : nullopt_sentinel;
	}
};

#endif

#if !defined(TPP_HAS_VARIANT_HASH) && (defined(TPP_STL_HASH_ALL) || defined(TPP_VARIANT_HASH))
#define TPP_HAS_VARIANT_HASH

#ifndef TPP_USE_IMPORT

#include <variant>

#endif

namespace tpp
{
	namespace detail
	{
		template<typename... Ts>
		struct hash_overload : public Ts ... { constexpr hash_overload() : Ts()... {} using Ts::operator()...; };

		template<typename... Ts> hash_overload(Ts...) -> hash_overload<Ts...>;
	}

	template<std::size_t (*Algo)(const void *, std::size_t)>
	struct hash<std::monostate, Algo>
	{
		[[nodiscard]] constexpr std::size_t operator()(const std::monostate &) const noexcept
		{
			/* Value taken from `https://github.com/gcc-mirror/gcc/blob/c838119946c9f75f1e42f4320275355822cc86fc/libstdc%2B%2B-v3/include/std/variant#L1929` */
			return static_cast<std::size_t>(-7777);
		}
	};

	template<typename... Ts, std::size_t (*Algo)(const void *, std::size_t)>
	struct hash<std::variant<Ts...>, Algo>
	{
		[[nodiscard]] std::size_t operator()(const std::variant<Ts...> &value) const
		noexcept((std::is_nothrow_invocable_v<hash<std::decay_t<Ts>, Algo>, Ts> && ...))
		{
			using visitor_hash = detail::hash_overload<hash<std::decay_t<Ts>, Algo>...>;
			if (value.index() >= sizeof...(Ts))
				return hash<std::monostate, Algo>{}({});
			else
				return std::visit(visitor_hash{}, value);
		}
	};
}

#endif

#if !defined(TPP_HAS_STRING_VIEW_HASH) && (defined(TPP_STL_HASH_ALL) || defined(TPP_STRING_VIEW_HASH))
#define TPP_HAS_STRING_VIEW_HASH

#ifndef TPP_USE_IMPORT

#include <string_view>

#endif

#define TPP_STRING_VIEW_HASH_IMPL(C)                                                \
    template<std::size_t (*Algo)(const void *, std::size_t)>                        \
    struct tpp::hash<std::basic_string_view<C>, Algo>                               \
    {                                                                               \
        using value_t = std::basic_string_view<C>;                                  \
                                                                                    \
        [[nodiscard]] std::size_t operator()(const value_t &value) const noexcept   \
        {                                                                           \
            return Algo(value.data(), value.size() * sizeof(C));                    \
        }                                                                           \
    };

TPP_STRING_VIEW_HASH_IMPL(char)
TPP_STRING_VIEW_HASH_IMPL(wchar_t)
TPP_STRING_VIEW_HASH_IMPL(char16_t)
TPP_STRING_VIEW_HASH_IMPL(char32_t)

#if defined(__cpp_char8_t) && __cpp_char8_t >= 201811L

TPP_STRING_VIEW_HASH_IMPL(char8_t)

#endif

#undef TPP_STRING_VIEW_HASH_IMPL

#endif

#if !defined(TPP_HAS_STRING_HASH) && (defined(TPP_STL_HASH_ALL) || defined(TPP_STRING_HASH) || defined(TPP_STRING_VIEW_HASH))
#define TPP_HAS_STRING_HASH

#ifndef TPP_USE_IMPORT

#include <string>

#endif

#define TPP_STRING_HASH_IMPL(C)                                                     \
    template<typename Alloc, std::size_t (*Algo)(const void *, std::size_t)>        \
    struct tpp::hash<std::basic_string<C, std::char_traits<C>, Alloc>, Algo>        \
    {                                                                               \
        using value_t = std::basic_string<C, std::char_traits<C>, Alloc>;           \
                                                                                    \
        [[nodiscard]] std::size_t operator()(const value_t &value) const noexcept   \
        {                                                                           \
            return Algo(value.data(), value.size() * sizeof(C));                    \
        }                                                                           \
    };

TPP_STRING_HASH_IMPL(char)
TPP_STRING_HASH_IMPL(wchar_t)
TPP_STRING_HASH_IMPL(char16_t)
TPP_STRING_HASH_IMPL(char32_t)

#if defined(__cpp_char8_t) && __cpp_char8_t >= 201811L

TPP_STRING_HASH_IMPL(char8_t)

#endif

#undef TPP_STRING_HASH_IMPL

#endif

#if !defined(TPP_HAS_BITSET_HASH) && (defined(TPP_STL_HASH_ALL) || defined(TPP_BITSET_HASH))
#define TPP_HAS_BITSET_HASH

#ifndef TPP_USE_IMPORT

#include <bitset>

#endif

template<std::size_t N, std::size_t (*Algo)(const void *, std::size_t)>
struct tpp::hash<std::bitset<N>, Algo>
{
	[[nodiscard]] std::size_t operator()(const std::bitset<N> &value) const noexcept
	{
		if constexpr (N == 0)
			return 0;
		else if constexpr (N <= sizeof(long long) * 8)
			return hash<long long, Algo>{}(static_cast<long long>(value.to_ullong()));
		else
		{
			/* Technically, this is UB. However, there is no standard way to get raw memory of a bitset. */
			return Algo(&value, sizeof(std::bitset<N>));
		}
	}
};

#endif

#if !defined(TPP_HAS_SYSTEM_ERROR_HASH) && (defined(TPP_STL_HASH_ALL) || defined(TPP_SYSTEM_ERROR_HASH))
#define TPP_HAS_SYSTEM_ERROR_HASH

#ifndef TPP_USE_IMPORT

#include <system_error>

#endif

template<std::size_t (*Algo)(const void *, std::size_t)>
struct tpp::hash<std::error_condition, Algo>
{
	[[nodiscard]] std::size_t operator()(const std::error_condition &e) const noexcept
	{
		using cat_hash = hash<const std::error_category *, Algo>;
		const auto seed = hash<int, Algo>{}(e.value());
		return hash_combine(seed, &e.category(), cat_hash{});
	}
};

template<std::size_t (*Algo)(const void *, std::size_t)>
struct tpp::hash<std::error_code, Algo>
{
	[[nodiscard]] std::size_t operator()(const std::error_code &e) const noexcept
	{
		using cat_hash = hash<const std::error_category *, Algo>;
		const auto seed = hash<int, Algo>{}(e.value());
		return hash_combine(seed, &e.category(), cat_hash{});
	}
};

#endif

#if !defined(TPP_HAS_MEMORY_HASH) && (defined(TPP_STL_HASH_ALL) || defined(TPP_MEMORY_HASH))
#define TPP_HAS_MEMORY_HASH

#ifdef TPP_USE_IMPORT

#if defined(_MSC_VER) && (__cplusplus <= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG <= 202002L))

import std.memory;

#endif

#else

#include <memory>

#endif

template<typename T, typename D, std::size_t (*Algo)(const void *, std::size_t)>
struct tpp::hash<std::unique_ptr<T, D>, Algo>
{
	[[nodiscard]] std::size_t operator()(const std::unique_ptr<T, D> &p) const noexcept
	{
		using ptr_hash = hash<const typename std::unique_ptr<T, D>::pointer *, Algo>;
		return ptr_hash{}(p.get());
	}
};

template<typename T, std::size_t (*Algo)(const void *, std::size_t)>
struct tpp::hash<std::shared_ptr<T>, Algo>
{
	[[nodiscard]] std::size_t operator()(const std::shared_ptr<T> &p) const noexcept
	{
		using ptr_hash = hash<const typename std::shared_ptr<T>::pointer *, Algo>;
		return ptr_hash{}(p.get());
	}
};

#endif

#if !defined(TPP_HAS_TYPEINDEX_HASH) && (defined(TPP_STL_HASH_ALL) || defined(TPP_TYPEINDEX_HASH))
#define TPP_HAS_TYPEINDEX_HASH

#ifndef TPP_USE_IMPORT

#include <typeindex>

#endif

template<std::size_t (*Algo)(const void *, std::size_t)>
struct tpp::hash<std::type_index, Algo>
{
	[[nodiscard]] std::size_t operator()(const std::type_index &i) const noexcept
	{
		/* type_index is already unique. */
		return i.hash_code();
	}
};

#endif

#if !defined(TPP_HAS_FILESYSTEM_HASH) && (defined(TPP_STL_HASH_ALL) || defined(TPP_FILESYSTEM_HASH))
#define TPP_HAS_FILESYSTEM_HASH

#ifdef TPP_USE_IMPORT

#if defined(_MSC_VER) && (__cplusplus <= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG <= 202002L))
import std.filesystem;
#endif

#else

#include <filesystem>

#endif

template<std::size_t (*Algo)(const void *, std::size_t)>
struct tpp::hash<std::filesystem::path, Algo>
{
	[[nodiscard]] std::size_t operator()(const std::filesystem::path &value) const noexcept
	{
		const hash<typename std::filesystem::path::string_type, Algo> str_hash = {};
		std::size_t result = 0;
		for (const auto &p: value)
			result = hash_combine(result, p.native(), str_hash);
		return result;
	}
};

#endif

#if !defined(TPP_HAS_THREAD_HASH) && (defined(TPP_STL_HASH_ALL) || defined(TPP_THREAD_HASH))
#define TPP_HAS_THREAD_HASH

#ifdef TPP_USE_IMPORT

#if defined(_MSC_VER) && (__cplusplus <= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG <= 202002L))
import std.threading;
#endif

#else

#include <thread>

#endif

template<std::size_t (*Algo)(const void *, std::size_t)>
struct tpp::hash<std::thread::id, Algo>
{
	[[nodiscard]] std::size_t operator()(const std::thread::id &value) const noexcept
	{
		/* If the size of thread::id fits into std::size_t, use its raw value directly. Otherwise, byte-hash the raw value.
		 * All of this is technically UB, however there is no standard way to obtain the underlying value of thread::id. */
		if constexpr (sizeof(std::thread::id) <= sizeof(std::size_t))
		{
			std::size_t result = 0;
			*((std::thread::id *) &result) = value;
			return result;
		}
		else
			return Algo(&value, sizeof(value));
	}
};

#endif

/* C++20 */

#if (__cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L)) && defined(TPP_STL_HASH_ALL) || defined(TPP_COROUTINE_HASH)

#ifndef TPP_USE_IMPORT

#include <coroutine>

#endif

#if !defined(TPP_HAS_COROUTINE_HASH) && defined(__cpp_lib_coroutine) && __cpp_lib_coroutine >= 201902L
#define TPP_HAS_COROUTINE_HASH

template<typename P, std::size_t (*Algo)(const void *, std::size_t)>
struct tpp::hash<std::coroutine_handle<P>, Algo>
{
	[[nodiscard]] std::size_t operator()(const std::coroutine_handle<P> &h) const noexcept
	{
		return reinterpret_cast<std::size_t>(h.address());
	}
};

#endif

#endif

/* C++23 */

#if (__cplusplus > 202002L || (defined(_MSVC_LANG) && _MSVC_LANG > 202002L)) && defined(TPP_STL_HASH_ALL) || defined(TPP_STACKTRACE_HASH)

#ifndef TPP_USE_IMPORT

#include <stacktrace>

#endif

#if !defined(TPP_HAS_STACKTRACE_HASH) && defined(__cpp_lib_stacktrace) && __cpp_lib_stacktrace >= 202011L
#define TPP_HAS_STACKTRACE_HASH

template<std::size_t (*Algo)(const void *, std::size_t)>
struct tpp::hash<std::stacktrace_entry, Algo>
{
	[[nodiscard]] std::size_t operator()(const std::stacktrace_entry &e) const noexcept
	{
		using handle_hash = hash<typename std::stacktrace_entry::native_handle_type, Algo>;
		return handle_hash{}(e.native_handle());
	}
};

template<typename A, std::size_t (*Algo)(const void *, std::size_t)>
struct tpp::hash<std::basic_stacktrace<A>, Algo>
{
	[[nodiscard]] std::size_t operator()(const std::stacktrace_entry &st) const noexcept
	{
		const hash<typename std::stacktrace_entry::native_handle_type, Algo> entry_hash;
		std::size_t result = hash<decltype(st.size()), Algo>{}(st.size());
		for (const auto &f : st) result = hash_combine(result, f, entry_hash);
		return result;
	}
};

#endif
#endif

#endif