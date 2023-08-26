/*
 * Created by switchblade on 2022-11-14.
 */

#pragma once

#include "utility.hpp"
#include "../hash.hpp"

#ifndef TPP_USE_IMPORT

#include <initializer_list>

#else

#if defined(_MSC_VER) && (__cplusplus <= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG <= 202002L))

import std.memory;

#endif

#endif

namespace tpp::_detail
{
	/* Helper type used to store potentially empty objects. */
	template<typename, typename = void>
	struct empty_base;

	template<typename T>
	using ebo_candidate = std::conjunction<std::is_empty<T>, std::negation<std::is_final<T>>>;

	template<typename T>
	struct empty_base<T, std::enable_if_t<ebo_candidate<T>::value>> : T
	{
		constexpr empty_base() noexcept(std::is_nothrow_default_constructible_v<T>) = default;
		constexpr empty_base(const empty_base &other) noexcept(std::is_nothrow_copy_constructible_v<T>) = default;
		constexpr empty_base(empty_base &&other) noexcept(std::is_nothrow_move_constructible_v<T>) = default;
		constexpr empty_base &operator=(const empty_base &other) noexcept(std::is_nothrow_copy_assignable_v<T>) = default;
		constexpr empty_base &operator=(empty_base &&other) noexcept(std::is_nothrow_move_assignable_v<T>) = default;

		template<typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
		constexpr empty_base(Args &&...args) noexcept(std::is_nothrow_constructible_v<T, Args...>) : T(std::forward<Args>(args)...) {}

