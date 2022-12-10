/*
 * Created by switchblade on 11/7/22.
 */

#pragma once

#include "table_util.hpp"

#ifndef TPP_USE_IMPORT

#include <vector>
#include <limits>
#include <tuple>

#endif

namespace tpp::detail
{
	template<typename V, typename K, typename KHash, typename KCmp, typename Alloc, typename ValueTraits>
	class dense_table : ValueTraits::link_type, ebo_container<KHash>, ebo_container<KCmp>
	{
		using traits_t = table_traits<V, V, K, KHash, KCmp, Alloc>;

	public:
		using insert_type = typename traits_t::insert_type;
		using value_type = typename traits_t::value_type;
		using key_type = typename traits_t::key_type;

		using hasher = typename traits_t::hasher;
		using key_equal = typename traits_t::key_equal;
		using allocator_type = typename traits_t::allocator_type;

		using size_type = typename traits_t::size_type;
		using difference_type = typename traits_t::difference_type;

		using is_transparent = std::conjunction<detail::is_transparent<hasher>, detail::is_transparent<key_equal>>;
		using is_ordered = detail::is_ordered<typename ValueTraits::link_type>;

		constexpr static size_type initial_capacity = 8;
		constexpr static size_type npos = std::numeric_limits<size_type>::max();

		constexpr static float initial_load_factor = .875f;

	private:
		using bucket_link = typename ValueTraits::link_type;
		struct bucket_node : table_node<V, ValueTraits>
		{
			using node_base = table_node<V, ValueTraits>;

			constexpr bucket_node() noexcept(std::is_nothrow_default_constructible_v<node_base>) = default;
			constexpr bucket_node(const bucket_node &other) : node_base(other), next(other.next) {}
			constexpr bucket_node(bucket_node &&other) noexcept : node_base(std::move(other)), next(std::exchange(other.next, npos)) {}

			constexpr bucket_node &operator=(const bucket_node &other)
			{
				node_base::operator=(other);
				next = other.next;
				return *this;
			}
			constexpr bucket_node &operator=(bucket_node &&other) noexcept
			{
				node_base::operator=(std::move(other));
				std::swap(next, other.next);
				return *this;
			}

			template<typename... Args>
			constexpr bucket_node(Args &&...args) noexcept(std::is_nothrow_constructible_v<node_base, Args...>) : node_base(std::forward<Args>(args)...) {}

			size_type next = npos;
		};

		using dense_alloc_t = typename std::allocator_traits<Alloc>::template rebind_alloc<bucket_node>;
		using dense_t = std::vector<bucket_node, dense_alloc_t>;

		using sparse_alloc_t = typename std::allocator_traits<Alloc>::template rebind_alloc<size_type>;
		using sparse_t = std::vector<size_type, sparse_alloc_t>;

		using hash_base = ebo_container<hasher>;
		using cmp_base = ebo_container<key_equal>;
		using header_base = bucket_link;

		using node_iterator = std::conditional_t<is_ordered::value, ordered_iterator<bucket_node, traits_t>, bucket_node *>;
		using const_node_iterator = std::conditional_t<is_ordered::value, ordered_iterator<const bucket_node, traits_t>, const bucket_node *>;

		template<typename N>
		class bucket_iterator
		{
			// @formatter:off
			template<typename>
			friend class bucket_iterator;
			// @formatter:on

		public:
			using value_type = V;
			using pointer = std::conditional_t<std::is_const_v<N>, typename ValueTraits::const_pointer, typename ValueTraits::pointer>;
			using reference = std::conditional_t<std::is_const_v<N>, typename ValueTraits::const_reference, typename ValueTraits::reference>;

			using size_type = typename bucket_node::size_type;
			using difference_type = typename bucket_node::difference_type;
			using iterator_category = std::forward_iterator_tag;

