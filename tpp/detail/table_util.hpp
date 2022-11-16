/*
 * Created by switchblade on 11/14/22.
 */

#pragma once

#include "common.hpp"

#ifndef TPP_USE_IMPORT

#include <iterator>
#include <utility>
#include <cstdint>

#else

#ifdef _MSC_VER
import std.memory;
#endif

#endif

namespace tpp::detail
{
	/* Helper type used to store potentially empty objects. */
	template<typename, typename = void>
	struct ebo_container;

	template<typename T>
	using ebo_candidate = std::conjunction<std::is_empty<T>, std::negation<std::is_final<T>>>;

	template<typename T, typename U>
	constexpr inline bool nothrow_assign = std::is_nothrow_assignable_v<T, U>;
	template<typename T, typename... Args>
	constexpr inline bool nothrow_ctor = std::is_nothrow_constructible_v<T, Args...>;

	template<typename T>
	struct ebo_container<T, std::enable_if_t<ebo_candidate<T>::value>> : T
	{
		constexpr ebo_container() noexcept(nothrow_ctor<T>) = default;
		constexpr ebo_container(const ebo_container &other) noexcept(nothrow_ctor<T, const T &>) = default;
		constexpr ebo_container(ebo_container &&other) noexcept(nothrow_ctor<T, T &&>) = default;
		constexpr ebo_container &operator=(const ebo_container &other) noexcept(nothrow_assign<T, const T &>) = default;
		constexpr ebo_container &operator=(ebo_container &&other) noexcept(nothrow_assign<T, T &&>) = default;

		template<typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
		constexpr ebo_container(Args &&...args) noexcept(nothrow_ctor<T, Args...>) : T(std::forward<Args>(args)...) {}

		template<typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
		constexpr ebo_container(std::tuple<Args...> args) noexcept(nothrow_ctor<T, Args...>)
				: ebo_container(std::make_index_sequence<sizeof...(Args)>(), std::forward<Args>(args)...) {}
		template<std::size_t... Is, typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
		constexpr ebo_container(std::index_sequence<Is...>, std::tuple<Args...> args) noexcept(nothrow_ctor<T, Args...>)
				: ebo_container(std::forward<Args>(std::get<Is>(args))...) {}

		[[nodiscard]] constexpr T &value() noexcept { return static_cast<T &>(*this); }
		[[nodiscard]] constexpr const T &value() const noexcept { return static_cast<const T &>(*this); }

		constexpr void swap(ebo_container &other) noexcept(std::is_nothrow_swappable_v<T>)
		{
			using std::swap;
			swap(value(), other.value());
		}
	};

	template<typename T>
	struct ebo_container<T, std::enable_if_t<!ebo_candidate<T>::value>>
	{
		constexpr ebo_container() noexcept(nothrow_ctor<T>) = default;
		constexpr ebo_container(const ebo_container &other) noexcept(nothrow_ctor<T, const T &>) = default;
		constexpr ebo_container(ebo_container &&other) noexcept(nothrow_ctor<T, T &&>) = default;
		constexpr ebo_container &operator=(const ebo_container &other) noexcept(nothrow_assign<T, const T &>) = default;
		constexpr ebo_container &operator=(ebo_container &&other) noexcept(nothrow_assign<T, T &&>) = default;

		template<typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
		constexpr ebo_container(Args &&...args) noexcept(nothrow_ctor<T, Args...>) : m_value(std::forward<Args>(args)...) {}

		template<typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
		constexpr ebo_container(std::tuple<Args...> args) noexcept(nothrow_ctor<T, Args...>)
				: ebo_container(std::make_index_sequence<sizeof...(Args)>(), std::forward<Args>(args)...) {}
		template<std::size_t... Is, typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
		constexpr ebo_container(std::index_sequence<Is...>, std::tuple<Args...> args) noexcept(nothrow_ctor<T, Args...>)
				: ebo_container(std::forward<Args>(std::get<Is>(args))...) {}

		[[nodiscard]] constexpr T &value() noexcept { return m_value; }
		[[nodiscard]] constexpr const T &value() const noexcept { return m_value; }

		constexpr void swap(ebo_container &other) noexcept(std::is_nothrow_swappable_v<T>)
		{
			using std::swap;
			swap(value(), other.value());
		}

	private:
		T m_value;
	};

	template<typename T>
	constexpr void swap(ebo_container<T> &a, ebo_container<T> &b) noexcept(std::is_nothrow_swappable_v<T>) { a.swap(b); }

	/* Placeholder node link. */
	template<typename N>
	struct empty_link {};

	/* Node link used for ordered tables. */
	template<typename N>
	struct ordered_link
	{
		constexpr ordered_link() noexcept = default;
		constexpr ordered_link(const ordered_link &) noexcept = default;
		constexpr ordered_link &operator=(const ordered_link &) noexcept = default;