		template<typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
		constexpr empty_base(std::tuple<Args...> args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
				:    empty_base(std::make_index_sequence<sizeof...(Args)>(), std::forward<Args>(args)...) {}
		template<std::size_t... Is, typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
		constexpr empty_base(std::index_sequence<Is...>, std::tuple<Args...> args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
				: empty_base(std::forward<Args>(std::get<Is>(args))...) {}

		[[nodiscard]] constexpr T &value() noexcept { return static_cast<T &>(*this); }
		[[nodiscard]] constexpr const T &value() const noexcept { return static_cast<const T &>(*this); }

		void swap(empty_base &other) noexcept(std::is_nothrow_swappable_v<T>)
		{
			using std::swap;
			swap(value(), other.value());
		}
	};

	template<typename T>
	struct empty_base<T, std::enable_if_t<!ebo_candidate<T>::value>>
	{
		constexpr empty_base() noexcept(std::is_nothrow_default_constructible_v<T>) = default;
		constexpr empty_base(const empty_base &other) noexcept(std::is_nothrow_copy_constructible_v<T>) = default;
		constexpr empty_base(empty_base &&other) noexcept(std::is_nothrow_move_constructible_v<T>) = default;
		constexpr empty_base &operator=(const empty_base &other) noexcept(std::is_nothrow_copy_assignable_v<T>) = default;
		constexpr empty_base &operator=(empty_base &&other) noexcept(std::is_nothrow_move_assignable_v<T>) = default;

		template<typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
		constexpr empty_base(Args &&...args) noexcept(std::is_nothrow_constructible_v<T, Args...>) : m_value(std::forward<Args>(args)...) {}

		template<typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
		constexpr empty_base(std::tuple<Args...> args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
				: empty_base(std::make_index_sequence<sizeof...(Args)>(), std::forward<Args>(args)...) {}
		template<std::size_t... Is, typename... Args, typename = std::enable_if_t<std::is_constructible_v<T, Args...>>>
		constexpr empty_base(std::index_sequence<Is...>, std::tuple<Args...> args) noexcept(std::is_nothrow_constructible_v<T, Args...>)
				: empty_base(std::forward<Args>(std::get<Is>(args))...) {}

		[[nodiscard]] constexpr T &value() noexcept { return m_value; }
		[[nodiscard]] constexpr const T &value() const noexcept { return m_value; }

		void swap(empty_base &other) noexcept(std::is_nothrow_swappable_v<T>)
		{
			using std::swap;
			swap(value(), other.value());
		}

	private:
		T m_value;
	};

	template<typename T>
	void swap(empty_base<T> &a, empty_base<T> &b) noexcept(std::is_nothrow_swappable_v<T>) { a.swap(b); }

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
	struct empty_link { constexpr void swap(empty_link &) noexcept {} };
	/* Node link used for ordered tables. */
	struct ordered_link
	{
		[[nodiscard]] static std::ptrdiff_t byte_diff(const void *a, const void *b) noexcept
		{
			const auto a_bytes = static_cast<const std::uint8_t *>(a);
			const auto b_bytes = static_cast<const std::uint8_t *>(b);
			return a_bytes - b_bytes;
		}
		[[nodiscard]] static const void *byte_off(const void *p, std::ptrdiff_t n) noexcept
		{
			return static_cast<const std::uint8_t *>(p) + n;
		}
		[[nodiscard]] static void *byte_off(void *p, std::ptrdiff_t n) noexcept
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
			next = byte_diff(next_ptr, this);
			prev = byte_diff(prev_ptr, this);
			next_ptr->prev = -next;
			prev_ptr->next = -prev;
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

		[[nodiscard]] ordered_link *off(std::ptrdiff_t n) noexcept { return static_cast<ordered_link *>(byte_off(this, n)); }
		[[nodiscard]] const ordered_link *off(std::ptrdiff_t n) const noexcept { return static_cast<const ordered_link *>(byte_off(this, n)); }

		void swap(ordered_link &other) noexcept
		{
			std::swap(next, other.next);
			std::swap(prev, other.prev);
		}
		friend void swap(ordered_link &a, ordered_link &b) noexcept
		{
			auto a_next = a.off(a.next), b_next = b.off(b.next);
			auto a_prev = a.off(a.prev), b_prev = b.off(b.prev);
			a.link(b_next, b_prev);
			b.link(a_next, a_prev);
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

		static constexpr bool value = std::is_same_v<decltype(test_key_size<T>(nullptr)), std::true_type>;
	};
	template<typename T, bool = has_key_size<T>::value>
	struct node_hash { using type = std::array<std::size_t, T::key_size>; };
	template<typename T>
	struct node_hash<T, false> { using type = std::size_t; };

	template<typename V, typename A, typename Traits>
	class packed_node : public Traits::link_type
	{
		using hash_type = typename node_hash<Traits>::type;
		using link_base = typename Traits::link_type;

		struct storage_t : empty_base<V> { hash_type hash; };

	public:
		using allocator_type = typename std::allocator_traits<A>::template rebind_alloc<V>;

		packed_node() noexcept {}
		~packed_node() {}

		template<typename... Args, typename = std::enable_if_t<std::is_constructible_v<V, Args...>>>
		void construct(allocator_type &alloc, Args &&...args)
		{
			std::allocator_traits<allocator_type>::construct(alloc, &value(), std::forward<Args>(args)...);
		}
		void construct(allocator_type &alloc, const packed_node &other)
		{
			link_base::operator=(other);
			construct(alloc, other.value());
			hash() = other.hash();
		}
		void construct(allocator_type &alloc, packed_node &&other)
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
		void destroy(A &alloc) { std::allocator_traits<allocator_type>::destroy(alloc, &value()); }

		template<typename U>
		void relocate(U &alloc_dst, U &alloc_src, packed_node &src)
		{
			auto conv_alloc_dst = allocator_type{alloc_dst};
			auto conv_alloc_src = allocator_type{alloc_src};
			relocate(conv_alloc_dst, conv_alloc_src, src);
		}
		void relocate(allocator_type &alloc_dst, allocator_type &alloc_src, packed_node &src)
		{
			construct(alloc_dst, std::move(src));
			src.destroy(alloc_src);
		}

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
		using hash_type = typename node_hash<Traits>::type;
		using link_base = typename Traits::link_type;

	public:
		using allocator_type = typename std::allocator_traits<A>::template rebind_alloc<V>;
		using is_extractable = std::true_type;

	private:
		using value_ptr = typename std::allocator_traits<allocator_type>::pointer;

	public:
		class extracted_type : empty_base<allocator_type>
		{
			friend class stable_node;

			using alloc_base = empty_base<allocator_type>;

		public:
			constexpr extracted_type() noexcept = default;

			extracted_type(extracted_type &&other) noexcept : alloc_base(std::move(other)), m_ptr(std::exchange(other.m_ptr, {})) { assert_alloc(other); }
			extracted_type(allocator_type &&alloc, stable_node &&other) noexcept : alloc_base(std::move(alloc)), m_ptr(std::exchange(other.m_ptr, {})) {}

			extracted_type &operator=(extracted_type &&other) noexcept
			{
				if (this != &other)
				{
					destroy_impl();

					if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value)
						alloc_base::operator=(std::move(other));

					m_ptr = std::exchange(other.m_ptr, value_ptr{});
					assert_alloc(other);
				}
				return *this;
			}

			~extracted_type() { destroy_impl(); }

			[[nodiscard]] constexpr bool empty() const noexcept { return !m_ptr; }
			[[nodiscard]] constexpr operator bool() const noexcept { return m_ptr; }

			[[nodiscard]] A get_allocator() const { return A{alloc()}; }

			[[nodiscard]] constexpr auto &value() const noexcept { return *m_ptr; }
			[[nodiscard]] constexpr auto &key() const noexcept { return Traits::get_key(value()); }
			[[nodiscard]] constexpr auto &mapped() const noexcept { return Traits::get_mapped(value()); }

			void swap(extracted_type &other) noexcept(std::is_nothrow_swappable_v<A>)
			{
				using std::swap;
				if constexpr (std::allocator_traits<allocator_type>::propagate_on_container_swap::value)
					swap(alloc(), other.alloc());

				assert_alloc(other);
				swap(m_ptr, other.m_ptr);
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
				if (m_ptr)
				{
					std::allocator_traits<allocator_type>::destroy(alloc(), m_ptr);
					std::allocator_traits<allocator_type>::deallocate(alloc(), m_ptr, 1);
				}
			}

			value_ptr m_ptr = {};
		};

		/* NOTE: Template signature is defined by the standard. */
		template<typename Iter, typename NodeType>
		struct insert_return
		{
			constexpr insert_return() = default;
			constexpr insert_return(const insert_return &) = default;
			constexpr insert_return &operator=(const insert_return &) = default;
			constexpr insert_return(insert_return &&) = default;
			constexpr insert_return &operator=(insert_return &&) = default;

			constexpr insert_return(const Iter &pos, bool ins) : position(pos), inserted(ins), node() {}
			constexpr insert_return(Iter &&pos, bool ins) : position(std::move(pos)), inserted(ins), node() {}

			constexpr insert_return(const Iter &pos, bool ins, const NodeType &node) : position(pos), inserted(ins), node(node) {}
			constexpr insert_return(Iter &&pos, bool ins, NodeType &&node) : position(std::move(pos)), inserted(ins), node(std::move(node)) {}

			template<typename I, typename = std::enable_if_t<std::is_constructible_v<Iter, I &&>>>
			constexpr insert_return(insert_return<I, NodeType> &&other) : insert_return(std::move(other.position), other.inserted, std::move(other.node)) {}

			Iter position = {};
			bool inserted = {};
			NodeType node = {};
		};

	public:
		constexpr stable_node() noexcept = default;

		void construct(allocator_type &alloc, const stable_node &other)
		{
			link_base::operator=(other);
			construct(alloc, other.value());
			m_hash = other.m_hash;
		}
		void construct(allocator_type &, stable_node &&other) { move_from(other); }
		void construct(allocator_type &alloc, extracted_type &&other)
		{
			using std::swap;
			if (allocator_eq(alloc, other.alloc()))
				m_ptr = std::exchange(other.m_ptr, value_ptr{});
			else
			{
				construct(alloc, std::move(other.value()));
				other.destroy_impl();
			}
		}
		void move_from(stable_node &other)
		{
			link_base::operator=(std::move(other));
			m_ptr = std::exchange(other.m_ptr, value_ptr{});
			m_hash = other.m_hash;
		}

		template<typename U>
		void relocate(U &, U &, stable_node &other) { move_from(other); }

		template<typename... Args, typename = std::enable_if_t<std::is_constructible_v<V, Args...>>>
		void construct(allocator_type &alloc, Args &&...args)
		{
			m_ptr = std::allocator_traits<allocator_type>::allocate(alloc, 1);
			std::allocator_traits<allocator_type>::construct(alloc, m_ptr, std::forward<Args>(args)...);
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
				std::allocator_traits<allocator_type>::destroy(alloc, m_ptr);
				std::allocator_traits<allocator_type>::deallocate(alloc, m_ptr, 1);
				m_ptr = value_ptr{};
			}
		}

		[[nodiscard]] constexpr auto &hash() noexcept { return m_hash; }
		[[nodiscard]] constexpr auto &hash() const noexcept { return m_hash; }

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
			using std::swap;
			a.link_base::swap(b);
			swap(a.m_ptr, b.m_ptr);
			swap(a.m_hash, b.m_hash);
		}

	private:
		value_ptr m_ptr = {};
		hash_type m_hash = 0;
	};

