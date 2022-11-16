/*
 * Created by switchblade on 11/14/22.
 */

#pragma once

#include "common.hpp"

#ifndef TPP_USE_IMPORT

#include <iterator>
#include <utility>
#include <cstdint>
#include <limits>

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

	template<typename V, typename K, typename A>
	struct table_traits
	{
		typedef V value_type;
		typedef K key_type;
		typedef A allocator_type;

		typedef typename std::allocator_traits<A>::size_type size_type;
		typedef typename std::allocator_traits<A>::difference_type difference_type;

		constexpr static float initial_load_factor = .875f;

		constexpr static size_type npos = std::numeric_limits<size_type>::max();
		constexpr static size_type initial_capacity = 8;
	};

	/* Placeholder node link. */
	struct empty_link {};
	/* Node link used for ordered tables. */
	struct ordered_link
	{
		template<typename T, typename U>
		[[nodiscard]] constexpr static auto const_exchange(T &a, U &&b) noexcept
		{
			const auto old = a;
			a = std::forward<U>(b);
			return old;
		}

		[[nodiscard]] constexpr static std::ptrdiff_t byte_diff(const void *a, const void *b) noexcept
		{
			const auto a_bytes = static_cast<const std::uint8_t *>(a);
			const auto b_bytes = static_cast<const std::uint8_t *>(b);
			return a_bytes - b_bytes;
		}
		[[nodiscard]] constexpr static const void *byte_off(const void *p, std::ptrdiff_t n) noexcept
		{
			return static_cast<const std::uint8_t *>(p) + n;
		}
		[[nodiscard]] constexpr static void *byte_off(void *p, std::ptrdiff_t n) noexcept
		{
			return static_cast<std::uint8_t *>(p) + n;
		}

		constexpr ordered_link() noexcept = default;
		constexpr ordered_link(ordered_link &&other) noexcept { relink_from(other); }
		constexpr ordered_link &operator=(ordered_link &&other) noexcept
		{
			relink_from(other);
			return *this;
		}

		constexpr void link(std::ptrdiff_t prev_off) noexcept { link(off(prev_off)); }
		constexpr void link(ordered_link *prev_ptr) noexcept
		{
			const auto next_ptr = prev_ptr->off(prev_ptr->next);
			next_ptr->prev = -(next = byte_diff(next_ptr, this));
			prev_ptr->next = -(prev = byte_diff(prev_ptr, this));
		}

		constexpr void relink_from(ordered_link &other) noexcept
		{
			next = prev = 0;
			if (other.next != 0)
			{
				auto *next_ptr = other.off(const_exchange(other.next, 0));
				next_ptr->prev = -(next = byte_diff(next_ptr, this));
			}
			if (other.prev != 0)
			{
				auto *prev_ptr = other.off(const_exchange(other.prev, 0));
				prev_ptr->next = -(prev = byte_diff(prev_ptr, this));
			}
		}
		constexpr void unlink() noexcept
		{
			auto *next_ptr = off(const_exchange(next, 0));
			auto *prev_ptr = off(const_exchange(prev, 0));
			next_ptr->prev = byte_diff(prev_ptr, next_ptr);
			prev_ptr->next = byte_diff(next_ptr, prev_ptr);
		}

		[[nodiscard]] constexpr ordered_link *off(std::ptrdiff_t n) noexcept { return static_cast<ordered_link *>(byte_off(this, n)); }
		[[nodiscard]] constexpr const ordered_link *off(std::ptrdiff_t n) const noexcept { return static_cast<const ordered_link *>(byte_off(this, n)); }

		/* Offsets from `this` to next & previous links in bytes. */
		std::ptrdiff_t next = 0; /* next_ptr - this */
		std::ptrdiff_t prev = 0; /* prev_ptr - this */
	};

	/* Helper used to check if the link is ordered. */
	template<typename>
	struct is_ordered : std::false_type {};
	template<>
	struct is_ordered<ordered_link> : std::true_type {};

	/* Helper used to check if a hash or compare functor are transparent. */
	template<typename, typename = void>
	struct is_transparent : std::false_type {};
	template<typename T>
	struct is_transparent<T, std::void_t<typename T::is_transparent>> : std::true_type {};

	template<typename N>
	struct ordered_iterator
	{
		using link_t = ordered_link;

		typedef N value_type;
		typedef N &reference;
		typedef N *pointer;

		typedef typename N::size_type size_type;
		typedef typename N::difference_type difference_type;
		typedef std::bidirectional_iterator_tag iterator_category;

		constexpr ordered_iterator() noexcept = default;

		/** Implicit conversion from a const iterator. */
		template<typename U, typename = std::enable_if_t<std::is_same_v<U, std::remove_const_t<N>> && !std::is_same_v<U, N>>>
		constexpr ordered_iterator(const ordered_iterator<U> &other) noexcept : link(other.link) {}

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
			link = link->off(link->next);
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
			link = link->off(link->prev);
			return *this;
		}

		[[nodiscard]] constexpr pointer operator->() const noexcept { return static_cast<pointer>(link); }
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
		// @formatter:off
		template<typename U, typename J>
		friend class table_iterator;
		friend struct random_access_iterator_base<table_iterator<V, I>, std::iterator_traits<I>>;
		// @formatter:on

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
		template<typename U, typename J, typename = std::enable_if_t<!std::is_same_v<I, J> && std::is_constructible_v<I, std::add_const_t<J> &>>>
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

		[[nodiscard]] constexpr pointer operator->() const noexcept { return m_iter->get(); }
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

	struct key_identity
	{
		template<typename T>
		[[nodiscard]] constexpr T &operator()(T &value) const noexcept { return value; }
	};
	struct key_first
	{
		template<typename T>
		[[nodiscard]] constexpr auto &operator()(T &value) const noexcept { return value.first; }
	};
}