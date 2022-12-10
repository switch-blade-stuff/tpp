/*
 * Created by switchblade on 11/5/22.
 */

#pragma once

#include "detail/dense_table.hpp"

namespace tpp
{
	using detail::multikey;

	template<typename Mk, typename = detail::multiset_hash<Mk>, typename = detail::multiset_eq<Mk>, typename = detail::multiset_alloc<Mk>>
	class dense_multiset;

	/** @brief Hash multiset based on dense hash table.
	 *
	 * Multiset is a one-to-one container that enables mapping multiple independent keys to each other.
	 * Every entry of a multiset is a tuple, that can be searched for using either one of the set's keys.
	 *
	 * Internally, dense multiset stores it's elements in a contiguous unordered vector.
	 * Insert and erase operations on a dense multiset may invalidate references to it's
	 * elements due to the internal element vector being reordered.
	 *
	 * @tparam KeySeq Key types of the multiset.
	 * @tparam KeyHash Hash functor used by the multiset. The functor must be invocable for both key types.
	 * @tparam KeyCmp Compare functor used by the multiset. The functor must be invocable for both key types.
	 * @tparam Alloc Allocator used by the multiset. */
	template<typename... KeySeq, typename KeyHash, typename KeyCmp, typename Alloc>
	class dense_multiset<multikey<KeySeq...>, KeyHash, KeyCmp, Alloc> : detail::ebo_container<KeyHash>, detail::ebo_container<KeyCmp>
	{
	public:
		typedef std::tuple<KeySeq...> key_type;

	private:
		using traits_t = detail::table_traits<key_type, key_type, key_type, KeyHash, KeyCmp, Alloc>;

	public:
		typedef typename traits_t::insert_type insert_type;
		typedef typename traits_t::value_type value_type;

		typedef typename traits_t::hasher hasher;
		typedef typename traits_t::key_equal key_equal;
		typedef typename traits_t::allocator_type allocator_type;

		typedef typename traits_t::size_type size_type;
		typedef typename traits_t::difference_type difference_type;

		typedef value_type &reference;
		typedef const value_type &const_reference;
		typedef value_type *pointer;
		typedef const value_type *const_pointer;

		constexpr static auto key_count = sizeof...(KeySeq);

	private:
		using is_transparent = std::conjunction<detail::is_transparent<hasher>, detail::is_transparent<key_equal>>;

		using hash_base = detail::ebo_container<hasher>;
		using cmp_base = detail::ebo_container<key_equal>;

		using keys_index = std::make_index_sequence<key_count>;
		using hash_list = std::array<std::size_t, key_count>;
		using index_list = std::array<size_type, key_count>;

		constexpr static size_type initial_capacity = 8;
		constexpr static size_type npos = std::numeric_limits<size_type>::max();

		constexpr static float initial_load_factor = .875f;

		[[nodiscard]] constexpr static auto make_index_list(size_type i) noexcept { return make_index_list(i, keys_index{}); }
		template<std::size_t... Is>
		[[nodiscard]] constexpr static auto make_index_list(size_type i, std::index_sequence<Is...>) noexcept
		{
			return index_list{detail::forward_i<Is>(i)...};
		}

		struct bucket_node : value_type
		{
			template<typename... Args>
			constexpr bucket_node(Args &&...args) noexcept(std::is_nothrow_constructible_v<key_type, Args...>) : key_type(std::forward<Args>(args)...) {}

			[[nodiscard]] constexpr auto *get() noexcept { return static_cast<key_type *>(this); }
			[[nodiscard]] constexpr auto *get() const noexcept { return static_cast<const key_type *>(this); }

			template<std::size_t I>
			[[nodiscard]] constexpr auto &key() const noexcept { return std::get<I>(*get()); }

			hash_list hash = {};
			index_list next = make_index_list(npos);
		};

		using dense_alloc_t = typename std::allocator_traits<Alloc>::template rebind_alloc<bucket_node>;
		using dense_t = std::vector<bucket_node, dense_alloc_t>;

		using sparse_alloc_t = typename std::allocator_traits<Alloc>::template rebind_alloc<index_list>;
		using sparse_t = std::vector<index_list, sparse_alloc_t>;