	/* Need a special relocate functor for packed_node & stable_node since they require an injected value allocator. */
	struct relocate_node
	{
		template<typename A, typename Ptr>
		void operator()(A &alloc_src, Ptr src, A &alloc_dst, Ptr dst) const { dst->relocate(alloc_dst, alloc_src, *src); }
	};

	/* Helper used to check if a node is extractable. */
	template<typename, typename = void>
	struct is_extractable : std::false_type {};
	template<typename T>
	struct is_extractable<T, std::void_t<typename T::is_extractable>> : std::true_type {};

	template<typename N, typename A>
	struct ordered_iterator
	{
		using link_t = std::conditional_t<std::is_const_v<N>, std::add_const_t<ordered_link>, ordered_link>;

		using link_ptr = typename std::allocator_traits<A>::template rebind_traits<link_t>::pointer;
		using node_ptr = typename std::allocator_traits<A>::template rebind_traits<N>::pointer;

		using value_type = N;
		using reference = N &;
		using pointer = node_ptr;

		using size_type = typename std::allocator_traits<A>::size_type;
		using difference_type = typename std::allocator_traits<A>::difference_type;
		using iterator_category = std::bidirectional_iterator_tag;

		constexpr ordered_iterator() noexcept = default;

		/** Implicit conversion from a const iterator. */
		template<typename U, typename = std::enable_if_t<!std::is_same_v<U, N>>>
		constexpr ordered_iterator(const ordered_iterator<U, A> &other) noexcept : link(other.link) {}