		constexpr ordered_link(ordered_link &&other) noexcept { relink(other.next, other.prev); }
		constexpr ordered_link &operator=(ordered_link &&other) noexcept
		{
			relink(other.next, other.prev);
			return *this;
		}

		constexpr void relink(ordered_link *new_next, ordered_link *new_prev) noexcept
		{
			if ((next = new_next) != nullptr)
				new_next->prev = this;
			if ((prev = new_prev) != nullptr)
				new_prev->next = this;
		}

		constexpr void link(ordered_link &new_prev) noexcept
		{
			prev = &new_prev;
			if ((next = std::exchange(new_prev.next, this)) != nullptr)
				next->prev = this;
		}
		constexpr void unlink() noexcept
		{
			prev->next = next;
			next->prev = prev;
			prev = nullptr;
			next = nullptr;
		}

		ordered_link *next = nullptr;
		ordered_link *prev = nullptr;
	};

	/* Helper used to check if the link is ordered. */
	template<template<typename> typename>
	struct is_ordered : std::false_type {};
	template<>
	struct is_ordered<ordered_link> : std::true_type {};

	/* Helper used to check if a hash or compare functor are transparent. */
	template<typename, typename = void>
	struct is_transparent : std::false_type {};
	template<typename T>
	struct is_transparent<T, std::void_t<typename T::is_transparent>> : std::true_type {};

	template<typename N, template<typename> typename L>
	struct table_header : L<N> { constexpr table_header() noexcept { if constexpr (is_ordered<L>::value) L<N>::relink(this, this); }};

	template<typename T, typename N>
	struct ordered_iterator
	{
		using link_t = ordered_link<N>;

		typedef T value_type;
		typedef T &reference;
		typedef T *pointer;

		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;
		typedef std::bidirectional_iterator_tag iterator_category;

		constexpr ordered_iterator() noexcept = default;

		/** Implicit conversion from a const iterator. */
		template<typename U, typename = std::enable_if_t<std::is_same_v<U, std::remove_const_t<T>> && !std::is_same_v<U, T>>>
		constexpr ordered_iterator(const ordered_iterator<U, N> &other) noexcept : link(other.link) {}

		constexpr explicit ordered_iterator(link_t *link) noexcept : link(link) {}
		constexpr explicit ordered_iterator(N *node) noexcept : ordered_iterator(static_cast<link_t *>(node)) {}

		constexpr ordered_iterator operator++(int) noexcept
		{
			auto tmp = *this;
			operator++();
			return tmp;
		}
		constexpr ordered_iterator &operator++() noexcept
		{
			link = link->next;
			return *this;
		}
		constexpr ordered_iterator operator--(int) noexcept
		{
			auto tmp = *this;
			operator--();
			return tmp;
		}
		constexpr ordered_iterator &operator--() noexcept
		{
			link = link->prev;
			return *this;
		}

		[[nodiscard]] constexpr pointer operator->() const noexcept { return static_cast<N *>(link)->get(); }
		[[nodiscard]] constexpr reference operator*() const noexcept { return *operator->(); }

		[[nodiscard]] constexpr bool operator==(const ordered_iterator &other) const noexcept { return link == other.link; }

#if __cplusplus >= 202002L
		[[nodiscard]] constexpr auto operator<=>(const ordered_iterator &other) const noexcept { return link <=> other.link; }
#else
		[[nodiscard]] constexpr bool operator!=(const ordered_iterator &other) const noexcept { return link != other.link; }
		[[nodiscard]] constexpr bool operator<=(const ordered_iterator &other) const noexcept { return link <= other.link; }
		[[nodiscard]] constexpr bool operator>=(const ordered_iterator &other) const noexcept { return link >= other.link; }
		[[nodiscard]] constexpr bool operator<(const ordered_iterator &other) const noexcept { return link < other.link; }
		[[nodiscard]] constexpr bool operator>(const ordered_iterator &other) const noexcept { return link > other.link; }
#endif

		link_t *link = nullptr;
	};

	template<typename>
	struct iterator_concept_base {};

#if __cplusplus >= 202002L
	template<std::contiguous_iterator I>
	struct iterator_concept_base<I> { typedef std::contiguous_iterator_tag iterator_concept; };
#endif

	template<typename, typename>
	class table_iterator;

	template<typename V, typename I>
	[[nodiscard]] constexpr auto to_underlying(table_iterator<V, I>) noexcept;

	template<typename, typename = void>
	struct iter_size { using type = std::size_t; };
	template<typename T>
	struct iter_size<T, std::void_t<typename T::size_type>> { using type = typename T::size_type; };

	template<typename, typename = void>
	struct iter_diff { using type = std::ptrdiff_t; };
	template<typename T>
	struct iter_diff<T, std::void_t<typename T::difference_type>> { using type = typename T::difference_type; };

