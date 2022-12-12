/*
 * Created by switchblade on 11/7/22.
 */

#pragma once

#include "utility.hpp"

#ifndef TPP_USE_IMPORT

#include <limits>
#include <tuple>

#endif

namespace tpp::detail
{
	template<typename V, typename K, typename Kh, typename Kc, typename Alloc, typename ValueTraits>
	struct dense_table_traits : table_traits<V, V, K, Kh, Kc, Alloc>
	{
		using size_type = typename table_traits<V, V, K, Kh, Kc, Alloc>::size_type;

		constexpr static size_type key_size = ValueTraits::key_size;

		constexpr static size_type npos = std::numeric_limits<size_type>::max();
		constexpr static size_type initial_capacity = 8;

		using is_transparent = std::conjunction<detail::is_transparent<Kh>, detail::is_transparent<Kc>>;
		using is_ordered = detail::is_ordered<typename ValueTraits::link_type>;

		using bucket_link = typename ValueTraits::link_type;
		using bucket_hash = std::array<std::size_t, ValueTraits::key_size>;
		using bucket_pos = std::array<size_type, ValueTraits::key_size>;

		struct bucket_node : table_node<V, Alloc, ValueTraits>
		{
			template<typename... Args>
			TPP_CXX20_CONSTEXPR void construct(Alloc &alloc, Args &&...args)
			{
				table_node<V, Alloc, ValueTraits>::construct(alloc, std::forward<Args>(args)...);
				next = make_array<key_size>(npos);
			}
			TPP_CXX20_CONSTEXPR void construct(Alloc &alloc, bucket_node &&other)
			{
				table_node<V, Alloc, ValueTraits>::construct(alloc, std::forward<table_node<V, Alloc, ValueTraits>>(other));
				next = other.next;
			}
			TPP_CXX20_CONSTEXPR void move_from(bucket_node &other)
			{
				table_node<V, Alloc, ValueTraits>::move_from(other);
				next = other.next;
			}

			bucket_pos next;
		};

		using sparse_allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<bucket_pos>;
		using dense_allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<bucket_node>;
	};