		constexpr explicit ordered_iterator(link_ptr link) noexcept : link(link) {}
		constexpr explicit ordered_iterator(node_ptr node) noexcept : ordered_iterator(link_ptr{node}) {}

		ordered_iterator operator++(int) noexcept
		{
			auto tmp = *this;
			operator++();
			return tmp;
		}
		ordered_iterator &operator++() noexcept
		{
			link = link->off(link->next);
			return *this;
		}
		ordered_iterator operator--(int) noexcept
		{
			auto tmp = *this;
			operator--();
			return tmp;
		}
		ordered_iterator &operator--() noexcept
		{
			link = link->off(link->prev);
			return *this;
		}

		[[nodiscard]] pointer operator->() const noexcept { return pointer{static_cast<N *>(to_address(link))}; }
		[[nodiscard]] reference operator*() const noexcept { return *operator->(); }

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

		link_ptr link = {};
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
		template<typename, typename, typename>
		friend class table_iterator;
		friend struct random_access_iterator_base<table_iterator<V, Traits, I>, std::iterator_traits<I>>;

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
		template<typename U, typename J, typename = std::enable_if_t<!std::is_same_v<U, V> && std::is_convertible_v<J, I>>>
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
	struct packed_map_ref
	{
		template<typename U, typename V>
		constexpr packed_map_ref &operator=(packed_map_ref<U, V> &&other)
		{
			first = std::move(other.first);
			second = std::move(other.second);
			return *this;
		}
		template<typename U, typename V>
		constexpr packed_map_ref &operator=(const packed_map_ref<U, V> &other)
		{
			first = other.first;
			second = other.second;
			return *this;
		}

		template<typename U, typename V>
		constexpr packed_map_ref(U &first, V &second) noexcept : first(first), second(second) {}
		template<typename U, typename V>
		constexpr packed_map_ref(std::pair<U &, V &> ref) noexcept : packed_map_ref(ref.first, ref.second) {}