		class multiset_iterator
		{
			friend class dense_multiset;

		public:
			typedef typename dense_multiset::value_type value_type;
			typedef typename dense_multiset::const_pointer pointer;
			typedef typename dense_multiset::const_reference reference;

			typedef typename dense_multiset::size_type size_type;
			typedef typename dense_multiset::difference_type difference_type;
			typedef std::random_access_iterator_tag iterator_category;

		private:
			constexpr multiset_iterator(bucket_node *node) noexcept : m_node(node) {}

		public:
			constexpr multiset_iterator() noexcept = default;

			constexpr multiset_iterator operator++(int) noexcept
			{
				auto tmp = *this;
				operator++();
				return tmp;
			}
			constexpr multiset_iterator &operator++() noexcept
			{
				++m_node;
				return *this;
			}
			constexpr multiset_iterator &operator+=(difference_type n) noexcept
			{
				m_node += n;
				return *this;
			}

			constexpr multiset_iterator operator--(int) noexcept
			{
				auto tmp = *this;
				operator--();
				return tmp;
			}
			constexpr multiset_iterator &operator--() noexcept
			{
				--m_node;
				return *this;
			}
			constexpr multiset_iterator &operator-=(difference_type n) noexcept
			{
				m_node -= n;
				return *this;
			}

			[[nodiscard]] constexpr multiset_iterator operator+(difference_type n) const noexcept { return multiset_iterator{m_node + n}; }
			[[nodiscard]] constexpr multiset_iterator operator-(difference_type n) const noexcept { return multiset_iterator{m_node - n}; }
			[[nodiscard]] constexpr difference_type operator-(const difference_type &other) const noexcept { return m_node - other.m_node; }

			[[nodiscard]] constexpr pointer operator->() const noexcept { return m_node->get(); }
			[[nodiscard]] constexpr reference operator*() const noexcept { return *operator->(); }
			[[nodiscard]] constexpr reference operator[](difference_type i) const noexcept { return *operator+(i); }

			[[nodiscard]] constexpr bool operator==(const multiset_iterator &other) const noexcept { return m_node == other.m_node; }

#if __cplusplus >= 202002L
			[[nodiscard]] constexpr auto operator<=>(const multiset_iterator &other) const noexcept { return m_node <=> other.m_node; }
#else
			[[nodiscard]] constexpr bool operator!=(const multiset_iterator &other) const noexcept { return m_node != other.m_node; }
			[[nodiscard]] constexpr bool operator<=(const multiset_iterator &other) const noexcept { return m_node <= other.m_node; }
			[[nodiscard]] constexpr bool operator>=(const multiset_iterator &other) const noexcept { return m_node >= other.m_node; }
			[[nodiscard]] constexpr bool operator<(const multiset_iterator &other) const noexcept { return m_node < other.m_node; }
			[[nodiscard]] constexpr bool operator>(const multiset_iterator &other) const noexcept { return m_node > other.m_node; }
#endif
		private:
			bucket_node *m_node = nullptr;
		};

	public:
		typedef multiset_iterator iterator;
		typedef multiset_iterator const_iterator;
		typedef std::reverse_iterator<multiset_iterator> reverse_iterator;
		typedef std::reverse_iterator<multiset_iterator> const_reverse_iterator;

	private:
		[[nodiscard]] constexpr static multiset_iterator to_iter(bucket_node *node) noexcept { return multiset_iterator{node}; }

	public:
		/** Initializes the multiset with default capacity. */
		constexpr dense_multiset() = default;
		/** Initializes the multiset with default capacity using the specified allocator. */
		constexpr explicit dense_multiset(const allocator_type &alloc) : m_sparse(sparse_alloc_t{alloc}), m_dense(dense_alloc_t{alloc})
		{
			m_sparse.resize(initial_capacity, npos);
			m_dense.reserve(to_capacity(initial_capacity));
		}