	template<typename V, typename K, typename Kh, typename Kc, typename Alloc, typename ValueTraits>
	class dense_table :
			ValueTraits::link_type,
			ebo_container<typename dense_table_traits<V, V, Kh, Kc, Alloc, ValueTraits>::sparse_allocator>,
			ebo_container<typename dense_table_traits<V, V, Kh, Kc, Alloc, ValueTraits>::dense_allocator>,
			ebo_container<Kh>,
			ebo_container<Kc>
	{
		using traits_t = dense_table_traits<V, V, Kh, Kc, Alloc, ValueTraits>;

	public:
		using insert_type = typename traits_t::insert_type;
		using value_type = typename traits_t::value_type;
		using key_type = typename traits_t::key_type;

		using hasher = typename traits_t::hasher;
		using key_equal = typename traits_t::key_equal;
		using allocator_type = typename traits_t::allocator_type;

		using size_type = typename traits_t::size_type;
		using difference_type = typename traits_t::difference_type;

		using is_transparent = typename traits_t::is_transparent;
		using is_ordered = typename traits_t::is_ordered;

		constexpr static size_type npos = traits_t::npos;
		constexpr static size_type key_size = traits_t::key_size;
		constexpr static size_type initial_capacity = traits_t::initial_capacity;

		constexpr static float initial_load_factor = .875f;

	private:
		using bucket_node = typename traits_t::bucket_node;
		using bucket_hash = typename traits_t::bucket_hash;
		using bucket_link = typename traits_t::bucket_link;
		using sparse_allocator = typename traits_t::sparse_allocator;
		using dense_allocator = typename traits_t::dense_allocator;

		using bucket_pos = typename traits_t::bucket_pos;
		using chain_slice = std::array<size_type *, key_size>;

		using header_base = bucket_link;
		using hash_base = ebo_container<hasher>;
		using cmp_base = ebo_container<key_equal>;
		using sparse_alloc_base = ebo_container<sparse_allocator>;
		using dense_alloc_base = ebo_container<dense_allocator>;

		using node_iterator = std::conditional_t<is_ordered::value, ordered_iterator<bucket_node, traits_t>, bucket_node *>;
		using const_node_iterator = std::conditional_t<is_ordered::value, ordered_iterator<const bucket_node, traits_t>, const bucket_node *>;

		template<typename N>
		class bucket_iterator
		{
			// @formatter:off
			template<typename>
			friend class bucket_iterator;
			// @formatter:on

			using value_pointer = std::conditional_t<std::is_const_v<N>, typename ValueTraits::const_pointer, typename ValueTraits::pointer>;
			using value_reference = std::conditional_t<std::is_const_v<N>, typename ValueTraits::const_reference, typename ValueTraits::reference>;

		public:
			using value_type = V;

			using reference = std::conditional_t<key_size == 1, value_reference, std::array<value_reference, key_size>>;
			struct pointer
			{
				friend class bucket_iterator;

				constexpr pointer(std::array<value_pointer, key_size> ptr) noexcept : m_ptr(ptr) {}

			public:
				pointer() = delete;

				constexpr pointer(const pointer &) noexcept = default;
				constexpr pointer &operator=(const pointer &) noexcept = default;
				constexpr pointer(pointer &&) noexcept = default;
				constexpr pointer &operator=(pointer &&) noexcept = default;

				[[nodiscard]] constexpr reference operator->() const noexcept { return get(std::make_index_sequence<key_size>{}); }

			private:
				template<std::size_t I, std::size_t... Is>
				[[nodiscard]] constexpr reference get(std::index_sequence<I, Is...>) const noexcept
				{
					if constexpr (sizeof...(Is) != 0)
						return reference{*m_ptr[Is]...};
					else
						return reference{*m_ptr[I]};
				}

				std::array<value_pointer, key_size> m_ptr = {};
			};

			using size_type = typename bucket_node::size_type;
			using difference_type = typename bucket_node::difference_type;
			using iterator_category = std::forward_iterator_tag;

		public:
			constexpr bucket_iterator() noexcept = default;
			template<typename U, typename = std::enable_if_t<!std::is_same_v<N, U> && std::is_constructible_v<U *, std::add_const_t<N> *>>>
			constexpr bucket_iterator(const bucket_iterator<U> &other) noexcept : m_base(other.m_base), m_pos(other.m_pos) {}

			constexpr explicit bucket_iterator(N *base, bucket_pos pos) noexcept : m_base(base), m_pos(pos) {}

			constexpr bucket_iterator operator++(int) noexcept
			{
				auto tmp = *this;
				inc(std::make_index_sequence<key_size>{});
				return tmp;
			}
			constexpr bucket_iterator &operator++() noexcept
			{
				inc(std::make_index_sequence<key_size>{});
				return *this;
			}

			[[nodiscard]] constexpr pointer operator->() const noexcept { return pointer{&node()->value()}; }
			[[nodiscard]] constexpr reference operator*() const noexcept { return *operator->(); }

			[[nodiscard]] constexpr bool operator==(const bucket_iterator &other) const noexcept { return m_pos == other.m_pos; }
#if __cplusplus < 202002L
			[[nodiscard]] constexpr bool operator!=(const bucket_iterator &other) const noexcept { return m_pos != other.m_pos; }
#endif

		private:
			[[nodiscard]] constexpr N *node() const noexcept { return m_base + m_pos; }

			template<std::size_t... Is>
			constexpr void increment(std::index_sequence<Is...>) noexcept { ((m_pos[Is] = node()->next[Is]), ...); }

			N *m_base = nullptr;
			bucket_pos m_pos = 0;
		};

	public:
		using iterator = table_iterator<value_type, ValueTraits, node_iterator>;
		using const_iterator = table_iterator<const value_type, ValueTraits, const_node_iterator>;
		using reverse_iterator = std::reverse_iterator<iterator>;
		using const_reverse_iterator = std::reverse_iterator<const_iterator>;
		using local_iterator = bucket_iterator<bucket_node>;
		using const_local_iterator = bucket_iterator<const bucket_node>;

		using reference = typename iterator::reference;
		using const_reference = typename const_iterator::reference;
		using pointer = typename iterator::pointer;
		using const_pointer = typename const_iterator::pointer;

	private:
		template<typename T>
		TPP_CXX20_CONSTEXPR static void replace_value(bucket_node &node, T &&value)
		{
			auto &mapped = node.mapped();
			using mapped_t = std::remove_reference_t<decltype(mapped)>;

			/* If move-assign is possible, do that. Otherwise, re-init the value. */
			if constexpr (std::is_assignable_v<mapped_t, T>)
				mapped = std::forward<T>(value);
			else
			{
				if constexpr (!std::is_trivially_destructible_v<mapped_t>)
					mapped.~mapped_t();

				static_assert(std::is_constructible_v<mapped_t, T>);
				new(&mapped) mapped_t(std::forward<T>(value));
			}
		}
		template<typename... Args>
		TPP_CXX20_CONSTEXPR static void replace_value(bucket_node &node, Args &&...args)
		{
			auto &mapped = node.mapped();
			using mapped_t = std::remove_reference_t<decltype(mapped)>;

			if constexpr (!std::is_trivially_destructible_v<mapped_t>)
				mapped.~mapped_t();

			static_assert(std::is_constructible_v<mapped_t, Args...>);
			new(&mapped) mapped_t(std::forward<Args>(args)...);
		}

	public:
		TPP_CXX20_CONSTEXPR dense_table() { init(initial_capacity); }

		TPP_CXX20_CONSTEXPR dense_table(const allocator_type &alloc) : sparse_alloc_base(sparse_allocator{alloc}), dense_alloc_base(dense_allocator{alloc})
		{
			init(initial_capacity);
		}
		TPP_CXX20_CONSTEXPR dense_table(size_type bucket_count, const hasher &hash, const key_equal &cmp, const allocator_type &alloc)
				: sparse_alloc_base(alloc), dense_alloc_base(alloc), hash_base(hash), cmp_base(cmp) { init(bucket_count); }

		template<typename Iter>
		TPP_CXX20_CONSTEXPR dense_table(Iter first, Iter last, size_type bucket_count, const hasher &hash, const key_equal &cmp, const allocator_type &alloc)
				: dense_table(bucket_count, hash, cmp, alloc) { insert(first, last); }

		TPP_CXX20_CONSTEXPR dense_table(const dense_table &other)
				: header_base(other),
				  sparse_alloc_base(std::allocator_traits<sparse_allocator>::select_on_container_copy_construction(other.get_sparse_alloc())),
				  dense_alloc_base(std::allocator_traits<dense_allocator>::select_on_container_copy_construction(other.get_dense_alloc())),
				  hash_base(other), cmp_base(other),
				  m_max_load_factor(other.m_max_load_factor)
		{
			copy_data(other);
		}
		TPP_CXX20_CONSTEXPR dense_table(const dense_table &other, const allocator_type &alloc)
				: header_base(other),
				  sparse_alloc_base(alloc),
				  dense_alloc_base(alloc),
				  hash_base(other),
				  cmp_base(other),
				  m_max_load_factor(other.m_max_load_factor)
		{
			copy_data(other);
		}

		TPP_CXX20_CONSTEXPR dense_table(dense_table &&other)
		noexcept(std::is_nothrow_move_constructible_v<sparse_allocator> &&
		         std::is_nothrow_move_constructible_v<dense_allocator> &&
		         std::is_nothrow_move_constructible_v<hasher> &&
		         std::is_nothrow_move_constructible_v<key_equal>)
				: header_base(std::move(other)),
				  sparse_alloc_base(std::move(other)), dense_alloc_base(std::move(other)),
				  hash_base(std::move(other)), cmp_base(std::move(other)),
				  m_max_load_factor(other.m_max_load_factor)
		{
			move_from(other);
		}
		TPP_CXX20_CONSTEXPR dense_table(dense_table &&other, const allocator_type &alloc)
		noexcept(std::is_nothrow_constructible_v<sparse_allocator, sparse_allocator> &&
		         std::is_nothrow_constructible_v<dense_allocator, dense_allocator> &&
		         std::is_nothrow_move_constructible_v<hasher> &&
		         std::is_nothrow_move_constructible_v<key_equal>)
				: header_base(std::move(other)),
				  sparse_alloc_base(sparse_allocator{alloc}), dense_alloc_base(dense_allocator{alloc}),
				  hash_base(std::move(other)), cmp_base(std::move(other)),
				  m_max_load_factor(other.m_max_load_factor)
		{
			move_from(other);
		}

		TPP_CXX20_CONSTEXPR dense_table &operator=(const dense_table &other)
		{
			if (this != &other)
			{
				header_base::operator=(other);
				hash_base::operator=(other);
				cmp_base::operator=(other);

				m_max_load_factor = other.m_max_load_factor;
				clear_data();

				if constexpr (std::allocator_traits<sparse_allocator>::propagate_on_container_copy_assignment::value)
				{
					std::allocator_traits<sparse_allocator>::deallocate(std::exchange(m_sparse, nullptr), std::exchange(m_sparse_size, 0));
					sparse_alloc_base::operator=(other);
				}
				if constexpr (std::allocator_traits<dense_allocator>::propagate_on_container_copy_assignment::value)
				{
					std::allocator_traits<sparse_allocator>::deallocate(std::exchange(m_dense, nullptr), std::exchange(m_dense_capacity, 0));
					dense_alloc_base::operator=(other);
				}

				copy_data(other);
			}
			return *this;
		}
		TPP_CXX20_CONSTEXPR dense_table &operator=(dense_table &&other)
		noexcept(std::is_nothrow_move_assignable_v<sparse_allocator> &&
		         std::is_nothrow_move_assignable_v<dense_allocator> &&
		         std::is_nothrow_move_assignable_v<hasher> &&
		         std::is_nothrow_move_assignable_v<key_equal>)
		{
			if (this != &other)
			{
				header_base::operator=(std::move(other));
				hash_base::operator=(std::move(other));
				cmp_base::operator=(std::move(other));

				m_max_load_factor = other.m_max_load_factor;
				clear_data();

				if constexpr (std::allocator_traits<sparse_allocator>::propagate_on_container_move_assignment::value)
				{
					std::allocator_traits<sparse_allocator>::deallocate(std::exchange(m_sparse, nullptr), std::exchange(m_sparse_size, 0));
					sparse_alloc_base::operator=(std::move(other));
				}
				if constexpr (std::allocator_traits<dense_allocator>::propagate_on_container_move_assignment::value)
				{
					std::allocator_traits<sparse_allocator>::deallocate(std::exchange(m_dense, nullptr), std::exchange(m_dense_capacity, 0));
					dense_alloc_base::operator=(std::move(other));
				}

				/* By this point `this` is clear and potentially buffer-less. */
				move_from(other);
			}
			return *this;
		}

		template<typename Iter>
		TPP_CXX20_CONSTEXPR void assign(Iter first, Iter last)
		{
			clear();
			insert(first, last);
		}

		[[nodiscard]] constexpr iterator begin() noexcept { return to_iter(begin_node()); }
		[[nodiscard]] constexpr const_iterator begin() const noexcept { return to_iter(begin_node()); }
		[[nodiscard]] constexpr iterator end() noexcept { return to_iter(end_node()); }
		[[nodiscard]] constexpr const_iterator end() const noexcept { return to_iter(end_node()); }

		[[nodiscard]] constexpr reverse_iterator rbegin() noexcept { return reverse_iterator{end()}; }
		[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator{end()}; }
		[[nodiscard]] constexpr reverse_iterator rend() noexcept { return reverse_iterator{begin()}; }
		[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator{begin()}; }

		[[nodiscard]] constexpr reference front() noexcept { return *to_iter(front_node()); }
		[[nodiscard]] constexpr const_reference front() const noexcept { return *to_iter(front_node()); }
		[[nodiscard]] constexpr reference back() noexcept { return *to_iter(back_node()); }
		[[nodiscard]] constexpr const_reference back() const noexcept { return *to_iter(back_node()); }

		[[nodiscard]] constexpr size_type size() const noexcept { return m_dense_size; }
		[[nodiscard]] constexpr size_type max_size() const noexcept { return to_capacity(npos - 1); }
		[[nodiscard]] constexpr size_type capacity() const noexcept { return to_capacity(bucket_count()); }
		[[nodiscard]] constexpr float load_factor() const noexcept { return static_cast<float>(size()) / static_cast<float>(bucket_count()); }

		[[nodiscard]] constexpr local_iterator begin(size_type n) noexcept { return local_iterator{m_dense, m_sparse[n]}; }
		[[nodiscard]] constexpr const_local_iterator begin(size_type n) const noexcept { return const_local_iterator{m_dense, m_sparse[n]}; }
		[[nodiscard]] constexpr local_iterator end(size_type) noexcept { return local_iterator{}; }
		[[nodiscard]] constexpr const_local_iterator end(size_type) const noexcept { return const_local_iterator{}; }

		[[nodiscard]] constexpr size_type bucket_count() const noexcept { return m_sparse_size; }
		[[nodiscard]] constexpr size_type max_bucket_count() const noexcept { return std::numeric_limits<size_type>::max(); }
		[[nodiscard]] constexpr size_type bucket_size(size_type n) const noexcept { return static_cast<size_type>(std::distance(begin(n), end(n))); }
		template<typename T>
		[[nodiscard]] TPP_CXX20_CONSTEXPR size_type bucket(const T &key) const { return hash(key) % bucket_count(); }

		TPP_CXX20_CONSTEXPR void clear()
		{
			/* Reset header link. */
			if constexpr (is_ordered::value) *header_link() = bucket_link{};

			/* Reset buckets. */
			std::fill_n(m_sparse, bucket_count(), make_array<key_size, size_type>(npos));

			/* Destroy entries. */
			clear_data();
		}
		TPP_CXX20_CONSTEXPR void reserve(size_type n)
		{
			[[maybe_unused]] const auto front_pos = front_node() - m_dense;
			[[maybe_unused]] const auto back_pos = back_node() - m_dense;

			reserve_data(n);
			rehash(static_cast<size_type>(static_cast<float>(n) / m_max_load_factor));
		}

		template<std::size_t I, typename T>
		[[nodiscard]] TPP_CXX20_CONSTEXPR bool contains(const T &key) const { return (*find_node<I>(key, hash(key)).second) != npos; }

		template<std::size_t I, typename T>
		[[nodiscard]] TPP_CXX20_CONSTEXPR iterator find(const T &key) { return to_iter(find_node<I>(key, hash(key)).first); }
		template<std::size_t I, typename T>
		[[nodiscard]] TPP_CXX20_CONSTEXPR const_iterator find(const T &key) const { return to_iter(find_node<I>(key, hash(key)).first); }

		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> insert(const insert_type &value) { return insert_impl({}, ValueTraits::key_get(value), value); }
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> insert(insert_type &&value) { return insert_impl({}, ValueTraits::key_get(value), std::move(value)); }
		TPP_CXX20_CONSTEXPR iterator insert(const_iterator hint, const insert_type &value)
		{
			return insert_impl(to_underlying(hint), ValueTraits::key_get(value), value).first;
		}
		TPP_CXX20_CONSTEXPR iterator insert(const_iterator hint, insert_type &&value)
		{
			return insert_impl(to_underlying(hint), ValueTraits::key_get(value), std::move(value)).first;
		}

		template<typename T, typename = std::enable_if_t<!(std::is_convertible_v<T &&, insert_type &&> || std::is_convertible_v<T &&, value_type &&>)>>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> insert(T &&value) TPP_REQUIRES((std::is_constructible_v<V, T>))
		{
			return emplace_impl({}, std::forward<T>(value));
		}
		template<typename T, typename = std::enable_if_t<!(std::is_convertible_v<T &&, insert_type &&> || std::is_convertible_v<T &&, value_type &&>)>>
		TPP_CXX20_CONSTEXPR iterator insert(const_iterator hint, T &&value) TPP_REQUIRES((std::is_constructible_v<V, T>))
		{
			return emplace_impl(to_underlying(hint), std::forward<T>(value)).first;
		}

		template<typename Iter>
		TPP_CXX20_CONSTEXPR void insert(Iter first, Iter last)
		{
			if constexpr (std::is_base_of_v<std::random_access_iterator_tag, typename std::iterator_traits<Iter>::iterator_category>)
				reserve(static_cast<size_type>(std::distance(first, last)));
			for (; first != last; ++first) insert(*first);
		}

		template<typename T, typename U>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> insert_or_assign(const T &key, U &&value) TPP_REQUIRES((std::is_constructible_v<V, T, U>))
		{
			return insert_or_assign_impl({}, key, std::forward<U>(value));
		}
		template<typename T, typename U>
		TPP_CXX20_CONSTEXPR iterator insert_or_assign(const_iterator hint, const T &key, U &&value) TPP_REQUIRES((std::is_constructible_v<V, T, U>))
		{
			return insert_or_assign_impl(to_underlying(hint), key, std::forward<U>(value)).first;
		}

		template<typename... Args>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> emplace(Args &&...args) TPP_REQUIRES((std::is_constructible_v<V, Args...>))
		{
			return emplace_impl({}, std::forward<Args>(args)...);
		}
		template<typename... Args>
		TPP_CXX20_CONSTEXPR iterator emplace_hint(const_iterator hint, Args &&...args) TPP_REQUIRES((std::is_constructible_v<V, Args...>))
		{
			return emplace_impl(to_underlying(hint), std::forward<Args>(args)...);
		}

		template<typename U, typename... Args>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> emplace_or_replace(U &&key, Args &&...args) TPP_REQUIRES(
				(std::is_constructible_v<V, std::piecewise_construct_t, std::tuple<U &&>, std::tuple<Args && ...>>))
		{
			return insert_or_assign_impl({}, key, std::forward<Args>(args)...);
		}
		template<typename U, typename... Args>
		TPP_CXX20_CONSTEXPR iterator emplace_or_replace(const_iterator hint, U &&key, Args &&...args) TPP_REQUIRES(
				(std::is_constructible_v<V, std::piecewise_construct_t, std::tuple<U &&>, std::tuple<Args && ...>>))
		{
			return insert_or_assign_impl(hint, key, std::forward<Args>(args)...);
		}

		template<typename... Ks, typename... Args>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> try_emplace(std::tuple<Ks...> keys, Args &&...args) TPP_REQUIRES(
				(std::is_constructible_v<V, std::piecewise_construct_t, std::tuple<Ks ...>, std::tuple<Args && ...>>))
		{
			return insert_impl({}, keys, std::piecewise_construct, keys, std::forward_as_tuple(std::forward<Args>(args)...));
		}
		template<typename... Ks, typename... Args>
		TPP_CXX20_CONSTEXPR iterator try_emplace(const_iterator hint, std::tuple<Ks...> keys, Args &&...args) TPP_REQUIRES(
				(std::is_constructible_v<V, std::piecewise_construct_t, std::tuple<Ks ...>, std::tuple<Args && ...>>))
		{
			return insert_impl(to_underlying(hint), keys, std::piecewise_construct, keys, std::forward_as_tuple(std::forward<Args>(args)...)).first;
		}

		template<std::size_t I, typename T, typename = std::enable_if_t<!std::is_convertible_v<T, const_iterator>>>
		TPP_CXX20_CONSTEXPR iterator erase(const T &key) { return erase_impl<I>(key, hash(key)); }
		TPP_CXX20_CONSTEXPR iterator erase(const_iterator where)
		{
			const auto pos = &(*to_underlying(where)) - m_dense;
			return erase_impl(pos, m_dense + pos);
		}
		TPP_CXX20_CONSTEXPR iterator erase(const_iterator first, const_iterator last)
		{
			iterator result = end();
			while (last != first) result = erase(--last);
			return result;
		}

		TPP_CXX20_CONSTEXPR void rehash(size_type new_cap)
		{
			using std::max;

			/* Adjust the capacity to be at least large enough to fit the current size. */
			new_cap = max(max(static_cast<size_type>(static_cast<float>(size()) / m_max_load_factor), new_cap), initial_capacity);
			if (new_cap != m_sparse_size) rehash_impl(new_cap);
		}

		[[nodiscard]] constexpr float max_load_factor() const noexcept { return m_max_load_factor; }
		constexpr void max_load_factor(float f) noexcept { m_max_load_factor = f; }

		[[nodiscard]] TPP_CXX20_CONSTEXPR auto &get_allocator() const { return get_dense_alloc(); }
		[[nodiscard]] TPP_CXX20_CONSTEXPR auto &get_hash() const noexcept { return hash_base::value(); }
		[[nodiscard]] TPP_CXX20_CONSTEXPR auto &get_cmp() const noexcept { return cmp_base::value(); }

		TPP_CXX20_CONSTEXPR void swap(dense_table &other)
		noexcept(std::is_nothrow_swappable_v<sparse_allocator> &&
		         std::is_nothrow_swappable_v<dense_allocator> &&
		         std::is_nothrow_swappable_v<hasher> &&
		         std::is_nothrow_swappable_v<key_equal>)
		{
			using std::swap;

			header_base::swap(other);
			hash_base::swap(other);
			cmp_base::swap(other);

			if constexpr (std::allocator_traits<sparse_allocator>::propagate_on_container_swap::value)
				swap(get_sparse_alloc(), other.get_sparse_alloc());
			if constexpr (std::allocator_traits<dense_allocator>::propagate_on_container_swap::value)
				swap(get_dense_alloc(), other.get_dense_alloc());

			TPP_ASSERT(std::allocator_traits<sparse_allocator>::is_always_equal::value || get_sparse_alloc() == other.get_sparse_alloc(),
			           "Swapped allocators must be equal");
			TPP_ASSERT(std::allocator_traits<dense_allocator>::is_always_equal::value || get_dense_alloc() == other.get_dense_alloc(),
			           "Swapped allocators must be equal");

			swap_data(other);
			swap(m_max_load_factor, other.m_max_load_factor);
		}

	private:
		[[nodiscard]] constexpr auto to_capacity(size_type n) const noexcept { return static_cast<size_type>(static_cast<float>(n) * m_max_load_factor); }

		[[nodiscard]] constexpr auto &get_sparse_alloc() noexcept { return sparse_alloc_base::value(); }
		[[nodiscard]] constexpr auto &get_sparse_alloc() const noexcept { return sparse_alloc_base::value(); }
		[[nodiscard]] constexpr auto &get_dense_alloc() noexcept { return dense_alloc_base::value(); }
		[[nodiscard]] constexpr auto &get_dense_alloc() const noexcept { return dense_alloc_base::value(); }

		template<typename T>
		[[nodiscard]] TPP_CXX20_CONSTEXPR std::size_t hash(const T &k) const { return get_hash()(k); }
		template<typename T, typename U>
		[[nodiscard]] TPP_CXX20_CONSTEXPR bool cmp(const T &a, const U &b) const { return get_cmp()(a, b); }

		[[nodiscard]] constexpr auto *header_link() const noexcept
		{
			/* Using `const_cast` here to avoid non-const function duplicates. Pointers will be converted to appropriate const-ness either way. */
			return const_cast<bucket_link *>(static_cast<const bucket_link *>(this));
		}
		[[nodiscard]] constexpr auto *begin_node() const noexcept
		{
			if constexpr (is_ordered::value)
				return static_cast<bucket_node *>(header_link()->off(header_link()->next));
			else
				return m_dense;
		}
		[[nodiscard]] constexpr auto *front_node() const noexcept { return begin_node(); }
		[[nodiscard]] constexpr auto *back_node() const noexcept
		{
			if constexpr (is_ordered::value)
				return static_cast<bucket_node *>(header_link()->off(header_link()->prev));
			else
				return m_dense + (size() - 1);
		}
		[[nodiscard]] constexpr auto *end_node() const noexcept
		{
			if constexpr (is_ordered::value)
				return static_cast<bucket_node *>(header_link());
			else
				return m_dense + size();
		}

		[[nodiscard]] constexpr auto to_iter(bucket_node *node) noexcept { return iterator{node_iterator{node}}; }
		[[nodiscard]] constexpr auto to_iter(const bucket_node *node) const noexcept { return const_iterator{const_node_iterator{node}}; }

		template<std::size_t I>
		[[nodiscard]] constexpr auto *get_chain(std::size_t h) const noexcept
		{
			/* Same reason for `const_cast` as with `header_link` above. */
			return const_cast<size_type *>(m_sparse[h % bucket_count()].data() + I);
		}
		template<std::size_t I>
		[[nodiscard]]  constexpr auto *find_chain_ptr(size_type *bucket, size_type pos) const noexcept
		{
			while (*bucket != npos && *bucket != pos) bucket = &m_dense[*bucket].next[I];
			return bucket;
		}
		template<std::size_t I, typename T>
		[[nodiscard]] TPP_CXX20_CONSTEXPR std::pair<bucket_node *, size_type *> find_node(const T &key, std::size_t h) const
		{
			auto *idx = get_chain<I>(h);
			while (*idx != npos)
			{
				auto &entry = m_dense[*idx];
				if (entry.template hash<I>() == h && cmp(key, entry.template key<I>()))
					return {const_cast<bucket_node *>(&entry), idx};
				idx = const_cast<size_type *>(&entry.next[I]);
			}
			return {end_node(), idx};
		}

		template<typename... Args>
		TPP_CXX20_CONSTEXPR auto push_node(Args &&...args) -> std::pair<size_type, bucket_node *>
		{
			const auto pos = m_dense_size++;
			resize_buffer(get_dense_alloc(), m_dense, m_dense_capacity, m_dense_size, relocate_node{});

			auto alloc = allocator_type{get_dense_alloc()};
			m_dense[pos].construct(alloc, std::forward<Args>(args)...);
			return {pos, m_dense + pos};
		}
		TPP_CXX20_CONSTEXPR void pop_node()
		{
			auto alloc = allocator_type{get_dense_alloc()};
			std::allocator_traits<allocator_type>::destroy(alloc, m_dense + (--m_dense_size));
		}

		template<std::size_t I>
		constexpr void insert_node(bucket_node &node, size_type pos) noexcept
		{
			auto *chain_idx = get_chain<I>(node.template hash<I>());
			node.next[I] = *chain_idx;
			*chain_idx = pos;
		}
		template<std::size_t I>
		constexpr void move_chain(size_type from, size_type to) noexcept
		{
			auto &target = *find_chain_ptr<I>(get_chain<I>(m_dense[from].template hash<I>()), from);
			TPP_ASSERT(target != npos, "Cannot move to an empty node");
			target = to;
		}

		template<std::size_t... Is>
		TPP_CXX20_CONSTEXPR iterator commit_node(const_node_iterator hint, size_type pos, chain_slice slice, bucket_hash hashes, bucket_node *node)
		{
			/* Create the bucket and insertion order links. */
			if constexpr (is_ordered::value) node->link(hint.link ? const_cast<bucket_link *>(hint.link) : back_node());
			((*slice[Is] = pos), ...); /* Node is always inserted at the end of the chain. */

			/* Initialize the hash & rehash the table. Doing it now so
			 * that any temporary nodes do not affect table hashing. */
			((node->template hash<Is>() = hashes[Is]), ...);
			maybe_rehash();

			return to_iter(node);
		}
		template<std::size_t... Is, typename... Args>
		TPP_CXX20_CONSTEXPR iterator emplace_node(const_node_iterator hint, chain_slice slice, bucket_hash hashes, Args &&...args)
		{
			const auto [pos, node] = push_node(std::forward<Args>(args)...);
			return commit_node<Is...>(hint, pos, slice, hashes, node);
		}
		template<std::size_t... Is>
		TPP_CXX20_CONSTEXPR iterator erase_node(size_type pos, bucket_node *node, chain_slice slice)
		{
			TPP_ASSERT(node != end_node(), "Erased node cannot be the end node");
			auto *next = node;

			/* If the node links are ordered, unlink from the insertion order chain as well. */
			if constexpr (is_ordered::value)
			{
				auto *link = static_cast<bucket_link *>(node);
				next = static_cast<bucket_node *>(link->off(link->next));
				link->unlink();
			}
			((*slice[Is] = node->next[Is]), ...);

			/* Swap the entry with the last if necessary. */
			if (const auto end_pos = size() - 1; pos != end_pos)
			{
				(move_chain<Is>(end_pos, pos), ...);
				node->move_from(m_dense[end_pos]);
			}

			pop_node();
			return to_iter(next);
		}

		template<typename... Args, std::size_t... Is>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> emplace_impl(std::index_sequence<Is...>, const_node_iterator hint, Args &&...args)
		{
			/* Create a temporary object to check if it already exists within the table. */
			const auto [pos, tmp] = push_node(std::forward<Args>(args)...);
			const auto hs = bucket_hash{hash(tmp->template key<Is>())...};
			const auto node_list = std::array{find_node<Is>(tmp->template key<Is>(), hs[Is])...};
			for (auto [node, chain]: node_list)
				if (*chain != npos)
				{
					/* Found a conflict, return the existing node. */
					pop_node();
					return {to_iter(node), false};
				}

			/* No conflicts found, commit the node. */
			return {commit_node<Is...>(hint, pos, {node_list[Is].second...}, hs, tmp), true};
		}
		template<typename... Args>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> emplace_impl(const_node_iterator hint, Args &&...args)
		{
			return emplace_impl(std::make_index_sequence<key_size>{}, hint, std::forward<Args>(args)...);
		}

		template<typename... Ks, typename... Args, std::size_t... Is>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> insert_impl(std::index_sequence<Is...>, const_node_iterator hint, std::tuple<Ks...> ks, Args &&...args)
		{
			/* If a candidate was found, do nothing. Otherwise, emplace a new entry. */
			const auto hs = bucket_hash{hash(std::get<Is>(ks))...};
			const auto node_list = std::array{find_node<Is>(std::get<Is>(ks), hs[Is])...};
			for (auto [node, chain]: node_list)
				if (*chain != npos)
				{
					/* Found a conflict, return the existing node. */
					return {to_iter(node), false};
				}

			/* No conflicts found, proceed with insertion. */
			return {emplace_node<Is...>(hint, {node_list[Is].second...}, hs, std::forward<Args>(args)...), true};
		}
		template<typename... Ks, typename... Args>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> insert_impl(const_node_iterator hint, std::tuple<Ks...> ks, Args &&...args)
		{
			return insert_impl(std::make_index_sequence<key_size>{}, hint, ks, std::forward<Args>(args)...);
		}

		/* NOTE: insert_or_assign is available only if there is a single key. Otherwise, we cannot assign conflicting entries. */
		template<typename T, typename... Args>
		TPP_CXX20_CONSTEXPR auto insert_or_assign_impl(const_node_iterator hint, T &&key, Args &&...args) -> std::pair<iterator, bool>
		{
			static_assert(key_size == 1, "insert_or_assign is only available for keys of size 1");

			/* If a candidate was found, replace the entry. Otherwise, emplace a new entry. */
			const auto h = hash(key);
			if (const auto [candidate, chain_idx] = find_node<0>(key, h); *chain_idx == npos)
				return {emplace_node<0>(hint, {chain_idx}, {h},
				                        std::piecewise_construct,
				                        std::forward_as_tuple(key),
				                        std::forward_as_tuple(std::forward<Args>(args)...)),
				        true};
			else
			{
				replace_value(*candidate, std::forward<Args>(args)...);
				return {to_iter(candidate), false};
			}
		}

		template<std::size_t... Is>
		TPP_CXX20_CONSTEXPR iterator erase_impl(std::index_sequence<Is...>, size_type pos, bucket_node *node)
		{
			if (const auto end = end_node(); node == end)
				return to_iter(end);

			const auto slice = chain_slice{find_chain_ptr<Is...>(get_chain<Is>(node->template hash<Is>()), pos)...};
			return erase_node<Is...>(pos, node, slice);
		}
		TPP_CXX20_CONSTEXPR iterator erase_impl(size_type pos, bucket_node *node)
		{
			return erase_impl(std::make_index_sequence<key_size>{}, pos, node);
		}

		template<std::size_t I, typename T, std::size_t... Is>
		TPP_CXX20_CONSTEXPR iterator erase_impl(std::index_sequence<Is...>, const T &key, std::size_t h)
		{
			for (auto *chain_idx = get_chain<I>(h); *chain_idx != npos;)
			{
				const auto pos = *chain_idx;
				auto *node = m_dense + pos;
				if (node->template hash<I>() == h && cmp(key, node->template key<I>()))
				{
					/* Grab other bucket indices that point to `pos`. */
					chain_slice slice = {};
					((slice[Is] = find_chain_ptr<Is...>(get_chain<Is>(node->template hash<Is>()), pos)), ...);
					slice[I] = chain_idx;
					return erase_node<I, Is...>(pos, node, slice);
				}
				chain_idx = &node->next[I];
			}
			return end();
		}
		template<std::size_t I, typename T>
		TPP_CXX20_CONSTEXPR iterator erase_impl(const T &key, std::size_t h)
		{
			return erase_impl<I>(remove_index_t<I, std::make_index_sequence<key_size>>{}, key, h);
		}

		TPP_CXX20_CONSTEXPR void maybe_rehash() { if (load_factor() > m_max_load_factor) rehash(bucket_count() * 2); }
		template<size_type... Is>
		TPP_CXX20_CONSTEXPR void rehash_impl(size_type new_cap) { rehash_impl(std::make_index_sequence<key_size>{}, new_cap); }
		template<size_type... Is>
		TPP_CXX20_CONSTEXPR void rehash_impl(std::index_sequence<Is...>, size_type new_cap)
		{
			/* Reallocate the sparse buffer. */
			realloc_buffer(get_sparse_alloc(), m_sparse, m_sparse_size, new_cap);
			std::fill_n(m_sparse, m_sparse_size, make_array<key_size>(npos));

			/* Go through each entry & re-insert it. */
			for (size_type i = 0; i < size(); ++i) (insert_node<Is>(m_dense[i], i), ...);
		}

		TPP_CXX20_CONSTEXPR void reserve_data(size_type new_cap)
		{
			if (new_cap > m_dense_capacity)
			{
				auto &alloc = get_dense_alloc();
				auto *new_dense = std::allocator_traits<dense_allocator>::allocate(alloc, new_cap);
				relocate(alloc, m_dense, m_dense + m_dense_size, alloc, new_dense, relocate_node{});
				std::allocator_traits<dense_allocator>::deallocate(alloc, m_dense, m_dense_capacity);
				m_dense_capacity = new_cap;
				m_dense = new_dense;
			}
		}

		template<std::size_t... Is>
		TPP_CXX20_CONSTEXPR void copy_data(std::index_sequence<Is...>, const dense_table &other)
		{
			/* Expect that there is no data in the buffers, but the buffers might still exist. */
			TPP_ASSERT(size() == 0, "Table must be empty prior to copying elements");

			/* (re)allocate the bucket & element buffers if needed. */
			realloc_buffer(get_sparse_alloc(), m_sparse, m_sparse_size, other.m_sparse_size);
			realloc_buffer(get_dense_alloc(), m_dense, m_dense_capacity, other.m_size);

			/* Copy & insert elements from the other table. */
			auto alloc = allocator_type{get_dense_alloc()};
			for (; m_dense_size < other.size(); ++m_dense_size)
			{
				auto &from = other.m_dense[m_dense_size];
				auto &to = m_dense[m_dense_size];
				to.construct(alloc, from);
				(insert_node<Is>(to, m_dense_size), ...);
			}
		}
		template<std::size_t... Is>
		TPP_CXX20_CONSTEXPR void move_data(std::index_sequence<Is...>, dense_table &other)
		{
			/* Expect that there is no data in the buffers, but the buffers might still exist. */
			TPP_ASSERT(size() == 0, "Table must be empty prior to moving elements");

			/* (re)allocate the bucket & element buffers if needed. */
			realloc_buffer(get_sparse_alloc(), m_sparse, m_sparse_size, other.m_sparse_size);
			realloc_buffer(get_dense_alloc(), m_dense, m_dense_capacity, other.m_dense_size);

			/* Move & insert elements from the other table. */
			auto alloc = allocator_type{get_dense_alloc()};
			for (; m_dense_size < other.size(); ++m_dense_size)
			{
				auto &from = other.m_dense[m_dense_size];
				auto &to = m_dense[m_dense_size];
				relocate_node{}(alloc, &from, alloc, &to);
				(insert_node<Is>(to, m_dense_size), ...);
			}
			other.m_dense_size = 0;
		}

		TPP_CXX20_CONSTEXPR void init(size_type capacity)
		{
			m_dense = std::allocator_traits<dense_allocator>::allocate(get_dense_alloc(), m_dense_capacity = capacity);
			m_sparse = std::allocator_traits<sparse_allocator>::allocate(get_sparse_alloc(), m_sparse_size = capacity);
			std::fill_n(m_sparse, m_sparse_size, make_array<key_size, size_type>(npos));
		}
		TPP_CXX20_CONSTEXPR void copy_data(const dense_table &other) { copy_data(std::make_index_sequence<key_size>{}, other); }
		TPP_CXX20_CONSTEXPR void move_data(dense_table &other) { move_data(std::make_index_sequence<key_size>{}, other); }
		TPP_CXX20_CONSTEXPR void swap_data(dense_table &other)
		{
			std::swap(m_dense_size, other.m_dense_size);
			std::swap(m_dense_capacity, other.m_dense_capacity);
			std::swap(m_sparse_size, other.m_sparse_size);
			std::swap(m_sparse, other.m_sparse);
			std::swap(m_dense, other.m_dense);
		}
		TPP_CXX20_CONSTEXPR void clear_data()
		{
			auto alloc = allocator_type{get_dense_alloc()};
			for (size_type i = 0, n = std::exchange(m_dense_size, 0); i < n; ++i) m_dense[i].destroy(alloc);
		}

		TPP_CXX20_CONSTEXPR void move_from(dense_table &other)
		{
			if ((std::allocator_traits<sparse_allocator>::is_always_equal::value || get_sparse_alloc() == other.get_sparse_alloc()) &&
			    (std::allocator_traits<dense_allocator>::is_always_equal::value || get_dense_alloc() == other.get_dense_alloc()))
				swap_data(other);
			else
				move_data(other);
		}

		size_type m_dense_size = 0;     /* Total amount of elements in the dense table. */
		size_type m_dense_capacity = 0; /* Capacity of the dense buffer. */
		size_type m_sparse_size = 0;   /* Total amount of bucket chains in the bucket buffer. */

		bucket_pos *m_sparse = nullptr;
		bucket_node *m_dense = nullptr;

		float m_max_load_factor = initial_load_factor;
	};
}