		template<typename U, typename V>
		constexpr packed_map_ref(std::pair<U, V> &ref) noexcept : packed_map_ref(ref.first, ref.second) {}
		template<typename U, typename V>
		constexpr packed_map_ref(const std::pair<U, V> &ref) noexcept : packed_map_ref(ref.first, ref.second) {}

		[[nodiscard]] constexpr operator std::pair<K &, M &>() const noexcept { return {first, second}; }

#if (__cplusplus < 202002L && (!defined(_MSVC_LANG) || _MSVC_LANG < 202002L))
		template<typename U, typename V>
		[[nodiscard]] constexpr bool operator==(const packed_map_ref<U, V> &other) const noexcept(noexcept(first == other.first && noexcept(second == other.second)))  { return first == other.first && second == other.second; }
		template<typename U, typename V>
		[[nodiscard]] constexpr bool operator!=(const packed_map_ref<U, V> &other) const noexcept(noexcept(first != other.first && noexcept(second != other.second))) { return first != other.first || second != other.second; }
		template<typename U, typename V>
		[[nodiscard]] constexpr bool operator>=(const packed_map_ref<U, V> &other) const noexcept(noexcept(first >= other.first && noexcept(second >= other.second)))  { return first >= other.first && second >= other.second; }
		template<typename U, typename V>
		[[nodiscard]] constexpr bool operator<=(const packed_map_ref<U, V> &other) const noexcept(noexcept(first <= other.first && noexcept(second <= other.second)))  { return first <= other.first && second <= other.second; }
		template<typename U, typename V>
		[[nodiscard]] constexpr bool operator>(const packed_map_ref<U, V> &other) const noexcept(noexcept(first > other.first && noexcept(second > other.second)))  { return first > other.first && second > other.second; }
		template<typename U, typename V>
		[[nodiscard]] constexpr bool operator<(const packed_map_ref<U, V> &other) const noexcept(noexcept(first < other.first && noexcept(second < other.second))) { return first < other.first && second < other.second; }
#else
		template<typename U, typename V> requires std::equality_comparable_with<K, U> && std::equality_comparable_with<M, V>
		[[nodiscard]] constexpr bool operator==(const packed_map_ref<U, V> &other) const noexcept(noexcept(first == other.first && noexcept(second == other.second)))  { return first == other.first && second == other.second; }
		template<typename U, typename V> requires(requires (const K &k, const M &m, const U &u, const V &v) { synth_three_way(k, u); synth_three_way(m, v); })
		[[nodiscard]] constexpr auto operator<=>(const packed_map_ref<U, V> &other) const noexcept(noexcept(synth_three_way(first, other.first)) && noexcept(synth_three_way(second, other.second))) -> std::common_comparison_category_t<synth_three_way_result<K, U>, synth_three_way_result<M, V>>
		{
			if (auto res = synth_three_way(first, other.first); res == 0)
				return synth_three_way(second, other.second);
			else
				return res;
		}
#endif

		K &first;
		M &second;
	};
	template<typename K, typename M>
	class packed_map_ptr
	{
		friend class dense_map_iterator;

	public:
		using element_type = std::pair<K, M>;
		using reference = packed_map_ref<K, M>;
		using pointer = const reference *;

	public:
		constexpr packed_map_ptr() noexcept : packed_map_ptr({}, {}) {}
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

	template<std::size_t I, typename K, typename M, typename = std::enable_if_t<(I < 2)>>
	[[nodiscard]] constexpr decltype(auto) get(tpp::_detail::packed_map_ref<K, M> &ref) noexcept
	{
		if constexpr (I == 0)
			return ref.first;
		else
			return ref.second;
	}
	template<std::size_t I, typename K, typename M, typename = std::enable_if_t<(I < 2)>>
	[[nodiscard]] constexpr decltype(auto) get(const tpp::_detail::packed_map_ref<K, M> &ref) noexcept
	{
		if constexpr (I == 0)
			return ref.first;
		else
			return ref.second;
	}
}

template<typename K, typename M>
struct std::tuple_size<tpp::_detail::packed_map_ref<K, M>> : std::integral_constant<std::size_t, 2> {};
template<std::size_t I, typename K, typename M>
struct std::tuple_element<I, tpp::_detail::packed_map_ref<K, M>> { using type = std::conditional_t<I == 0, K, M>; };