		public:
			constexpr bucket_iterator() noexcept = default;
			template<typename U, typename = std::enable_if_t<!std::is_same_v<N, U> && std::is_constructible_v<U *, std::add_const_t<N> *>>>
			constexpr bucket_iterator(const bucket_iterator<U> &other) noexcept : m_base(other.m_base), m_off(other.m_off) {}

			constexpr explicit bucket_iterator(N *base, size_type off) noexcept : m_base(base), m_off(off) {}

			constexpr bucket_iterator operator++(int) noexcept
			{
				auto tmp = *this;
				operator++();
				return tmp;
			}
			constexpr bucket_iterator &operator++() noexcept
			{
				m_off = node()->next;
				return *this;
			}

			[[nodiscard]] constexpr pointer operator->() const noexcept { return pointer{node()->get()}; }
			[[nodiscard]] constexpr reference operator*() const noexcept { return *operator->(); }

			[[nodiscard]] constexpr bool operator==(const bucket_iterator &other) const noexcept { return m_off == other.m_off; }

#if __cplusplus >= 202002L
			[[nodiscard]] constexpr auto operator<=>(const bucket_iterator&other) const noexcept { return m_off <=> other.m_off; }
#else
			[[nodiscard]] constexpr bool operator!=(const bucket_iterator &other) const noexcept { return m_off != other.m_off; }
			[[nodiscard]] constexpr bool operator<=(const bucket_iterator &other) const noexcept { return m_off <= other.m_off; }
			[[nodiscard]] constexpr bool operator>=(const bucket_iterator &other) const noexcept { return m_off >= other.m_off; }
			[[nodiscard]] constexpr bool operator<(const bucket_iterator &other) const noexcept { return m_off < other.m_off; }
			[[nodiscard]] constexpr bool operator>(const bucket_iterator &other) const noexcept { return m_off > other.m_off; }
#endif

		private:
			[[nodiscard]] constexpr N *node() const noexcept { return m_base + m_off; }

			N *m_base = nullptr;
			size_type m_off = 0;
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
		constexpr static void replace_value(bucket_node &node, T &&value)
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
		constexpr static void replace_value(bucket_node &node, Args &&...args)
		{
			auto &mapped = node.mapped();
			using mapped_t = std::remove_reference_t<decltype(mapped)>;

			if constexpr (!std::is_trivially_destructible_v<mapped_t>)
				mapped.~mapped_t();

			static_assert(std::is_constructible_v<mapped_t, Args...>);
			new(&mapped) mapped_t(std::forward<Args>(args)...);
		}

	public:
		constexpr dense_table()
		{
			m_sparse.resize(initial_capacity, npos);
			m_dense.reserve(to_capacity(initial_capacity));
		}

		constexpr dense_table(const dense_table &other)
				: header_base(other), hash_base(other), cmp_base(other),
				  m_sparse(other.m_sparse),
				  m_dense(other.m_dense),
				  m_max_load_factor(other.m_max_load_factor) {}
		constexpr dense_table(const dense_table &other, const allocator_type &alloc)
				: header_base(other), hash_base(other), cmp_base(other),
				  m_sparse(other.m_sparse, sparse_alloc_t{alloc}),
				  m_dense(other.m_dense, dense_alloc_t{alloc}),
				  m_max_load_factor(other.m_max_load_factor) {}

