/*
 * Created by switchblade on 2022-12-28.
 */

#pragma once

#include "table_common.hpp"

namespace tpp::detail
{
	template<typename, typename = void, typename = void>
	struct pair_traits
	{
		template<typename T>
		static constexpr auto &get_key(T &value) noexcept { return value; }
		template<typename T>
		static constexpr auto &get_mapped(T &value) noexcept { return value; }
	};
	template<typename P>
	struct pair_traits<P, std::void_t<decltype(&P::first)>, std::void_t<decltype(&P::second)>>
	{
		template<typename T>
		static constexpr auto &get_key(T &value) noexcept { return value.first; }
		template<typename T>
		static constexpr auto &get_mapped(T &value) noexcept { return value.second; }
	};

	/* Value traits for stable maps & sets. Defined top-level to enable node compatibility across different template instances. */
	template<typename Value, typename Link>
	struct stable_value_traits : pair_traits<Value>
	{
		using is_stable = std::true_type;
		using link_type = Link;

		using pointer = Value *;
		using const_pointer = const Value *;
		using reference = Value &;
		using const_reference = const Value &;
	};
}