		/** Copy-constructs the multiset. */
		constexpr dense_multiset(const dense_multiset &other)
				: hash_base(other), cmp_base(other),
				  m_sparse(other.m_sparse),
				  m_dense(other.m_dense),
				  m_max_load_factor(other.m_max_load_factor) {}
		/** Copy-constructs the multiset using the specified allocator. */
		constexpr dense_multiset(const dense_multiset &other, const allocator_type &alloc)
				: hash_base(other), cmp_base(other),
				  m_sparse(other.m_sparse, sparse_alloc_t{alloc}),
				  m_dense(other.m_dense, dense_alloc_t{alloc}),
				  m_max_load_factor(other.m_max_load_factor) {}

		/** Move-constructs the multiset. */
		constexpr dense_multiset(dense_multiset &&other)
		noexcept(std::is_nothrow_move_constructible_v<sparse_t &&> &&
		         std::is_nothrow_move_constructible_v<dense_t> &&
		         std::is_nothrow_move_constructible_v<hasher &&> &&
		         std::is_nothrow_move_constructible_v<key_equal>)
				: hash_base(std::move(other)), cmp_base(std::move(other)),
				  m_sparse(std::move(other.m_sparse)),
				  m_dense(std::move(other.m_dense)),
				  m_max_load_factor(other.m_max_load_factor) {}
		/** Move-constructs the multiset using the specified allocator. */
		constexpr dense_multiset(dense_multiset &&other, const allocator_type &alloc)
		noexcept(std::is_nothrow_constructible_v<sparse_t, sparse_t &&, sparse_alloc_t> &&
		         std::is_nothrow_constructible_v<dense_t, dense_t &&, dense_alloc_t> &&
		         std::is_nothrow_move_constructible_v<hasher> &&
		         std::is_nothrow_move_constructible_v<key_equal>)
				: hash_base(std::move(other)), cmp_base(std::move(other)),
				  m_sparse(std::move(other.m_sparse), sparse_alloc_t{alloc}),
				  m_dense(std::move(other.m_dense), dense_alloc_t{alloc}),
				  m_max_load_factor(other.m_max_load_factor) {}

		/** Initializes the multiset with the specified bucket count, hasher, comparator and allocator. */
		constexpr explicit dense_multiset(size_type bucket_count, const hasher &hash = hasher{}, const key_equal &cmp = key_equal{},
		                                  const allocator_type &alloc = allocator_type{})
				: hash_base(hash), cmp_base(cmp),
				  m_sparse(sparse_alloc_t{alloc}),
				  m_dense(dense_alloc_t{alloc})
		{
			m_sparse.resize(bucket_count, npos);
			m_dense.reserve(to_capacity(bucket_count));
		}
		/** Initializes the multiset with the specified bucket count, hasher and allocator. */
		constexpr dense_multiset(size_type bucket_count, const hasher &hash, const allocator_type &alloc)
				: dense_multiset(bucket_count, hash, key_equal{}, alloc) {}
		/** Initializes the multiset with the specified bucket count and allocator. */
		constexpr dense_multiset(size_type bucket_count, const allocator_type &alloc)
				: dense_multiset(bucket_count, hasher{}, alloc) {}

		/** Initializes the multiset with an initializer list of elements and the specified bucket count, hasher, comparator and allocator. */
		constexpr dense_multiset(std::initializer_list<value_type> il, size_type bucket_count = initial_capacity, const hasher &hash = hasher{},
		                         const key_equal &cmp = key_equal{}, const allocator_type &alloc = allocator_type{})
				: dense_multiset(il.begin(), il.end(), bucket_count, hash, cmp, alloc) {}
		/** @copydoc dense_multiset */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		constexpr dense_multiset(std::initializer_list<T> il, size_type bucket_count = initial_capacity, const hasher &hash = hasher{},
		                         const key_equal &cmp = key_equal{}, const allocator_type &alloc = allocator_type{})
				: dense_multiset(il.begin(), il.end(), bucket_count, hash, cmp, alloc) {}

		/** Initializes the multiset with an initializer list of elements and the specified bucket count, hasher and allocator. */
		constexpr dense_multiset(std::initializer_list<value_type> il, size_type bucket_count, const hasher &hash, const allocator_type &alloc)
				: dense_multiset(il.begin(), il.end(), bucket_count, hash, key_equal{}, alloc) {}
		/** @copydoc dense_multiset */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		constexpr dense_multiset(std::initializer_list<T> il, size_type bucket_count, const hasher &hash, const allocator_type &alloc)
				: dense_multiset(il.begin(), il.end(), bucket_count, hash, key_equal{}, alloc) {}