		constexpr dense_table(dense_table &&other)
		noexcept(std::is_nothrow_move_constructible_v<sparse_t> &&
		         std::is_nothrow_move_constructible_v<dense_t> &&
		         std::is_nothrow_move_constructible_v<hasher> &&
		         std::is_nothrow_move_constructible_v<key_equal>)
				: header_base(std::move(other)), hash_base(std::move(other)), cmp_base(std::move(other)),
				  m_sparse(std::move(other.m_sparse)),
				  m_dense(std::move(other.m_dense)),
				  m_max_load_factor(other.m_max_load_factor) {}
		constexpr dense_table(dense_table &&other, const allocator_type &alloc)
		noexcept(std::is_nothrow_constructible_v<sparse_t, sparse_t &&, sparse_alloc_t> &&
		         std::is_nothrow_constructible_v<dense_t, dense_t &&, dense_alloc_t> &&
		         std::is_nothrow_move_constructible_v<hasher> &&
		         std::is_nothrow_move_constructible_v<key_equal>)
				: header_base(std::move(other)), hash_base(std::move(other)), cmp_base(std::move(other)),
				  m_sparse(std::move(other.m_sparse), sparse_alloc_t{alloc}),
				  m_dense(std::move(other.m_dense), dense_alloc_t{alloc}),
				  m_max_load_factor(other.m_max_load_factor) {}

		constexpr dense_table(const allocator_type &alloc)
				: m_sparse(sparse_alloc_t{alloc}),
				  m_dense(dense_alloc_t{alloc})
		{
			m_sparse.resize(initial_capacity, npos);
			m_dense.reserve(to_capacity(initial_capacity));
		}
		constexpr dense_table(size_type bucket_count, const hasher &hash, const key_equal &cmp, const allocator_type &alloc)
				: hash_base(hash), cmp_base(cmp),
				  m_sparse(sparse_alloc_t{alloc}),
				  m_dense(dense_alloc_t{alloc})
		{
			m_sparse.resize(bucket_count, npos);
			m_dense.reserve(to_capacity(bucket_count));
		}

		template<typename Iter>
		constexpr dense_table(Iter first, Iter last, size_type bucket_count, const hasher &hash, const key_equal &cmp, const allocator_type &alloc)
				: dense_table(bucket_count, hash, cmp, alloc) { insert(first, last); }

		constexpr dense_table &operator=(const dense_table &other)
		{
			if (this != &other)
			{
				header_base::operator=(other);
				hash_base::operator=(other);
				cmp_base::operator=(other);

				m_sparse = other.m_sparse;
				m_dense = other.m_dense;
				m_max_load_factor = other.m_max_load_factor;
			}
			return *this;
		}
		constexpr dense_table &operator=(dense_table &&other)
		noexcept(std::is_nothrow_move_assignable_v<hasher> &&
		         std::is_nothrow_move_assignable_v<key_equal> &&
		         std::is_nothrow_move_assignable_v<sparse_t> &&
		         std::is_nothrow_move_assignable_v<dense_t>)
		{
			if (this != &other)
			{
				header_base::operator=(std::move(other));
				hash_base::operator=(std::move(other));
				cmp_base::operator=(std::move(other));

				m_sparse = std::move(other.m_sparse);
				m_dense = std::move(other.m_dense);
				m_max_load_factor = other.m_max_load_factor;
			}
			return *this;
		}

		template<typename Iter>
		constexpr void assign(Iter first, Iter last)
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

		[[nodiscard]] constexpr size_type size() const noexcept { return m_dense.size(); }
		[[nodiscard]] constexpr size_type capacity() const noexcept { return to_capacity(bucket_count()); }
		[[nodiscard]] constexpr size_type max_size() const noexcept { return to_capacity(std::min(m_dense.max_size(), npos - 1)); }
		[[nodiscard]] constexpr float load_factor() const noexcept { return static_cast<float>(size()) / static_cast<float>(bucket_count()); }

		[[nodiscard]] constexpr local_iterator begin(size_type n) noexcept { return local_iterator{m_dense.data(), m_sparse[n]}; }
		[[nodiscard]] constexpr const_local_iterator begin(size_type n) const noexcept { return const_local_iterator{m_dense.data(), m_sparse[n]}; }
		[[nodiscard]] constexpr local_iterator end(size_type) noexcept { return local_iterator{}; }
		[[nodiscard]] constexpr const_local_iterator end(size_type) const noexcept { return const_local_iterator{}; }