	template<typename, typename, typename = void>
	struct random_access_iterator_base;
	template<typename I, typename T>
	struct random_access_iterator_base<I, T, std::enable_if_t<std::is_base_of_v<std::random_access_iterator_tag, typename T::iterator_category>>>
	{
		constexpr I &operator+=(typename iter_diff<T>::type n) noexcept { return iter() += n; }
		constexpr I &operator-=(typename iter_diff<T>::type n) noexcept { return iter() -= n; }

		[[nodiscard]] constexpr I operator+(typename iter_diff<T>::type n) const noexcept { return I{iter() + n}; }
		[[nodiscard]] constexpr I operator-(typename iter_diff<T>::type n) const noexcept { return I{iter() - n}; }
		[[nodiscard]] constexpr typename iter_diff<T>::type operator-(const I &other) const noexcept { return iter() - other.m_iter; }

		[[nodiscard]] constexpr auto &operator[](typename iter_diff<T>::type i) const noexcept { return *operator+(i); }

	private:
		[[nodiscard]] constexpr auto &iter() noexcept { return static_cast<I *>(this)->m_iter; }
		[[nodiscard]] constexpr const auto &iter() const noexcept { return static_cast<const I *>(this)->m_iter; }
	};
	template<typename I, typename T>
	struct random_access_iterator_base<I, T, std::enable_if_t<!std::is_base_of_v<std::random_access_iterator_tag, typename T::iterator_category>>> {};

	template<typename V, typename I>
	class table_iterator : public iterator_concept_base<I>, public random_access_iterator_base<table_iterator<V, I>, std::iterator_traits<I>>
	{
		friend struct random_access_iterator_base<table_iterator<V, I>, std::iterator_traits<I>>;

		template<typename U, typename J>
		friend constexpr auto to_underlying(table_iterator<U, J>) noexcept;

		using traits_t = std::iterator_traits<I>;

	public:
		typedef V value_type;
		typedef V &reference;
		typedef V *pointer;

		typedef typename iter_size<traits_t>::type size_type;
		typedef typename iter_diff<traits_t>::type difference_type;
		typedef typename traits_t::iterator_category iterator_category;

	public:
		constexpr table_iterator() noexcept = default;
		template<typename U, typename J, typename = std::enable_if_t<!std::is_same_v<I, J> && std::is_constructible_v<I, const J &>>>
		constexpr table_iterator(const table_iterator<U, J> &other) noexcept : m_iter(other.m_iter) {}

		constexpr explicit table_iterator(I iter) noexcept : m_iter(iter) {}

		constexpr table_iterator operator++(int) noexcept
		{
			auto tmp = *this;
			operator++();
			return tmp;
		}
		constexpr table_iterator &operator++() noexcept
		{
			++m_iter;
			return *this;
		}
		constexpr table_iterator operator--(int) noexcept
		{
			auto tmp = *this;
			operator--();
			return tmp;
		}
		constexpr table_iterator &operator--() noexcept
		{
			--m_iter;
			return *this;
		}

		[[nodiscard]] constexpr pointer operator->() const noexcept
		{
			if constexpr (std::is_pointer_v<I>) /* Pointer to node. */
				return m_iter->get();
			else
				return m_iter.operator->();
		}
		[[nodiscard]] constexpr reference operator*() const noexcept { return *operator->(); }

		[[nodiscard]] constexpr bool operator==(const table_iterator &other) const noexcept { return m_iter == other.m_iter; }

#if __cplusplus >= 202002L
		[[nodiscard]] constexpr auto operator<=>(const table_iterator &other) const noexcept { return m_iter <=> other.m_iter; }
#else
		[[nodiscard]] constexpr bool operator!=(const table_iterator &other) const noexcept { return m_iter != other.m_iter; }
		[[nodiscard]] constexpr bool operator<=(const table_iterator &other) const noexcept { return m_iter <= other.m_iter; }
		[[nodiscard]] constexpr bool operator>=(const table_iterator &other) const noexcept { return m_iter >= other.m_iter; }
		[[nodiscard]] constexpr bool operator<(const table_iterator &other) const noexcept { return m_iter < other.m_iter; }
		[[nodiscard]] constexpr bool operator>(const table_iterator &other) const noexcept { return m_iter > other.m_iter; }
#endif

	private:
		I m_iter = {};
	};

	template<typename V, typename I>
	[[nodiscard]] constexpr auto to_underlying(table_iterator<V, I> iter) noexcept { return iter.m_iter; }

	struct key_identity { [[nodiscard]] constexpr auto &operator()(auto &value) const noexcept { return value; }};
	struct key_first { [[nodiscard]] constexpr auto &operator()(auto &value) const noexcept { return value.first; }};
}