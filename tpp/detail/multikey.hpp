/*
 * Created by switchblade on 12/14/22.
 */

#pragma once

#include "define.hpp"

#ifndef TPP_USE_IMPORT

#include <tuple>

#endif

namespace tpp
{
	/** @brief Helper structure used to specify keys of a multiset. */
	template<typename... Ks>
	struct multikey { static_assert(sizeof...(Ks) != 0, "Multikey must have at least one key type"); };

	namespace detail
	{
		template<typename, typename>
		struct multikey_alloc;
		template<typename>
		struct multikey_hash;
		template<typename>
		struct multikey_eq;

		template<typename T, typename U, typename... Us>
		struct is_pack_element : is_pack_element<T, Us...> {};
		template<typename T, typename... Us>
		struct is_pack_element<T, T, Us...> : std::true_type {};
		template<typename T>
		struct is_pack_element<T, T> : std::true_type {};
		template<typename T, typename U>
		struct is_pack_element<T, U> : std::false_type {};

		template<typename... Ks, typename Mapped>
		struct multikey_alloc<multikey<Ks...>, Mapped> { using type = std::allocator<std::pair<std::tuple<Ks...>, Mapped>>; };
		template<typename... Ks>
		struct multikey_alloc<multikey<Ks...>, void> { using type = std::allocator<std::tuple<Ks...>>; };
		template<typename Mk, typename Mapped>
		using multikey_alloc_t = typename multikey_alloc<Mk, Mapped>::type;

		template<typename... Ks>
		struct multikey_hash<multikey<Ks...>>
		{
			template<typename T, typename = std::enable_if_t<is_pack_element<T, Ks...>::value>>
			[[nodiscard]] constexpr auto operator()(const T &key) const { return default_hash<T>{}(key); }
		};
		template<typename... Ks>
		struct multikey_eq<multikey<Ks...>>
		{
			template<typename T, typename = std::enable_if_t<is_pack_element<T, Ks...>::value>>
			[[nodiscard]] constexpr auto operator()(const T &a, const T &b) const { return std::equal_to<T>{}(a, b); }
		};
	}
}