		[[nodiscard]] constexpr size_type bucket_count() const noexcept { return m_sparse.size(); }
		[[nodiscard]] constexpr size_type max_bucket_count() const noexcept { return m_sparse.max_size(); }
		[[nodiscard]] constexpr size_type bucket_size(size_type n) const noexcept { return static_cast<size_type>(std::distance(begin(n), end(n))); }
		template<typename T>
		[[nodiscard]] constexpr size_type bucket(const T &key) const { return hash(key) % bucket_count(); }

		constexpr void clear()
		{
			/* Reset header link. */
			if constexpr (is_ordered::value) *header_link() = bucket_link{};

			std::fill_n(m_sparse.data(), bucket_count(), npos);
			m_dense.clear();
		}
		constexpr void reserve(size_type n)
		{
			[[maybe_unused]] const auto front_pos = front_node() - m_dense.data();
			[[maybe_unused]] const auto back_pos = back_node() - m_dense.data();

			m_dense.reserve(n);
			rehash(static_cast<size_type>(static_cast<float>(n) / m_max_load_factor));
		}

		template<typename T>
		[[nodiscard]] constexpr bool contains(const T &key) const { return (*find_node(hash(key), key).second) != npos; }

		template<typename T>
		[[nodiscard]] constexpr iterator find(const T &key) { return to_iter(find_node(hash(key), key).first); }
		template<typename T>
		[[nodiscard]] constexpr const_iterator find(const T &key) const { return to_iter(find_node(hash(key), key).first); }

		constexpr std::pair<iterator, bool> insert(const insert_type &value) { return insert_impl({}, ValueTraits::key_get(value), value); }
		constexpr std::pair<iterator, bool> insert(insert_type &&value) { return insert_impl({}, ValueTraits::key_get(value), std::move(value)); }
		constexpr iterator insert(const_iterator hint, const insert_type &value)
		{
			return insert_impl(to_underlying(hint), ValueTraits::key_get(value), value).first;
		}
		constexpr iterator insert(const_iterator hint, insert_type &&value)
		{
			return insert_impl(to_underlying(hint), ValueTraits::key_get(value), std::move(value)).first;
		}

		template<typename T, typename = std::enable_if_t<!(std::is_convertible_v<T &&, insert_type &&> || std::is_convertible_v<T &&, value_type &&>)>>
		constexpr std::pair<iterator, bool> insert(T &&value) TPP_REQUIRES((std::is_constructible_v<V, T>))
		{
			return emplace_impl({}, std::forward<T>(value));
		}
		template<typename T, typename = std::enable_if_t<!(std::is_convertible_v<T &&, insert_type &&> || std::is_convertible_v<T &&, value_type &&>)>>
		constexpr iterator insert(const_iterator hint, T &&value) TPP_REQUIRES((std::is_constructible_v<V, T>))
		{
			return emplace_impl(to_underlying(hint), std::forward<T>(value)).first;
		}

		template<typename Iter>
		constexpr void insert(Iter first, Iter last)
		{
			if constexpr (std::is_base_of_v<std::random_access_iterator_tag, typename std::iterator_traits<Iter>::iterator_category>)
				reserve(static_cast<size_type>(std::distance(first, last)));
			for (; first != last; ++first) insert(*first);
		}

		template<typename T, typename U>
		constexpr std::pair<iterator, bool> insert_or_assign(const T &key, U &&value) TPP_REQUIRES((std::is_constructible_v<V, T, U>))
		{
			return insert_or_assign_impl({}, key, std::forward<U>(value));
		}
		template<typename T, typename U>
		constexpr iterator insert_or_assign(const_iterator hint, const T &key, U &&value) TPP_REQUIRES((std::is_constructible_v<V, T, U>))
		{
			return insert_or_assign_impl(to_underlying(hint), key, std::forward<U>(value)).first;
		}