		/** Initializes the multiset with an initializer list of elements and the specified bucket count and allocator. */
		constexpr dense_multiset(std::initializer_list<value_type> il, size_type bucket_count, const allocator_type &alloc)
				: dense_multiset(il.begin(), il.end(), bucket_count, hasher{}, alloc) {}
		/** @copydoc dense_multiset */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		constexpr dense_multiset(std::initializer_list<T> il, size_type bucket_count, const allocator_type &alloc)
				: dense_multiset(il.begin(), il.end(), bucket_count, hasher{}, alloc) {}

		/** Initializes the multiset with a range of elements and the specified bucket count, hasher, comparator and allocator. */
		template<typename I>
		constexpr dense_multiset(I first, I last, size_type bucket_count = initial_capacity, const hasher &hash = hasher{},
		                         const key_equal &cmp = key_equal{}, const allocator_type &alloc = allocator_type{})
				: dense_multiset(bucket_count, hash, cmp, alloc) { insert(first, last); }
		/** Initializes the multiset with a range of elements and the specified bucket count, hasher and allocator. */
		template<typename I>
		constexpr dense_multiset(I first, I last, size_type bucket_count, const hasher &hash, const allocator_type &alloc)
				: dense_multiset(first, last, bucket_count, hash, key_equal{}, alloc) {}
		/** Initializes the multiset with a range of elements and the specified bucket count and allocator. */
		template<typename I>
		constexpr dense_multiset(I first, I last, size_type bucket_count, const allocator_type &alloc)
				: dense_multiset(first, last, bucket_count, hasher{}, alloc) {}

		/** Copy-assigns the multiset. */
		constexpr dense_multiset &operator=(const dense_multiset &other)
		{
			if (this != &other)
			{
				hash_base::operator=(other);
				cmp_base::operator=(other);

				m_sparse = other.m_sparse;
				m_dense = other.m_dense;
				m_max_load_factor = other.m_max_load_factor;
			}
			return *this;
		}
		/** Move-assigns the multiset. */
		constexpr dense_multiset &operator=(dense_multiset &&other)
		noexcept(std::is_nothrow_move_assignable_v<hasher> &&
		         std::is_nothrow_move_assignable_v<key_equal> &&
		         std::is_nothrow_move_assignable_v<sparse_t> &&
		         std::is_nothrow_move_assignable_v<dense_t>)
		{
			if (this != &other)
			{
				hash_base::operator=(std::move(other));
				cmp_base::operator=(std::move(other));

				m_sparse = std::move(other.m_sparse);
				m_dense = std::move(other.m_dense);
				m_max_load_factor = other.m_max_load_factor;
			}
			return *this;
		}

		/** Replaces elements of the multiset with elements of the initializer list. */
		constexpr dense_multiset &operator=(std::initializer_list<value_type> il)
		{
			assign(il.begin(), il.end());
			return *this;
		}
		/** @copydoc operator= */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		constexpr dense_multiset &operator=(std::initializer_list<T> il)
		{
			assign(il.begin(), il.end());
			return *this;
		}

		/** Returns iterator to the first element of the multiset.
		 * @note Elements are stored in no particular order. */
		[[nodiscard]] constexpr const_iterator begin() const noexcept { return to_iter(begin_node()); }
		/** @copydoc begin */
		[[nodiscard]] constexpr const_iterator cbegin() const noexcept { return begin(); }
		/** Returns iterator one past the last element of the multiset.
		 * @note Elements are stored in no particular order. */
		[[nodiscard]] constexpr const_iterator end() const noexcept { return to_iter(end_node()); }
		/** @copydoc end */
		[[nodiscard]] constexpr const_iterator cend() const noexcept { return end(); }

