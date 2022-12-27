/*
 * Created by switchblade on 11/14/22.
 */

#pragma once

#include "utility.hpp"
#include "../stl_hash.hpp"

#ifndef TPP_USE_IMPORT

#include <initializer_list>

#else

#if defined(_MSC_VER) && (__cplusplus <= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG <= 202002L))

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

		void swap(ebo_container &other) noexcept(std::is_nothrow_swappable_v<T>)
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

		void swap(ebo_container &other) noexcept(std::is_nothrow_swappable_v<T>)
		{
			using std::swap;
			swap(value(), other.value());
		}

	private:
		T m_value;
	};

	template<typename T>
	void swap(ebo_container<T> &a, ebo_container<T> &b) noexcept(std::is_nothrow_swappable_v<T>) { a.swap(b); }

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
	struct empty_link { constexpr void swap(empty_link &) noexcept {}};
	/* Node link used for ordered tables. */
	struct ordered_link
	{
		[[nodiscard]] static constexpr std::ptrdiff_t byte_diff(const void *a, const void *b) noexcept
		{
			const auto a_bytes = static_cast<const std::uint8_t *>(a);
			const auto b_bytes = static_cast<const std::uint8_t *>(b);
			return a_bytes - b_bytes;
		}
		[[nodiscard]] static constexpr const void *byte_off(const void *p, std::ptrdiff_t n) noexcept
		{
			return static_cast<const std::uint8_t *>(p) + n;
		}
		[[nodiscard]] static constexpr void *byte_off(void *p, std::ptrdiff_t n) noexcept
		{
			return static_cast<std::uint8_t *>(p) + n;
		}

		ordered_link() noexcept = default;
		ordered_link(const ordered_link &) noexcept = default;
		ordered_link &operator=(const ordered_link &) noexcept = default;
		ordered_link(ordered_link &&other) noexcept { relink_from(other); }
		ordered_link &operator=(ordered_link &&other) noexcept
		{
			relink_from(other);
			return *this;
		}

		void link(std::ptrdiff_t prev_off) noexcept { link(off(prev_off)); }
		void link(ordered_link *prev_ptr) noexcept { link(prev_ptr->off(prev_ptr->next), prev_ptr); }
		void link(ordered_link *next_ptr, ordered_link *prev_ptr) noexcept
		{
			next_ptr->prev = -(next = byte_diff(next_ptr, this));
			prev_ptr->next = -(prev = byte_diff(prev_ptr, this));
		}

		void relink_from(ordered_link &other) noexcept
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
		void unlink() noexcept
		{
			auto *next_ptr = off(next);
			auto *prev_ptr = off(prev);
			next_ptr->prev = byte_diff(prev_ptr, next_ptr);
			prev_ptr->next = byte_diff(next_ptr, prev_ptr);
		}

		[[nodiscard]] constexpr ordered_link *off(std::ptrdiff_t n) noexcept { return static_cast<ordered_link *>(byte_off(this, n)); }
		[[nodiscard]] constexpr const ordered_link *off(std::ptrdiff_t n) const noexcept { return static_cast<const ordered_link *>(byte_off(this, n)); }

		void swap(ordered_link &other) noexcept
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
	struct is_transparent<T, std::void_t<typename T::is_transparent>> : T::is_transparent {};

	template<typename T>
	struct has_key_size
	{
		template<typename C>
		[[maybe_unused]] static std::true_type test_key_size(decltype(&C::key_size));
		template<typename C>
		[[maybe_unused]] static std::false_type test_key_size(...);

		constexpr static bool value = std::is_same_v<decltype(test_key_size<T>(nullptr)), std::true_type>;
	};
	template<typename T, bool = has_key_size<T>::value>
	struct node_key { using type = std::array<std::size_t, T::key_size>; };
	template<typename T>
	struct node_key<T, false> { using type = std::size_t; };

	template<typename V, typename A, typename Traits>
	class packed_node : public Traits::link_type
	{
		using hash_type = typename node_key<Traits>::type;
		using link_base = typename Traits::link_type;

		struct storage_t : ebo_container<V>
		{
			hash_type hash;
		};

	public:
		using allocator_type = A;

		constexpr packed_node() noexcept {}
		~packed_node() {}

		template<typename... Args, typename = std::enable_if_t<std::is_constructible_v<V, Args...>>>
		void construct(A &alloc, Args &&...args) { std::allocator_traits<A>::construct(alloc, &value(), std::forward<Args>(args)...); }
		void construct(A &alloc, const packed_node &other)
		{
			link_base::operator=(other);
			construct(alloc, other.value());
			hash() = other.hash();
		}
		void construct(A &alloc, packed_node &&other)
		{
			link_base::operator=(std::move(other));
			construct(alloc, std::move(other.value()));
			hash() = other.hash();
		}
		void move_from(packed_node &other)
		{
			link_base::operator=(std::move(other));
			value() = std::move(other.value());
			hash() = other.hash();
		}
		template<typename... Args>
		void replace(Args &&...args)
		{
			auto &target = mapped();
			using mapped_t = std::remove_reference_t<decltype(target)>;

			if constexpr (!std::is_trivially_destructible_v<mapped_t>)
				target.~mapped_t();

			static_assert(std::is_constructible_v<mapped_t, Args...>);
			new(&target) mapped_t(std::forward<Args>(args)...);
		}
		void destroy(A &alloc) { std::allocator_traits<A>::destroy(alloc, &value()); }

		[[nodiscard]] constexpr auto &hash() noexcept { return m_storage.hash; }
		[[nodiscard]] constexpr auto &hash() const noexcept { return m_storage.hash; }

		template<std::size_t I>
		[[nodiscard]] constexpr auto &hash() noexcept { return hash()[I]; }
		template<std::size_t I>
		[[nodiscard]] constexpr auto &hash() const noexcept { return hash()[I]; }

		[[nodiscard]] constexpr auto &value() noexcept { return m_storage.value(); }
		[[nodiscard]] constexpr auto &value() const noexcept { return m_storage.value(); }

		template<std::size_t I>
		[[nodiscard]] constexpr decltype(auto) key() const noexcept { return Traits::template get_key<I>(value()); }
		[[nodiscard]] constexpr decltype(auto) key() const noexcept { return Traits::get_key(value()); }

		[[nodiscard]] constexpr decltype(auto) mapped() noexcept { return Traits::get_mapped(value()); }
		[[nodiscard]] constexpr decltype(auto) mapped() const noexcept { return Traits::get_mapped(value()); }

		friend void swap(packed_node &a, packed_node &b) noexcept(std::is_nothrow_swappable_v<V>)
		{
			using std::swap;
			a.link_base::swap(b);
			swap(a.value(), b.value());
			swap(a.hash(), b.hash());
		}

	private:
		union
		{
			std::array<std::uint8_t, sizeof(storage_t)> m_bytes = {};
			storage_t m_storage;
		};
	};
	template<typename V, typename A, typename Traits>
	class stable_node : public Traits::link_type
	{
		using hash_type = typename node_key<Traits>::type;
		using link_base = typename Traits::link_type;

	public:
		using allocator_type = A;
		using is_extractable = std::true_type;

		class extracted_type : ebo_container<A>
		{
			friend class stable_node;

			using alloc_base = ebo_container<A>;

		public:
			constexpr extracted_type() noexcept = default;

			extracted_type(extracted_type &&other) noexcept : alloc_base(std::move(other)), m_ptr(std::exchange(other.m_ptr, nullptr)), m_hash(other.m_hash)
			{
				assert_alloc(other);
			}
			extracted_type(A &alloc, extracted_type &&other) noexcept : alloc_base(alloc), m_ptr(std::exchange(other.m_ptr, nullptr)), m_hash(other.m_hash)
			{
				assert_alloc(other);
			}

			extracted_type &operator=(extracted_type &&other) noexcept
			{
				if (this != &other)
				{
					destroy_impl();

					if constexpr (std::allocator_traits<A>::propagate_on_container_move_assignment::value)
						alloc_base::operator=(std::move(other));

					m_ptr = std::exchange(other.m_ptr, nullptr);
					m_hash = other.m_hash;
					assert_alloc(other);
				}
				return *this;
			}

			~extracted_type() { destroy_impl(); }

			[[nodiscard]] constexpr bool empty() const noexcept { return m_ptr == nullptr; }
			[[nodiscard]] constexpr operator bool() const noexcept { return !empty(); }

			[[nodiscard]] allocator_type get_allocator() const { return alloc(); }

			[[nodiscard]] constexpr auto &value() const noexcept { return *m_ptr; }
			[[nodiscard]] constexpr auto &key() const noexcept { return Traits::get_key(value()); }
			[[nodiscard]] constexpr auto &mapped() const noexcept { return Traits::get_mapped(value()); }

			void swap(extracted_type &other) noexcept(std::is_nothrow_swappable_v<A>)
			{
				using std::swap;
				if constexpr (std::allocator_traits<A>::propagate_on_container_swap::value)
					swap(alloc(), other.alloc());

				assert_alloc(other);

				swap(m_ptr, other.m_ptr);
				swap(m_hash, other.m_hash);
			}
			friend void swap(extracted_type &a, extracted_type &b) noexcept(std::is_nothrow_swappable_v<extracted_type>) { a.swap(b); }

		private:
			[[nodiscard]] constexpr auto &alloc() noexcept { return alloc_base::value(); }
			[[nodiscard]] constexpr auto &alloc() const noexcept { return alloc_base::value(); }

			void assert_alloc(const extracted_type &other) const noexcept
			{
				TPP_ASSERT(allocator_eq(alloc(), other.alloc()), "Node allocators must compare equal");
			}
			void destroy_impl()
			{
				if (!empty())
				{
					std::allocator_traits<A>::destroy(alloc(), m_ptr);
					std::allocator_traits<A>::deallocate(alloc(), m_ptr, 1);
				}
			}

			V *m_ptr = nullptr;
			hash_type m_hash = 0;
		};

		/* NOTE: Template signature is defined by the standard. */
		template<typename Iter, typename NodeType>
		struct insert_return
		{
			Iter position = {};
			bool inserted = false;
			NodeType node = {};
		};

	public:
		constexpr stable_node() noexcept = default;

		void construct(A &alloc, const stable_node &other)
		{
			link_base::operator=(other);
			construct(alloc, other.value());
			m_hash = other.m_hash;
		}
		void construct(A &, stable_node &&other)
		{
			link_base::operator=(std::move(other));
			std::swap(m_ptr, other.m_ptr);
			m_hash = other.m_hash;
		}
		void move_from(stable_node &other)
		{
			link_base::operator=(std::move(other));
			std::swap(m_ptr, other.m_ptr);
			m_hash = other.m_hash;
		}

		void construct(A &, extracted_type &&other)
		{
			std::swap(m_ptr, other.m_ptr);
			std::swap(m_hash, other.m_hash);
		}

		template<typename... Args, typename = std::enable_if_t<std::is_constructible_v<V, Args...>>>
		void construct(A &alloc, Args &&...args)
		{
			m_ptr = std::allocator_traits<A>::allocate(alloc, 1);
			std::allocator_traits<A>::construct(alloc, m_ptr, std::forward<Args>(args)...);
		}
		template<typename... Args>
		void replace(Args &&...args)
		{
			auto &target = mapped();
			using mapped_t = std::remove_reference_t<decltype(target)>;

			if constexpr (!std::is_trivially_destructible_v<mapped_t>)
				target.~mapped_t();

			static_assert(std::is_constructible_v<mapped_t, Args...>);
			new(&target) mapped_t(std::forward<Args>(args)...);
		}
		void destroy(A &alloc)
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
		[[nodiscard]] constexpr decltype(auto) key() const noexcept { return Traits::template get_key<I>(value()); }
		[[nodiscard]] constexpr decltype(auto) key() const noexcept { return Traits::get_key(value()); }

		[[nodiscard]] constexpr decltype(auto) mapped() noexcept { return Traits::get_mapped(value()); }
		[[nodiscard]] constexpr decltype(auto) mapped() const noexcept { return Traits::get_mapped(value()); }

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

	/* Need a special relocate functor for packed_node & stable_node since they require an injected value allocator. */
	struct relocate_node
	{
		template<typename A, typename N, typename Alloc = typename N::allocator_type>
		void operator()(A &alloc_src, N *src, A &alloc_dst, N *dst) const
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
	struct is_stable<T, std::void_t<typename T::is_stable>> : T::is_stable {};
	template<typename V, typename Traits, typename Alloc>
	using table_node = std::conditional_t<is_stable<Traits>::value, stable_node<V, Traits, Alloc>, packed_node<V, Traits, Alloc>>;

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
		template<typename U, typename = std::enable_if_t<!std::is_same_v<U, N>>>
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

#if (__cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L))
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

#if (__cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L))
	template<std::contiguous_iterator I>
	struct iterator_concept_base<I> { using iterator_concept = std::contiguous_iterator_tag; };
#endif

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

	template<typename, typename, typename>
	class table_iterator;

	template<typename V, typename T, typename I>
	[[nodiscard]] constexpr auto to_underlying(table_iterator<V, T, I>) noexcept;

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

#if (__cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L))
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

	template<typename K, typename M>
	class packed_map_ptr
	{
		friend class dense_map_iterator;

	public:
		using element_type = std::pair<K, M>;
		using reference = std::pair<K &, M &>;
		using pointer = const reference *;

	public:
		constexpr packed_map_ptr() noexcept : packed_map_ptr(nullptr, nullptr) {}
		template<typename U, typename = std::enable_if_t<!std::is_same_v<U, M> && std::is_convertible_v<U &, M &>>>
		constexpr explicit packed_map_ptr(const packed_map_ptr<K, U> other) noexcept : packed_map_ptr(other.m_ref.first, other.m_ref.second) {}

		constexpr packed_map_ptr(K *first, M *second) noexcept : m_ref(*first, *second) {}
		template<typename U, typename V, typename = std::enable_if_t<std::is_convertible_v<U *, K *> && std::is_convertible_v<V *, M *>>>
		constexpr explicit packed_map_ptr(std::pair<U, V> *ptr) noexcept : packed_map_ptr(&ptr->first, &ptr->second) {}
		template<typename U, typename V, typename = std::enable_if_t<std::is_convertible_v<const U *, K *> && std::is_convertible_v<const V *, M *>>>
		constexpr explicit packed_map_ptr(const std::pair<U, V> *ptr) noexcept : packed_map_ptr(&ptr->first, &ptr->second) {}

		[[nodiscard]] constexpr pointer operator->() const noexcept { return &m_ref; }
		[[nodiscard]] constexpr reference operator*() const noexcept { return m_ref; }

	private:
		reference m_ref;
	};
}