		template<typename... Args>
		constexpr std::pair<iterator, bool> emplace(Args &&...args) TPP_REQUIRES((std::is_constructible_v<V, Args...>))
		{
			return emplace_impl({}, std::forward<Args>(args)...);
		}
		template<typename... Args>
		constexpr iterator emplace_hint(const_iterator hint, Args &&...args) TPP_REQUIRES((std::is_constructible_v<V, Args...>))
		{
			return emplace_impl(to_underlying(hint), std::forward<Args>(args)...);
		}

		template<typename U, typename... Args>
		constexpr std::pair<iterator, bool> emplace_or_replace(U &&key, Args &&...args) TPP_REQUIRES((std::is_constructible_v<V, Args...>))
		{
			return insert_or_assign_impl({}, key, std::forward<Args>(args)...);
		}
		template<typename U, typename... Args>
		constexpr iterator emplace_or_replace(const_iterator hint, U &&key, Args &&...args) TPP_REQUIRES((std::is_constructible_v<V, Args...>))
		{
			return insert_or_assign_impl(to_underlying(hint), key, std::forward<Args>(args)...);
		}

		template<typename U, typename... Args>
		constexpr std::pair<iterator, bool> try_emplace(U &&key, Args &&...args)
		TPP_REQUIRES((std::is_constructible_v<V, std::piecewise_construct_t, std::tuple<U &&>, std::tuple<Args && ...>>))
		{
			return insert_impl({}, key, std::piecewise_construct,
			                   std::forward_as_tuple(std::forward<U>(key)),
			                   std::forward_as_tuple(std::forward<Args>(args)...));
		}
		template<typename U, typename... Args>
		constexpr iterator try_emplace(const_iterator hint, U &&key, Args &&...args)
		TPP_REQUIRES((std::is_constructible_v<V, std::piecewise_construct_t, std::tuple<U &&>, std::tuple<Args && ...>>))
		{
			return insert_impl(to_underlying(hint), key, std::piecewise_construct,
			                   std::forward_as_tuple(std::forward<U>(key)),
			                   std::forward_as_tuple(std::forward<Args>(args)...)).first;
		}

		template<typename T, typename = std::enable_if_t<!std::is_convertible_v<T, const_iterator>>>
		constexpr iterator erase(const T &key) { return erase_impl(hash(key), key); }
		constexpr iterator erase(const_iterator where)
		{
			const auto &node = *to_underlying(where);
			return erase_impl(node.hash, node.key());
		}
		constexpr iterator erase(const_iterator first, const_iterator last)
		{
			iterator result = end();
			while (last != first) result = erase(--last);
			return result;
		}

		constexpr void rehash(size_type new_cap)
		{
			using std::max;

			/* Adjust the capacity to be at least large enough to fit the current size. */
			new_cap = max(max(static_cast<size_type>(static_cast<float>(size()) / m_max_load_factor), new_cap), initial_capacity);
			if (new_cap != m_sparse.capacity()) rehash_impl(new_cap);
		}

		[[nodiscard]] constexpr float max_load_factor() const noexcept { return m_max_load_factor; }
		constexpr void max_load_factor(float f) noexcept { m_max_load_factor = f; }

		[[nodiscard]] constexpr auto get_allocator() const { return m_dense.get_allocator(); }
		[[nodiscard]] constexpr auto &get_hash() const noexcept { return hash_base::value(); }
		[[nodiscard]] constexpr auto &get_cmp() const noexcept { return cmp_base::value(); }

		constexpr void swap(dense_table &other)
		noexcept(std::is_nothrow_swappable_v<hasher> &&
		         std::is_nothrow_swappable_v<key_equal> &&
		         std::is_nothrow_swappable_v<sparse_t> &&
		         std::is_nothrow_swappable_v<dense_t>)
		{
			header_base::swap(other);
			hash_base::swap(other);
			cmp_base::swap(other);

			std::swap(m_sparse, other.m_sparse);
			std::swap(m_dense, other.m_dense);
			std::swap(m_max_load_factor, other.m_max_load_factor);
		}

