/*
 * Created by switchblade on 11/14/22.
 */

#pragma once

#include "../stl_hash.hpp"

#ifndef TPP_USE_IMPORT

#include <initializer_list>
#include <algorithm>
#include <iterator>
#include <cstdint>
#include <limits>
#include <array>

#else

#ifdef _MSC_VER

import std.memory;

#endif

#endif

#if defined(TPP_DEBUG) || !defined(NDEBUG)

#ifndef TPP_USE_IMPORT

#include <cstdio>

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
#if defined(TPP_DEBUG) || !defined(NDEBUG)
	static inline void assert_msg(bool cnd, const char *cstr, const char *file, std::size_t line, const char *func, const char *msg) noexcept
	{
		if (!cnd)
		{
			printf("Assertion (%s) failed at '%s:%zu' in '%s'", cstr, file, line, func);
			if (msg) printf("%s", msg);
			TPP_DEBUG_TRAP();
			std::terminate();
		}
	}
#endif

	template<std::size_t, typename T>
	[[nodiscard]] constexpr static decltype(auto) forward_n(T &&value) noexcept { return std::forward<T>(value); }

	template<std::size_t... Is, typename T>
	[[nodiscard]] constexpr static std::array<T, sizeof...(Is)> make_array(std::index_sequence<Is...>, const T &value) { return {forward_n<Is>(value)...}; }
	template<std::size_t Size, typename T>
	[[nodiscard]] constexpr static std::array<T, Size> make_array(const T &value) noexcept { return make_array(std::make_index_sequence<Size>{}, value); }

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

		TPP_CXX20_CONSTEXPR void swap(ebo_container &other) noexcept(std::is_nothrow_swappable_v<T>)
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

		TPP_CXX20_CONSTEXPR void swap(ebo_container &other) noexcept(std::is_nothrow_swappable_v<T>)
		{
			using std::swap;
			swap(value(), other.value());
		}

	private:
		T m_value;
	};

	template<typename T>
	TPP_CXX20_CONSTEXPR void swap(ebo_container<T> &a, ebo_container<T> &b) noexcept(std::is_nothrow_swappable_v<T>) { a.swap(b); }

	template<typename I, typename V, typename K, typename Kh, typename Kc, typename A>
	struct table_traits
	{
		using insert_type = I;
		using value_type = V;
		using key_type = K;
		using allocator_type = A;

		using hasher = Kh;
		using key_equal = Kc;

		using size_type = typename std::allocator_traits<A>::size_type;
		using difference_type = typename std::allocator_traits<A>::difference_type;
	};

	/* Placeholder node link. */
	struct empty_link
	{
		constexpr void link(std::ptrdiff_t) noexcept {}
		constexpr void link(empty_link *) noexcept {}

		constexpr void relink_from(empty_link &) noexcept {}
		constexpr void unlink() noexcept {}

		[[nodiscard]] constexpr empty_link *off(std::ptrdiff_t) noexcept { return this; }
		[[nodiscard]] constexpr const empty_link *off(std::ptrdiff_t) const noexcept { return this; }

		constexpr void swap(empty_link &) noexcept {}
	};
	/* Node link used for ordered tables. */
	struct ordered_link
	{
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

		TPP_CXX20_CONSTEXPR ordered_link() noexcept = default;
		TPP_CXX20_CONSTEXPR ordered_link(ordered_link &&other) noexcept { relink_from(other); }
		TPP_CXX20_CONSTEXPR ordered_link &operator=(ordered_link &&other) noexcept
		{
			relink_from(other);
			return *this;
		}

		TPP_CXX20_CONSTEXPR void link(std::ptrdiff_t prev_off) noexcept { link(off(prev_off)); }
		TPP_CXX20_CONSTEXPR void link(ordered_link *prev_ptr) noexcept
		{
			const auto next_ptr = prev_ptr->off(prev_ptr->next);
			next_ptr->prev = -(next = byte_diff(next_ptr, this));
			prev_ptr->next = -(prev = byte_diff(prev_ptr, this));
		}

		TPP_CXX20_CONSTEXPR void relink_from(ordered_link &other) noexcept
		{
			next = prev = 0;
			if (other.next != 0)
			{
				auto *next_ptr = other.off(std::exchange(other.next, 0));
				next_ptr->prev = -(next = byte_diff(next_ptr, this));
			}
			if (other.prev != 0)
			{
				auto *prev_ptr = other.off(std::exchange(other.prev, 0));
				prev_ptr->next = -(prev = byte_diff(prev_ptr, this));
			}
		}
		TPP_CXX20_CONSTEXPR void unlink() noexcept
		{
			auto *next_ptr = off(std::exchange(next, 0));
			auto *prev_ptr = off(std::exchange(prev, 0));
			next_ptr->prev = byte_diff(prev_ptr, next_ptr);
			prev_ptr->next = byte_diff(next_ptr, prev_ptr);
		}

		[[nodiscard]] constexpr ordered_link *off(std::ptrdiff_t n) noexcept { return static_cast<ordered_link *>(byte_off(this, n)); }
		[[nodiscard]] constexpr const ordered_link *off(std::ptrdiff_t n) const noexcept { return static_cast<const ordered_link *>(byte_off(this, n)); }

		TPP_CXX20_CONSTEXPR void swap(ordered_link &other) noexcept
		{
			std::swap(next, other.next);
			std::swap(prev, other.prev);
		}

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

	template<typename Alloc>
	[[nodiscard]] constexpr static bool allocator_eq(const Alloc &a, const Alloc &b)
	{
		if constexpr (!std::allocator_traits<Alloc>::is_always_equal::value)
			return a == b;
		else
			return true;
	}

	template<typename A, typename T>
	TPP_CXX20_CONSTEXPR void relocate(A &alloc_src, T *src, A &alloc_dst, T *dst)
	{
		std::allocator_traits<A>::construct(alloc_dst, dst, std::move(*src));
		std::allocator_traits<A>::destroy(alloc_src, src);
	}
	template<typename A, typename T, typename F>
	TPP_CXX20_CONSTEXPR void relocate(A &alloc_src, T *src_first, T *src_last, A &alloc_dst, T *dst_first, F rel = relocate<A, T>)
	{
		while (src_first != src_last) rel(alloc_src, src_first++, alloc_dst, dst_first++);
	}

	template<typename A, typename T, typename SizeT = typename std::allocator_traits<A>::size_type>
	TPP_CXX20_CONSTEXPR bool realloc_buffer(A &alloc, T *&buff, SizeT &buff_size, SizeT new_size)
	{
		if (buff_size < new_size)
		{
			if (buff) std::allocator_traits<A>::deallocate(alloc, buff, buff_size);
			buff = std::allocator_traits<A>::allocate(alloc, buff_size = new_size);
			return true;
		}
		return false;
	}
	template<typename A, typename T, typename F, typename SizeT = typename std::allocator_traits<A>::size_type>
	TPP_CXX20_CONSTEXPR bool resize_buffer(A &alloc, T *&buff, SizeT &buff_size, SizeT new_size, F rel = relocate<A, T>)
	{
		if (buff_size < new_size)
		{
			new_size = std::max(buff_size * 2, new_size);
			auto *new_buff = std::allocator_traits<A>::allocate(alloc, new_size);
			relocate(alloc, buff, buff + buff_size, alloc, new_buff, rel);

			if (buff) std::allocator_traits<A>::deallocate(alloc, buff, buff_size);
			buff_size = new_size;
			buff = new_buff;
			return true;
		}
		return false;
	}

	template<typename V, typename A, typename Traits>
	class dense_node : public Traits::link_type
	{
		using link_base = typename Traits::link_type;
		using hash_type = std::array<std::size_t, Traits::key_size>;

		using storage_t = std::array<std::uint8_t, sizeof(hash_type) + (std::is_empty_v<V> ? 0 : sizeof(V))>;

		template<typename T>
		[[nodiscard]] constexpr static auto *void_cast(void *ptr) noexcept { return static_cast<T *>(ptr); }
		template<typename T>
		[[nodiscard]] constexpr static auto *void_cast(const void *ptr) noexcept { return static_cast<const T *>(ptr); }

	public:
		using allocator_type = A;

		dense_node(const dense_node &) = delete;
		dense_node &operator=(const dense_node &) = delete;

		constexpr dense_node() noexcept = default;

		template<typename... Args>
		TPP_CXX20_CONSTEXPR void construct(A &alloc, Args &&...args) { std::allocator_traits<A>::construct(alloc, &value(), std::forward<Args>(args)...); }
		TPP_CXX20_CONSTEXPR void construct(A &alloc, dense_node &&other)
		{
			link_base::operator=(std::move(other));
			construct(alloc, std::move(other.value()));
			hash() = other.hash();
		}
		TPP_CXX20_CONSTEXPR void move_from(dense_node &other)
		{
			link_base::operator=(std::move(other));
			value() = std::move(other.value());
			hash() = other.hash();
		}
		TPP_CXX20_CONSTEXPR void destroy(A &alloc) { std::allocator_traits<A>::destroy(alloc, &value()); }

		[[nodiscard]] constexpr auto &hash() noexcept { return data_off<hash_type>(0); }
		[[nodiscard]] constexpr auto &hash() const noexcept { return data_off<hash_type>(0); }

		template<std::size_t I>
		[[nodiscard]] constexpr auto &hash() noexcept { return hash()[I]; }
		template<std::size_t I>
		[[nodiscard]] constexpr auto &hash() const noexcept { return hash()[I]; }

		[[nodiscard]] constexpr auto &value() noexcept { return data_off<V>(sizeof(hash_type)); }
		[[nodiscard]] constexpr auto &value() const noexcept { return data_off<V>(sizeof(hash_type)); }

		template<std::size_t I>
		[[nodiscard]] constexpr auto &key() const noexcept { return Traits::template key_get<I>(value()); }
		[[nodiscard]] constexpr auto &mapped() noexcept { return Traits::mapped_get(value()); }
		[[nodiscard]] constexpr auto &mapped() const noexcept { return Traits::mapped_get(value()); }

		TPP_CXX20_CONSTEXPR friend void swap(dense_node &a, dense_node &b) noexcept(std::is_nothrow_swappable_v<V>)
		{
			using std::swap;
			a.link_base::swap(b);
			swap(a.value(), b.value());
		}

	private:
		template<typename T>
		[[nodiscard]] constexpr T &data_off(std::size_t off) noexcept { return *void_cast<T>(m_storage.data() + off); }
		template<typename T>
		[[nodiscard]] constexpr const T &data_off(std::size_t off) const noexcept { return *void_cast<T>(m_storage.data() + off); }

		storage_t m_storage = {};
	};
	template<typename V, typename A, typename Traits>
	class stable_node : public Traits::link_type
	{
		using link_base = typename Traits::link_type;
		using hash_type = std::array<std::size_t, Traits::key_size>;

	public:
		using allocator_type = A;
		using is_extractable = std::true_type;

		stable_node(const stable_node &) = delete;
		stable_node &operator=(const stable_node &) = delete;

		constexpr stable_node() noexcept = default;

		template<typename... Args>
		TPP_CXX20_CONSTEXPR void construct(A &alloc, Args &&...args)
		{
			m_ptr = std::allocator_traits<A>::allocate(alloc, 1);
			std::allocator_traits<A>::construct(alloc, m_ptr, std::forward<Args>(args)...);
		}
		TPP_CXX20_CONSTEXPR void construct(A &, stable_node &&other)
		{
			link_base::operator=(std::move(other));
			std::swap(m_ptr, other.m_ptr);
			m_hash = other.m_hash;
		}
		TPP_CXX20_CONSTEXPR void move_from(stable_node &other)
		{
			link_base::operator=(std::move(other));
			std::swap(m_ptr, other.m_ptr);
			m_hash = other.m_hash;
		}
		TPP_CXX20_CONSTEXPR void destroy(A &alloc)
		{
			if (m_ptr)
			{
				std::allocator_traits<A>::destroy(alloc, m_ptr);
				std::allocator_traits<A>::deallocate(alloc, m_ptr, 1);
				m_ptr = nullptr;
			}
		}

		template<std::size_t I>
		[[nodiscard]] constexpr auto &hash() noexcept { return m_hash[I]; }
		template<std::size_t I>
		[[nodiscard]] constexpr auto &hash() const noexcept { return m_hash[I]; }

		[[nodiscard]] constexpr auto &value() noexcept { return *m_ptr; }
		[[nodiscard]] constexpr const auto &value() const noexcept { return *m_ptr; }

		template<std::size_t I>
		[[nodiscard]] constexpr auto &key() const noexcept { return Traits::template key_get<I>(value()); }
		[[nodiscard]] constexpr auto &mapped() noexcept { return Traits::mapped_get(value()); }
		[[nodiscard]] constexpr auto &mapped() const noexcept { return Traits::mapped_get(value()); }

		constexpr friend void swap(stable_node &a, stable_node &b) noexcept
		{
			a.link_base::swap(b);
			std::swap(a.m_ptr, b.m_ptr);
			std::swap(a.m_hash, b.m_hash);
		}

	private:
		V *m_ptr = nullptr;
		hash_type m_hash = 0;
	};

	/* Need a special relocate functor for dense_node & stable_node since they require an injected value allocator. */
	struct relocate_node
	{
		template<typename A, typename N, typename Alloc = typename N::allocator_type>
		TPP_CXX20_CONSTEXPR void operator()(A &alloc_src, N *src, A &alloc_dst, N *dst) const
		{
			auto value_alloc_src = Alloc{alloc_src};
			auto value_alloc_dst = Alloc{alloc_dst};
			dst->construct(value_alloc_dst, std::move(*src));
			src->destroy(value_alloc_src);
		}
	};

	/* Helper used to check if a node is extractable. */
	template<typename, typename = void>
	struct is_extractable : std::false_type {};
	template<typename T>
	struct is_extractable<T, std::void_t<typename T::is_extractable>> : std::true_type {};

	/* Helper used to select node table type. */
	template<typename T, typename = void>
	struct is_stable : std::false_type {};
	template<typename T>
	struct is_stable<T, std::void_t<typename T::is_stable>> : std::true_type {};
	template<typename V, typename Traits, typename Alloc>
	using table_node = std::conditional_t<is_stable<Traits>::value, stable_node<V, Traits, Alloc>, dense_node<V, Traits, Alloc>>;

	template<typename N, typename Traits>
	struct ordered_iterator
	{
		using link_t = std::conditional_t<std::is_const_v<N>, std::add_const_t<ordered_link>, ordered_link>;

		using value_type = N;
		using reference = N &;
		using pointer = N *;

		using size_type = typename Traits::size_type;
		using difference_type = typename Traits::difference_type;
		using iterator_category = std::bidirectional_iterator_tag;

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
	struct iterator_concept_base<I> { using iterator_concept = std::contiguous_iterator_tag; };
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
		using value_type = V;
		using pointer = std::conditional_t<std::is_const_v<V>, typename Traits::const_pointer, typename Traits::pointer>;
		using reference = std::conditional_t<std::is_const_v<V>, typename Traits::const_reference, typename Traits::reference>;

		using size_type = typename iter_size<traits_t>::type;
		using difference_type = typename iter_diff<traits_t>::type;
		using iterator_category = typename traits_t::iterator_category;

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

		[[nodiscard]] constexpr pointer operator->() const noexcept { return pointer{&m_iter->value()}; }
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

	template<std::size_t, typename...>
	struct remove_index;
	template<std::size_t I, std::size_t... Is>
	struct remove_index<I, std::index_sequence<Is...>, std::index_sequence<>>
	{
		using type = std::index_sequence<Is...>;
	};
	template<std::size_t I, std::size_t... Is, std::size_t... Us>
	struct remove_index<I, std::index_sequence<Is...>, std::index_sequence<I, Us...>>
	{
		using type = typename remove_index<I, std::index_sequence<Is...>, std::index_sequence<Us...>>::type;
	};
	template<std::size_t I, std::size_t... Is, std::size_t U, std::size_t... Us>
	struct remove_index<I, std::index_sequence<Is...>, std::index_sequence<U, Us...>>
	{
		using type = typename remove_index<I, std::index_sequence<Is..., U>, std::index_sequence<Us...>>::type;
	};
	template<std::size_t I, std::size_t... Is>
	struct remove_index<I, std::index_sequence<Is...>>
	{
		using type = typename remove_index<I, std::index_sequence<>, std::index_sequence<Is...>>::type;
	};

	template<std::size_t I, typename Is>
	using remove_index_t = typename remove_index<I, Is>::type;
}

#if defined(TPP_DEBUG) || !defined(NDEBUG)
#ifdef TPP_HAS_CONSTEVAL
#define TPP_ASSERT(cnd, msg) if (!TPP_IS_CONSTEVAL) tpp::detail::assert_msg(cnd, (#cnd), (__FILE__), (__LINE__), (TPP_PRETTY_FUNC), (msg));
#else
#define TPP_ASSERT(cnd, msg) tpp::detail::assert_msg(cnd, (#cnd), (__FILE__), (__LINE__), (TPP_PRETTY_FUNC), (msg));
#endif
#else
#define TPP_ASSERT(cnd, msg) TPP_ASSUME(cnd)
#endif