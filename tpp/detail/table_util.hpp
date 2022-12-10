/*
 * Created by switchblade on 11/14/22.
 */

#pragma once

#include "../stl_hash.hpp"

#ifndef TPP_USE_IMPORT

#include <initializer_list>
#include <iterator>
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

	template<typename T>
	struct ebo_container<T, std::enable_if_t<ebo_candidate<T>::value>> : T
	{
		constexpr ebo_container() noexcept(std::is_nothrow_default_constructible_v<T>) = default;
		constexpr ebo_container(const ebo_container &other) noexcept(std::is_nothrow_copy_constructible_v<T>) = default;
		constexpr ebo_container(ebo_container &&other) noexcept(std::is_nothrow_move_constructible_v<T>) = default;
		constexpr ebo_container &operator=(const ebo_container &other) noexcept(std::is_nothrow_copy_assignable_v<T>) = default;
		constexpr ebo_container &operator=(ebo_container &&other) noexcept(std::is_nothrow_move_assignable_v<T>) = default;

		template<typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
		constexpr ebo_container(Args &&...args) noexcept(std::is_nothrow_constructible_v<T, Args...>) : T(std::forward<Args>(args)...) {}

		template<typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
		constexpr ebo_container(std::tuple<Args...> args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
				:    ebo_container(std::make_index_sequence<sizeof...(Args)>(), std::forward<Args>(args)...) {}
		template<std::size_t... Is, typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
		constexpr ebo_container(std::index_sequence<Is...>, std::tuple<Args...> args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
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
		constexpr ebo_container() noexcept(std::is_nothrow_default_constructible_v<T>) = default;
		constexpr ebo_container(const ebo_container &other) noexcept(std::is_nothrow_copy_constructible_v<T>) = default;
		constexpr ebo_container(ebo_container &&other) noexcept(std::is_nothrow_move_constructible_v<T>) = default;
		constexpr ebo_container &operator=(const ebo_container &other) noexcept(std::is_nothrow_copy_assignable_v<T>) = default;
		constexpr ebo_container &operator=(ebo_container &&other) noexcept(std::is_nothrow_move_assignable_v<T>) = default;

		template<typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
		constexpr ebo_container(Args &&...args) noexcept(std::is_nothrow_constructible_v<T, Args...>) : m_value(std::forward<Args>(args)...) {}

		template<typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
		constexpr ebo_container(std::tuple<Args...> args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
				: ebo_container(std::make_index_sequence<sizeof...(Args)>(), std::forward<Args>(args)...) {}
		template<std::size_t... Is, typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
		constexpr ebo_container(std::index_sequence<Is...>, std::tuple<Args...> args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
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

	template<typename I, typename V, typename K, typename Kh, typename Kc, typename A>
	struct table_traits
	{
		typedef I insert_type;
		typedef V value_type;
		typedef K key_type;
		typedef A allocator_type;

		typedef Kh hasher;
		typedef Kc key_equal;

		typedef typename std::allocator_traits<A>::size_type size_type;
		typedef typename std::allocator_traits<A>::difference_type difference_type;
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

	template<typename V, typename Traits>
	class dense_node : public Traits::link_type, public ebo_container<V>
	{
		using link_base = typename Traits::link_type;
		using ebo_base = ebo_container<V>;

	public:
		constexpr dense_node() noexcept = default;
		constexpr dense_node(const dense_node &other) : link_base(other), ebo_base(other), hash(other.hash) {}
		constexpr dense_node(dense_node &&other) noexcept : link_base(std::move(other)), ebo_base(std::move(other)), hash(other.hash) {}

		constexpr dense_node &operator=(const dense_node &other)
		{
			if (this != &other)
			{
				link_base::operator=(other);
				ebo_base::operator=(other);
				hash = other.hash;
			}
			return *this;
		}
		constexpr dense_node &operator=(dense_node &&other) noexcept
		{
			link_base::operator=(std::move(other));
			ebo_base::operator=(std::move(other));
			hash = other.hash;
			return *this;
		}

		template<typename... Args, typename = std::enable_if_t<std::is_constructible_v<V, Args...>>>
		constexpr dense_node(Args &&...args) noexcept(std::is_nothrow_constructible_v<V, Args...>) : ebo_base(std::forward<Args>(args)...) {}

		using ebo_base::value;

		[[nodiscard]] constexpr auto *get() noexcept { return &value(); }
		[[nodiscard]] constexpr const auto *get() const noexcept { return &value(); }

		[[nodiscard]] constexpr auto &key() const noexcept { return Traits::key_get(value()); }
		[[nodiscard]] constexpr auto &mapped() noexcept { return Traits::mapped_get(value()); }
		[[nodiscard]] constexpr auto &mapped() const noexcept { return Traits::mapped_get(value()); }

		constexpr void swap(dense_node &other) noexcept(std::is_nothrow_swappable_v<ebo_base>)
		{
			using std::swap;

			swap(static_cast<link_base &>(*this), static_cast<link_base &>(other));
			swap(static_cast<ebo_base &>(*this), static_cast<ebo_base &>(other));
			swap(hash, other.hash);
		}

		std::size_t hash = 0;
	};
	template<typename V, typename Traits>
	class stable_node : public Traits::link_type
	{
		using link_base = typename Traits::link_type;

	public:
		using is_extractable = std::true_type;

		constexpr stable_node() noexcept = default;
		constexpr stable_node(const stable_node &other) : link_base(other), m_ptr(new V(other.value())), hash(other.hash) {}
		constexpr stable_node(stable_node &&other) noexcept : link_base(std::move(other)), m_ptr(other.extract()), hash(other.hash) {}

		constexpr stable_node &operator=(const stable_node &other)
		{
			if (this != &other)
			{
				link_base::operator=(other);
				if (std::is_copy_assignable_v<V> && m_ptr)
					value() = other.value();
				else
				{
					delete m_ptr;
					m_ptr = new V(other.value());
				}
				hash = other.hash;
			}
			return *this;
		}
		constexpr stable_node &operator=(stable_node &&other) noexcept
		{
			link_base::operator=(std::move(other));
			std::swap(m_ptr, other.m_ptr);
			hash = other.hash;
			return *this;
		}

		template<typename... Args, typename = std::enable_if_t<std::is_constructible_v<V, Args...>>>
		constexpr stable_node(Args &&...args) noexcept(std::is_nothrow_constructible_v<V, Args...>) : m_ptr(new V(std::forward<Args>(args)...)) {}

		~stable_node() { delete m_ptr; }

		[[nodiscard]] constexpr auto *get() noexcept { return m_ptr; }
		[[nodiscard]] constexpr const auto *get() const noexcept { return m_ptr; }

		[[nodiscard]] constexpr auto &value() noexcept { return *get(); }
		[[nodiscard]] constexpr const auto &value() const noexcept { return *get(); }

		[[nodiscard]] constexpr auto &key() const noexcept { return Traits::key_get(value()); }
		[[nodiscard]] constexpr auto &mapped() noexcept { return Traits::mapped_get(value()); }
		[[nodiscard]] constexpr auto &mapped() const noexcept { return Traits::mapped_get(value()); }

		constexpr void swap(stable_node &other) noexcept
		{
			std::swap(static_cast<link_base &>(*this), static_cast<link_base &>(other));
			std::swap(m_ptr, other.m_ptr);
			std::swap(hash, other.hash);
		}

	private:
		V *m_ptr = nullptr;

	public:
		std::size_t hash = 0;
	};

	/* Helper used to check if a node is extractable. */
	template<typename, typename = void>
	struct is_extractable : std::false_type {};
	template<typename T>
	struct is_extractable<T, std::void_t<typename T::is_extractable>> : std::true_type {};

	template<typename N, typename Traits>
	struct ordered_iterator
	{
		using link_t = std::conditional_t<std::is_const_v<N>, std::add_const_t<ordered_link>, ordered_link>;

		typedef N value_type;
		typedef N &reference;
		typedef N *pointer;

		typedef typename Traits::size_type size_type;
		typedef typename Traits::difference_type difference_type;
		typedef std::bidirectional_iterator_tag iterator_category;

		constexpr ordered_iterator() noexcept = default;

		/** Implicit conversion from a const iterator. */
		template<typename U, typename = std::enable_if_t<std::is_same_v<U, std::remove_const_t<N>> && !std::is_same_v<U, N>>>
		constexpr ordered_iterator(const ordered_iterator<U, Traits> &other) noexcept : link(other.link) {}

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

	template<typename, typename, typename>
	class table_iterator;

	template<typename V, typename T, typename I>
	[[nodiscard]] constexpr auto to_underlying(table_iterator<V, T, I>) noexcept;

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

	template<typename V, typename Traits, typename I>
	class table_iterator : public iterator_concept_base<I>, public random_access_iterator_base<table_iterator<V, Traits, I>, std::iterator_traits<I>>
	{
		// @formatter:off
		template<typename, typename, typename>
		friend class table_iterator;
		friend struct random_access_iterator_base<table_iterator<V, Traits, I>, std::iterator_traits<I>>;
		// @formatter:on

		template<typename U, typename T, typename J>
		friend constexpr auto to_underlying(table_iterator<U, T, J>) noexcept;

		using traits_t = std::iterator_traits<I>;

	public:
		typedef V value_type;
		typedef std::conditional_t<std::is_const_v<V>, typename Traits::const_pointer, typename Traits::pointer> pointer;
		typedef std::conditional_t<std::is_const_v<V>, typename Traits::const_reference, typename Traits::reference> reference;

		typedef typename iter_size<traits_t>::type size_type;
		typedef typename iter_diff<traits_t>::type difference_type;
		typedef typename traits_t::iterator_category iterator_category;

	public:
		constexpr table_iterator() noexcept = default;
		template<typename U, typename J, typename = std::enable_if_t<!std::is_same_v<I, J> && std::is_constructible_v<I, std::add_const_t<J> &>>>
		constexpr table_iterator(const table_iterator<U, Traits, J> &other) noexcept : m_iter(other.m_iter) {}

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

		[[nodiscard]] constexpr pointer operator->() const noexcept { return pointer{m_iter->get()}; }
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

	template<typename V, typename T, typename I>
	[[nodiscard]] constexpr auto to_underlying(table_iterator<V, T, I> iter) noexcept { return iter.m_iter; }

	template<typename T, typename U, typename... Us>
	struct is_pack_element : is_pack_element<T, Us...> {};
	template<typename T, typename... Us>
	struct is_pack_element<T, T, Us...> : std::true_type {};
	template<typename T>
	struct is_pack_element<T, T> : std::true_type {};
	template<typename T, typename U>
	struct is_pack_element<T, U> : std::false_type {};

	/** @brief Helper structure used to specify keys of a multiset. */
	template<typename... Ks>
	struct multikey { static_assert(sizeof...(Ks) != 0, "Multikey must have at least one key type"); };

	template<typename>
	struct multiset_alloc;
	template<typename>
	struct multiset_hash;
	template<typename>
	struct multiset_eq;

	template<typename... Ks>
	struct multiset_alloc<multikey<Ks...>> : std::allocator<std::tuple<Ks...>> {};
	template<typename... Ks>
	struct multiset_hash<multikey<Ks...>>
	{
		template<typename T, typename = std::enable_if_t<is_pack_element<T, Ks...>::value>>
		[[nodiscard]] constexpr auto operator()(const T &key) const { return default_hash<T>{}(key); }
	};
	template<typename... Ks>
	struct multiset_eq<multikey<Ks...>>
	{
		template<typename T, typename = std::enable_if_t<is_pack_element<T, Ks...>::value>>
		[[nodiscard]] constexpr auto operator()(const T &a, const T &b) const { return std::equal_to<>{}(a, b); }
	};

	template<std::size_t, typename T>
	[[nodiscard]] constexpr static decltype(auto) forward_i(T &&value) noexcept { return std::forward<T>(value); }
}