	private:
		[[nodiscard]] constexpr auto to_capacity(size_type n) const noexcept { return static_cast<size_type>(static_cast<float>(n) * m_max_load_factor); }

		template<typename T>
		[[nodiscard]] constexpr std::size_t hash(const T &k) const { return get_hash()(k); }
		template<typename T, typename U>
		[[nodiscard]] constexpr bool cmp(const T &a, const U &b) const { return get_cmp()(a, b); }

		/* Using `const_cast` here to avoid non-const function duplicates. Pointers will be converted to appropriate const-ness either way. */
		[[nodiscard]] constexpr auto *header_link() const noexcept { return const_cast<bucket_link *>(static_cast<const bucket_link *>(this)); }
		[[nodiscard]] constexpr auto *begin_node() const noexcept
		{
			if constexpr (is_ordered::value)
				return static_cast<bucket_node *>(header_link()->off(header_link()->next));
			else
				return const_cast<bucket_node *>(m_dense.data());
		}
		[[nodiscard]] constexpr auto *front_node() const noexcept { return begin_node(); }
		[[nodiscard]] constexpr auto *back_node() const noexcept
		{
			if constexpr (is_ordered::value)
				return static_cast<bucket_node *>(header_link()->off(header_link()->prev));
			else
				return const_cast<bucket_node *>(m_dense.data()) + (size() - 1);
		}
		[[nodiscard]] constexpr auto *end_node() const noexcept
		{
			if constexpr (is_ordered::value)
				return static_cast<bucket_node *>(header_link());
			else
				return const_cast<bucket_node *>(m_dense.data()) + size();
		}

		[[nodiscard]] constexpr auto to_iter(bucket_node *node) noexcept { return iterator{node_iterator{node}}; }
		[[nodiscard]] constexpr auto to_iter(const bucket_node *node) const noexcept { return const_iterator{const_node_iterator{node}}; }

		/* Same reason for `const_cast` as with node getters above. */
		[[nodiscard]] constexpr auto *get_chain(std::size_t h) const noexcept { return const_cast<size_type *>(m_sparse.data()) + (h % bucket_count()); }
		template<typename T>
		[[nodiscard]] constexpr auto find_node(std::size_t h, const T &key) const -> std::pair<bucket_node *, size_type *>
		{
			auto *idx = get_chain(h);
			while (*idx != npos)
			{
				auto &entry = m_dense[*idx];
				if (entry.hash == h && cmp(key, entry.key()))
					return {const_cast<bucket_node *>(&entry), idx};
				idx = const_cast<size_type *>(&entry.next);
			}
			return {end_node(), idx};
		}

		constexpr iterator commit_node([[maybe_unused]] const_node_iterator hint, size_type pos, size_type *chain_idx, std::size_t h, bucket_node *node)
		{
			/* Create the bucket and insertion order links. */
			node->link(hint.link ? const_cast<bucket_link *>(hint.link) : back_node());
			*chain_idx = pos;

			/* Initialize the hash & rehash the table. Doing it now so
			 * that any temporary nodes do not affect table hashing. */
			node->hash = h;
			maybe_rehash();

			return to_iter(node);
		}

		template<typename... Args>
		constexpr iterator emplace_at(const_node_iterator hint, size_type *chain_idx, std::size_t h, Args &&...args)
		{
			const auto pos = size();
			auto &node = m_dense.emplace_back(std::forward<Args>(args)...);
			return commit_node(hint, pos, chain_idx, h, &node);
		}