		/** Returns reverse iterator to the last element of the multiset.
		 * @note Elements are stored in no particular order. */
		[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator{end()}; }
		/** @copydoc rbegin */
		[[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
		/** Returns reverse iterator one past the first element of the multiset.
		 * @note Elements are stored in no particular order. */
		[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator{begin()}; }
		/** @copydoc rend */
		[[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return rend(); }

		/** Returns the total number of elements within the multiset. */
		[[nodiscard]] constexpr size_type size() const noexcept { return m_dense.size(); }
		/** Checks if the multiset is empty (`size() == 0`). */
		[[nodiscard]] constexpr bool empty() const noexcept { return size() == 0; }
		/** Returns the current capacity of the multiset (taking into account the maximum load factor). */
		[[nodiscard]] constexpr size_type capacity() const noexcept { return to_capacity(bucket_count()); }
		/** Returns the absolute maximum size of the multiset (taking into account the maximum load factor). */
		[[nodiscard]] constexpr size_type max_size() const noexcept { return to_capacity(std::min(m_dense.max_size(), npos - 1)); }
		/** Returns the current load factor of the multiset as if via `size() / bucket_count()`. */
		[[nodiscard]] constexpr float load_factor() const noexcept { return static_cast<float>(size()) / static_cast<float>(bucket_count()); }

		/** Returns the current amount of buckets of the multiset. */
		[[nodiscard]] constexpr size_type bucket_count() const noexcept { return m_sparse.size(); }
		/** Returns the maximum amount of buckets of the multiset. */
		[[nodiscard]] constexpr size_type max_bucket_count() const noexcept { return m_sparse.max_size(); }

		/** Erases all elements from the multiset. */
		constexpr void clear()
		{
			std::fill_n(m_sparse.data(), bucket_count(), make_index_list(npos));
			m_dense.clear();
		}

		/** @brief Inserts an element (of `value_type`) into the multiset if none of it's keys exist within the multiset.
		 * @param value Value of the to-be inserted element.
		 * @return Pair where `first` is the iterator to the inserted or existing element, and `second` is a boolean
		 * indicating whether insertion took place (`true` if element was inserted, `false` otherwise). */
		constexpr std::pair<iterator, bool> insert(const insert_type &value) { return insert_impl(value); }
		/** @copydoc insert */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		constexpr std::pair<iterator, bool> insert(const T &value) { return insert_impl(value); }
		/** @copydoc insert */
		constexpr std::pair<iterator, bool> insert(insert_type &&value) { return insert_impl(std::forward<insert_type>(value)); }
		/** @copydoc insert */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, T &&>>>
		constexpr std::pair<iterator, bool> insert(T &&value) { return insert_impl(std::forward<T>(value)); }
		/** @copybrief insert
		 * @param value Value of the to-be inserted element.
		 * @return Iterator to the inserted or existing element.
		 * @note \p hint has no effect, this overload exists for API compatibility. */
		constexpr iterator insert([[maybe_unused]] const_iterator hint, const insert_type &value) { return insert(value); }
		/** @copydoc insert */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		constexpr iterator insert([[maybe_unused]] const_iterator hint, const T &value) { return insert(value); }
		/** @copydoc insert */
		constexpr iterator insert([[maybe_unused]] const_iterator hint, insert_type &&value) { return insert(std::forward<insert_type>(value)); }
		/** @copydoc insert */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, T &&>>>
		constexpr iterator insert([[maybe_unused]] const_iterator hint, T &&value) { return insert(std::forward<T>(value)); }

		/** Inserts all elements from the range `[first, last)` into the multiset.
		 * @param first Iterator to the first element of the source range.
		 * @param last Iterator one past the last element of the source range. */
		template<typename I>
		constexpr void insert(I first, I last)
		{
			if constexpr (std::is_base_of_v<std::random_access_iterator_tag, typename std::iterator_traits<I>::iterator_category>)
				reserve(static_cast<size_type>(std::distance(first, last)));
			for (; first != last; ++first) insert(*first);
		}
		/** Inserts all elements of an initializer list into the multiset. */
		constexpr void insert(std::initializer_list<value_type> il) { return insert(il.begin(), il.end()); }
		/** @copydoc insert */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		constexpr void insert(std::initializer_list<T> il) { return insert(il.begin(), il.end()); }

		/** @brief Inserts an in-place constructed element (of `value_type`) into the multiset if none of it's keys exist within the multiset.
		 * @param args Arguments passed to constructor of `value_type`.
		 * @return Pair where `first` is the iterator to the inserted or existing element, and `second` is a boolean
		 * indicating whether insertion took place (`true` if element was inserted, `false` otherwise). */
		template<typename... Args>
		constexpr std::pair<iterator, bool> emplace(Args &&...args) { return emplace_impl(std::forward<Args>(args)...); }
		/** @copybrief emplace
		 * @param args Arguments passed to constructor of `value_type`.
		 * @return Iterator to the inserted or existing element.
		 * @note \p hint has no effect, this overload exists for API compatibility. */
		template<typename... Args>
		constexpr iterator emplace_hint([[maybe_unused]] const_iterator hint, Args &&...args) { return emplace(std::forward<Args>(args)...).first; }

		/** Removes the specified element from the multiset.
		 * @tparam I Index of the key within the multiset key pack.
		 * @param key `I`th Key of the element to search for.
		 * @return Iterator to the element following the erased one, or `end()`. */
		template<std::size_t I>
		constexpr iterator erase(const std::tuple_element_t<I, key_type> &key) { return erase(find<I>(key)); }
		/** @copydoc find
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<std::size_t I, typename K, typename = std::enable_if_t<is_transparent::value && std::is_invocable_v<hasher, K>>>
		constexpr iterator erase(const K &key) { return erase(find<I>(key)); }

		/** Removes the specified element from the multiset.
		 * @param pos Iterator pointing to the element to remove.
		 * @return Iterator to the element following the erased one, or `end()`. */
		constexpr iterator erase(const_iterator pos) { return erase_impl(pos.m_node - begin_node()); }
		/** Removes a range of elements from the multiset.
		 * @param first Iterator to the first element of the to-be removed range.
		 * @param last Iterator one past the last element of the to-be removed range.
		 * @return Iterator to the element following the erased range, or `end()`. */
		constexpr iterator erase(const_iterator first, const_iterator last)
		{
			iterator result = end();
			while (last != first) result = erase(--last);
			return result;
		}

		/** Searches for the specified element within the multiset.
		 * @tparam I Index of the key within the multiset key pack.
		 * @param key `I`th Key of the element to search for.
		 * @return Iterator to the specified element, or `end()`. */
		template<std::size_t I>
		[[nodiscard]] constexpr iterator find(const std::tuple_element_t<I, key_type> &key) const { return to_iter(find_node<I>(hash(key), key).first); }
		/** @copydoc find
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<std::size_t I, typename K, typename = std::enable_if_t<is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] constexpr iterator find(const K &key) const { return to_iter(find_node<I>(hash(key), key).first); }
		/** Checks if the specified element is present within the multiset as if by `find(key) != end()`.
		 * @tparam I Index of the key within the multiset key pack.
		 * @param key `I`th Key of the element to search for.
		 * @return `true` if the element is present within the multiset, `false` otherwise. */
		template<std::size_t I>
		[[nodiscard]] constexpr bool contains(const std::tuple_element_t<I, key_type> &key) const { return find<I>(key) != end(); }
		/** @copydoc contains
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<std::size_t I, typename K, typename = std::enable_if_t<is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] constexpr bool contains(const K &key) const { return find<I>(key) != end(); }

		/** Reserves space for at least `n` buckets and rehashes the multiset if necessary.
		 * @note The new amount of buckets is clamped to be at least `size() / max_load_factor()`. */
		constexpr void rehash(size_type n)
		{
			/* Adjust the capacity to be at least large enough to fit the current size. */
			const auto load_cap = static_cast<size_type>(static_cast<float>(size()) / m_max_load_factor);
			n = std::max(std::max(load_cap, n), initial_capacity);

			/* Don't do anything if the capacity did not change after the adjustment. */
			if (n != bucket_count())
				rehash_impl(n);
		}
		/** Reserves space for at least `n` elements. */
		constexpr void reserve(size_type n)
		{
			m_dense.reserve(n);
			rehash(static_cast<size_type>(static_cast<float>(n) / m_max_load_factor));
		}
		/** Returns the current maximum load factor. */
		[[nodiscard]] constexpr float max_load_factor() const noexcept { return m_max_load_factor; }
		/** Sets the current maximum load factor. */
		constexpr void max_load_factor(float f) noexcept { m_max_load_factor = f; }

		[[nodiscard]] constexpr allocator_type get_allocator() const { return allocator_type{m_dense.get_allocator()}; }
		[[nodiscard]] constexpr hasher hash_function() const { return hash_base::value(); }
		[[nodiscard]] constexpr key_equal key_eq() const { return cmp_base::value(); }

		[[nodiscard]] constexpr bool operator==(const dense_multiset &other) const { return std::is_permutation(begin(), end(), other.begin(), other.end()); }
#if __cplusplus < 202002L
		[[nodiscard]] constexpr bool operator!=(const dense_multiset &other) const { return !std::is_permutation(begin(), end(), other.begin(), other.end()); }
#endif

		constexpr void swap(dense_multiset &other) noexcept(std::is_nothrow_swappable_v<hasher> && std::is_nothrow_swappable_v<key_equal> &&
		                                                    std::is_nothrow_swappable_v<sparse_t> && std::is_nothrow_swappable_v<dense_t>)
		{
			std::swap(m_sparse, other.m_sparse);
			std::swap(m_dense, other.m_dense);
			std::swap(m_max_load_factor, other.m_max_load_factor);
		}

	private:
		[[nodiscard]] constexpr auto to_capacity(size_type n) const noexcept { return static_cast<size_type>(static_cast<float>(n) * m_max_load_factor); }

		template<typename T>
		[[nodiscard]] constexpr std::size_t hash(const T &k) const { return hash_function()(k); }
		template<typename T, typename U>
		[[nodiscard]] constexpr bool cmp(const T &a, const U &b) const { return key_eq()(a, b); }

		[[nodiscard]] constexpr auto *begin_node() const noexcept { return const_cast<bucket_node *>(m_dense.data()); }
		[[nodiscard]] constexpr auto *end_node() const noexcept { return const_cast<bucket_node *>(m_dense.data()) + size(); }

		template<std::size_t I>
		[[nodiscard]] constexpr auto *get_chain(std::size_t h) const noexcept { return const_cast<size_type *>(m_sparse[h % bucket_count()].data() + I); }
		template<std::size_t I, typename T>
		[[nodiscard]] constexpr auto find_node(std::size_t h, const T &key) const noexcept -> std::pair<bucket_node *, size_type *>
		{
			auto *idx = get_chain<I>(h);
			while (*idx != npos)
			{
				auto &entry = m_dense[*idx];
				if (entry.hash[I] == h && cmp(key, entry.template key<I>()))
					return {const_cast<bucket_node *>(&entry), idx};
				idx = const_cast<size_type *>(&entry.next[I]);
			}
			return {end_node(), idx};
		}
		template<std::size_t I>
		constexpr void move_chain(size_type from, size_type to) noexcept
		{
			auto &src = m_dense[from];

			/* Find the chain offset pointing to the old position & replace it with the new position. */
			for (auto *chain_idx = get_chain<I>(src.hash[I]); *chain_idx != npos; chain_idx = &(m_dense[*chain_idx].next[I]))
				if (*chain_idx == from)
				{
					*chain_idx = to;
					break;
				}
		}

		template<std::size_t I>
		constexpr void insert_node(bucket_node &node, size_type pos) noexcept
		{
			auto *chain_idx = get_chain<I>(node.hash[I]);
			node.next[I] = *chain_idx;
			*chain_idx = pos;
		}
		template<size_type... Is>
		constexpr void move_node(size_type from, size_type to)
		{
			(move_chain<Is>(from, to), ...);
			m_dense[to] = std::move(m_dense[from]);
		}
		template<size_type I>
		constexpr void unlink_node(bucket_node &node)
		{
			const auto &key = node.template key<I>();
			const auto h = node.hash[I];
			for (auto *chain_idx = get_chain<I>(h); *chain_idx != npos;)
			{
				const auto pos = *chain_idx;
				auto entry_ptr = m_dense.data() + static_cast<difference_type>(pos);

				/* Un-link the entry from the chain. */
				if (entry_ptr->hash[I] == h && cmp(key, entry_ptr->template key<I>()))
				{
					*chain_idx = entry_ptr->next[I];
					break;
				}
				chain_idx = &(entry_ptr->next[I]);
			}
		}

		template<typename T>
		[[nodiscard]] constexpr std::pair<iterator, bool> insert_impl(T &&value) { return insert_impl(keys_index{}, std::forward<T>(value)); }
		template<std::size_t... Is, typename T>
		[[nodiscard]] constexpr std::pair<iterator, bool> insert_impl(std::index_sequence<Is...>, T &&value)
		{
			maybe_rehash();

			/* Find any conflicts for the new node. */
			auto hlist = hash_list{hash(std::get<Is>(value))...};
			auto chain_list = std::array{find_node<Is>(hlist[Is], std::get<Is>(value))...};
			for (auto [node, chain]: chain_list)
				if (*chain != npos)
				{
					/* Found a conflict, return the existing node. */
					return {to_iter(node), false};
				}

			/* No conflicts found, preform insertion. */
			const auto insert_pos = size();
			auto &node = m_dense.emplace_back(std::forward<T>(value));
			node.hash = hlist;

			(insert_node<Is>(node, insert_pos), ...);
			return {to_iter(&node), true};
		}

		template<typename... Args>
		[[nodiscard]] constexpr std::pair<iterator, bool> emplace_impl(Args &&...args) { return emplace_impl(keys_index{}, std::forward<Args>(args)...); }
		template<std::size_t... Is, typename... Args>
		[[nodiscard]] constexpr std::pair<iterator, bool> emplace_impl(std::index_sequence<Is...>, Args &&...args)
		{
			maybe_rehash();

			/* Create a placeholder node. */
			auto insert_pos = size();
			auto *node_ptr = &m_dense.emplace_back(std::forward<Args>(args)...);
			node_ptr->hash = hash_list{hash(node_ptr->template key<Is>())...};

			/* Find any conflicts for the new node. */
			auto chain_list = std::array{find_node<Is>(node_ptr->hash[Is], node_ptr->template key<Is>())...};
			for (auto [node, chain]: chain_list)
				if (*chain != npos)
				{
					/* Found a conflict, erase the temporary and return the existing node. */
					m_dense.pop_back();
					return {to_iter(node), false};
				}

			/* No conflicts found, commit the node to every chain. */
			(insert_node<Is>(*node_ptr, insert_pos), ...);
			return {to_iter(node_ptr), true};
		}

		constexpr iterator erase_impl(size_type pos) { return erase_impl(keys_index{}, pos); }
		template<size_type... Is>
		constexpr iterator erase_impl(std::index_sequence<Is...>, size_type pos)
		{
			if (pos < size())
			{
				/* Remove the entry from every bucket chain. */
				auto &node = m_dense[pos];
				(unlink_node<Is>(node), ...);

				/* If the entry is not at the end, swap it with the last entry. */
				if (const auto end_pos = size() - 1; pos != end_pos)
					move_node<Is...>(end_pos, pos);

				m_dense.pop_back();
			}
			return begin() + static_cast<difference_type>(pos);
		}

		constexpr void maybe_rehash()
		{
			if (load_factor() >= m_max_load_factor)
				rehash(bucket_count() * 2);
		}

		template<size_type... Is>
		constexpr void rehash_impl(size_type new_cap) { rehash_impl(keys_index{}, new_cap); }
		template<size_type... Is>
		constexpr void rehash_impl(std::index_sequence<Is...>, size_type new_cap)
		{
			/* Clear & reserve the vector filled with npos. */
			m_sparse.clear();
			m_sparse.resize(new_cap, make_index_list(npos));

			/* Go through each entry & re-insert it. */
			for (size_type i = 0; i < size(); ++i)
				(insert_node<Is>(m_dense[i], i), ...);
		}

		template<typename I>
		constexpr void assign(I first, I last)
		{
			clear();
			insert(first, last);
		}

		sparse_t m_sparse = sparse_t(initial_capacity, make_index_list(npos));
		dense_t m_dense;

		float m_max_load_factor = initial_load_factor;
	};
}