		template<typename... Args>
		constexpr auto emplace_impl(const_node_iterator hint, Args &&...args) -> std::pair<iterator, bool>
		{
			/* Create a temporary object to check if it already exists within the table. */
			const auto pos = size();
			auto &tmp = m_dense.emplace_back(std::forward<Args>(args)...);
			const auto h = hash(tmp.key());

			/* If there is no conflict, commit the temporary to the table. Otherwise, destroy the temporary. */
			if (const auto [candidate, chain_idx] = find_node(h, tmp.key()); *chain_idx == npos)
				return {commit_node(hint, pos, chain_idx, h, &tmp), true};
			else
			{
				m_dense.pop_back();
				return {to_iter(candidate), false};
			}
		}
		template<typename T, typename... Args>
		constexpr auto insert_impl(const_node_iterator hint, const T &key, Args &&...args) -> std::pair<iterator, bool>
		{
			/* If a candidate was found, do nothing. Otherwise, emplace a new entry. */
			const auto h = hash(key);
			if (const auto [candidate, chain_idx] = find_node(h, key); *chain_idx == npos)
				return {emplace_at(hint, chain_idx, h, std::forward<Args>(args)...), true};
			else
				return {to_iter(candidate), false};
		}
		template<typename T, typename... Args>
		constexpr auto insert_or_assign_impl(const_node_iterator hint, T &&key, Args &&...args) -> std::pair<iterator, bool>
		{
			/* If a candidate was found, replace the entry. Otherwise, emplace a new entry. */
			const auto h = hash(key);
			if (const auto [candidate, chain_idx] = find_node(h, key); *chain_idx == npos)
				return {emplace_at(hint, chain_idx, h, std::forward<T>(key), std::forward<Args>(args)...), true};
			else
			{
				replace_value(*candidate, std::forward<Args>(args)...);
				return {to_iter(candidate), false};
			}
		}

		template<typename T>
		constexpr iterator erase_impl(std::size_t h, const T &key)
		{
			for (auto *chain_idx = get_chain(h); *chain_idx != npos;)
			{
				const auto pos = *chain_idx;
				auto *entry_ptr = m_dense.data() + static_cast<difference_type>(pos);
				if (entry_ptr->hash == h && cmp(key, entry_ptr->key()))
				{
					bucket_node *next = entry_ptr; /* Save the next pointer for returning. */

					/* Unlink the entry from both bucket & insertion chains. */
					if constexpr (is_ordered::value)
					{
						auto *entry_link = static_cast<bucket_link *>(entry_ptr);
						next = static_cast<bucket_node *>(entry_link->off(entry_link->next));
						entry_link->unlink();
					}
					*chain_idx = entry_ptr->next;

					/* Swap the entry with the last if necessary & erase the last entry. */
					if (const auto end_pos = size() - 1; pos != end_pos)
					{
						if constexpr (std::is_move_assignable_v<bucket_node>)
							*entry_ptr = std::move(m_dense.back());
						else
						{
							entry_ptr->~bucket_node();
							new(entry_ptr) bucket_node(std::move(m_dense.back()));
						}

						/* Find the chain offset pointing to the old position & replace it with the new position. */
						for (chain_idx = get_chain(entry_ptr->hash); *chain_idx != npos; chain_idx = &m_dense[*chain_idx].next)
							if (*chain_idx == end_pos)
							{
								*chain_idx = pos;
								break;
							}
					}
					m_dense.pop_back();

					return to_iter(next);
				}
				chain_idx = &entry_ptr->next;
			}
			return end();
		}

		constexpr void maybe_rehash()
		{
			if (load_factor() > m_max_load_factor)
				rehash(bucket_count() * 2);
		}
		constexpr void rehash_impl(size_type new_cap)
		{
			/* Clear & reserve the sparse vector with npos. */
			m_sparse.clear();
			m_sparse.resize(new_cap, npos);

			/* Go through each entry & re-insert it. */
			for (size_type i = 0; i < size(); ++i)
			{
				auto &entry = m_dense[i];
				auto *chain_idx = get_chain(entry.hash);
				entry.next = *chain_idx;
				*chain_idx = i;
			}
		}

		sparse_t m_sparse = sparse_t(initial_capacity, npos);
		dense_t m_dense;

		float m_max_load_factor = initial_load_factor;
	};
}