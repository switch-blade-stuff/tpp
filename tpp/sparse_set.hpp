/*
 * Created by switchblade on 2022-12-26.
 */

#pragma once

#include "detail/swiss_table.hpp"

namespace tpp
{
	/** @brief Hash set based on SwissHash open addressing hash table.
	 *
	 * Internally, sparse set stores it's elements in open-addressing element and metadata buffers.
	 * Insert and erase operations on a sparse set may invalidate iterators to it's elements due to the table being rehashed.
	 * Reference implementation details can be found at <a href="https://abseil.io/about/design/swisstables">https://abseil.io/about/design/swisstables</a>.
	 *
	 * @tparam Key Key type stored by the set.
	 * @tparam KeyHash Hash functor used by the set.
	 * @tparam KeyCmp Compare functor used by the set.
	 * @tparam Alloc Allocator used by the set. */
	template<typename Key, typename KeyHash = detail::default_hash<Key>, typename KeyCmp = std::equal_to<Key>, typename Alloc = std::allocator<Key>>
	class sparse_set
	{
		struct traits_t
		{
			using link_type = detail::empty_link;

			using pointer = const Key *;
			using const_pointer = const Key *;
			using reference = const Key &;
			using const_reference = const Key &;

			template<typename T>
			static constexpr auto &get_key(T &value) noexcept { return value; }
		};

		using table_t = detail::swiss_table<Key, Key, Key, KeyHash, KeyCmp, Alloc, traits_t>;

	public:
		using insert_type = typename table_t::insert_type;
		using value_type = typename table_t::value_type;
		using key_type = typename table_t::key_type;

		/** Reference to `const value_type`. */
		using reference = typename table_t::reference;
		/** @copydoc reference */
		using const_reference = typename table_t::const_reference;
		/** Pointer to `const value_type`. */
		using pointer = typename table_t::pointer;
		/** @copydoc pointer */
		using const_pointer = typename table_t::const_pointer;

		/** Forward iterator to `const value_type`. */
		using iterator = typename table_t::const_iterator;
		/** @copydoc const_iterator */
		using const_iterator = typename table_t::const_iterator;

		using size_type = typename table_t::size_type;
		using difference_type = typename table_t::difference_type;

		using hasher = typename table_t::hasher;
		using key_equal = typename table_t::key_equal;
		using allocator_type = typename table_t::allocator_type;

	public:
		/** Initializes the set with default capacity. */
		sparse_set() = default;
		/** Initializes the set with default capacity using the specified allocator. */
		explicit sparse_set(const allocator_type &alloc) : m_table(alloc) {}

		/** Copy-constructs the set. */
		sparse_set(const sparse_set &other) = default;
		/** Copy-constructs the set using the specified allocator. */
		sparse_set(const sparse_set &other, const allocator_type &alloc) : m_table(other.m_table, alloc) {}

		/** Move-constructs the set. */
		sparse_set(sparse_set &&other) noexcept(std::is_nothrow_move_constructible_v<table_t>) = default;
		/** Move-constructs the set using the specified allocator. */
		sparse_set(sparse_set &&other, const allocator_type &alloc) noexcept(std::is_nothrow_constructible_v<table_t, table_t &&, allocator_type>) : m_table(std::move(other.m_table), alloc) {}

		/** Initializes the set with the specified bucket count, hasher, comparator and allocator. */
		explicit sparse_set(size_type bucket_count, const hasher &hash = hasher{}, const key_equal &cmp = key_equal{}, const allocator_type &alloc = allocator_type{}) : m_table(bucket_count, hash, cmp, alloc) {}
		/** Initializes the set with the specified bucket count, hasher and allocator. */
		sparse_set(size_type bucket_count, const hasher &hash, const allocator_type &alloc) : sparse_set(bucket_count, hash, key_equal{}, alloc) {}
		/** Initializes the set with the specified bucket count and allocator. */
		sparse_set(size_type bucket_count, const allocator_type &alloc) : sparse_set(bucket_count, hasher{}, alloc) {}

		/** Initializes the set with an initializer list of elements and the specified bucket count, hasher, comparator and allocator. */
		sparse_set(std::initializer_list<value_type> il, size_type bucket_count = 0, const hasher &hash = hasher{}, const key_equal &cmp = key_equal{}, const allocator_type &alloc = allocator_type{})
				: sparse_set(il.begin(), il.end(), bucket_count, hash, cmp, alloc) {}
		/** @copydoc sparse_set */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		sparse_set(std::initializer_list<T> il, size_type bucket_count = 0, const hasher &hash = hasher{}, const key_equal &cmp = key_equal{}, const allocator_type &alloc = allocator_type{})
				: sparse_set(il.begin(), il.end(), bucket_count, hash, cmp, alloc) {}

		/** Initializes the set with an initializer list of elements and the specified bucket count, hasher and allocator. */
		sparse_set(std::initializer_list<value_type> il, size_type bucket_count, const hasher &hash, const allocator_type &alloc) : sparse_set(il.begin(), il.end(), bucket_count, hash, key_equal{}, alloc) {}
		/** @copydoc sparse_set */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		sparse_set(std::initializer_list<T> il, size_type bucket_count, const hasher &hash, const allocator_type &alloc): sparse_set(il.begin(), il.end(), bucket_count, hash, key_equal{}, alloc) {}

		/** Initializes the set with an initializer list of elements and the specified bucket count and allocator. */
		sparse_set(std::initializer_list<value_type> il, size_type bucket_count, const allocator_type &alloc) : sparse_set(il.begin(), il.end(), bucket_count, hasher{}, alloc) {}
		/** @copydoc sparse_set */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		sparse_set(std::initializer_list<T> il, size_type bucket_count, const allocator_type &alloc): sparse_set(il.begin(), il.end(), bucket_count, hasher{}, alloc) {}

		/** Initializes the set with a range of elements and the specified bucket count, hasher, comparator and allocator. */
		template<typename I>
		sparse_set(I first, I last, size_type bucket_count = 0, const hasher &hash = hasher{}, const key_equal &cmp = key_equal{}, const allocator_type &alloc = allocator_type{})
				: m_table(first, last, bucket_count, hash, cmp, alloc) {}
		/** Initializes the set with a range of elements and the specified bucket count, hasher and allocator. */
		template<typename I>
		sparse_set(I first, I last, size_type bucket_count, const hasher &hash, const allocator_type &alloc) : sparse_set(first, last, bucket_count, hash, key_equal{}, alloc) {}
		/** Initializes the set with a range of elements and the specified bucket count and allocator. */
		template<typename I>
		sparse_set(I first, I last, size_type bucket_count, const allocator_type &alloc) : sparse_set(first, last, bucket_count, hasher{}, alloc) {}

		/** Copy-assigns the set. */
		sparse_set &operator=(const sparse_set &) = default;
		/** Move-assigns the set. */
		sparse_set &operator=(sparse_set &&) noexcept(std::is_nothrow_move_assignable_v<table_t>) = default;

		/** Replaces elements of the set with elements of the initializer list. */
		sparse_set &operator=(std::initializer_list<value_type> il)
		{
			m_table.assign(il.begin(), il.end());
			return *this;
		}
		/** @copydoc operator= */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		sparse_set &operator=(std::initializer_list<T> il)
		{
			m_table.assign(il.begin(), il.end());
			return *this;
		}

		/** Returns iterator to the first element of the set.
		 * @note Elements are stored in no particular order. */
		[[nodiscard]] constexpr const_iterator begin() const noexcept { return m_table.begin(); }
		/** @copydoc begin */
		[[nodiscard]] constexpr const_iterator cbegin() const noexcept { return begin(); }
		/** Returns iterator one past the last element of the set.
		 * @note Elements are stored in no particular order. */
		[[nodiscard]] constexpr const_iterator end() const noexcept { return m_table.end(); }
		/** @copydoc end */
		[[nodiscard]] constexpr const_iterator cend() const noexcept { return end(); }

		/** Returns the total number of elements within the set. */
		[[nodiscard]] constexpr size_type size() const noexcept { return m_table.size(); }
		/** Checks if the set is empty (`size() == 0`). */
		[[nodiscard]] constexpr bool empty() const noexcept { return m_table.size() == 0; }
		/** Returns the current capacity of the set (taking into account the maximum load factor). */
		[[nodiscard]] constexpr size_type capacity() const noexcept { return m_table.capacity(); }
		/** Returns the absolute maximum size of the set (taking into account the maximum load factor). */
		[[nodiscard]] constexpr size_type max_size() const noexcept { return m_table.max_size(); }
		/** Returns the current load factor of the set as if via `size() / bucket_count()`. */
		[[nodiscard]] constexpr float load_factor() const noexcept { return m_table.load_factor(); }

		/** Erases all elements from the set. */
		void clear() { m_table.clear(); }

		/** @brief Inserts an element (of `value_type`) into the set if it does not exist yet.
		 * @param value Value of the to-be inserted element.
		 * @return Pair where `first` is the iterator to the inserted or existing element, and `second` is a boolean
		 * indicating whether insertion took place (`true` if element was inserted, `false` otherwise). */
		std::pair<iterator, bool> insert(const insert_type &value) { return m_table.insert(value); }
		/** @copydoc insert */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		std::pair<iterator, bool> insert(const T &value) { return m_table.insert(value); }
		/** @copydoc insert */
		std::pair<iterator, bool> insert(insert_type &&value) { return m_table.insert(std::forward<insert_type>(value)); }
		/** @copydoc insert */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, T &&>>>
		std::pair<iterator, bool> insert(T &&value) { return m_table.insert(std::forward<T>(value)); }
		/** @copybrief insert
		 * @param value Value of the to-be inserted element.
		 * @return Iterator to the inserted or existing element.
		 * @note \p hint has no effect, this overload exists for API compatibility. */
		iterator insert(const_iterator hint, const insert_type &value) { return m_table.insert(hint, value); }
		/** @copydoc insert */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		iterator insert(const_iterator hint, const T &value) { return m_table.insert(hint, value); }
		/** @copydoc insert */
		iterator insert(const_iterator hint, insert_type &&value) { return m_table.insert(hint, std::forward<insert_type>(value)); }
		/** @copydoc insert */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, T &&>>>
		iterator insert(const_iterator hint, T &&value) { return m_table.insert(hint, std::forward<T>(value)); }

		/** Inserts all elements from the range `[first, last)` into the set.
		 * @param first Iterator to the first element of the source range.
		 * @param last Iterator one past the last element of the source range. */
		template<typename I>
		void insert(I first, I last) { return m_table.insert(first, last); }
		/** Inserts all elements of an initializer list into the set. */
		void insert(std::initializer_list<value_type> il) { return insert(il.begin(), il.end()); }
		/** @copydoc insert */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		void insert(std::initializer_list<T> il) { return insert(il.begin(), il.end()); }

		/** @brief Inserts an in-place constructed element (of `value_type`) into the set if it does not exist yet.
		 * @param args Arguments passed to constructor of `value_type`.
		 * @return Pair where `first` is the iterator to the inserted or existing element, and `second` is a boolean
		 * indicating whether insertion took place (`true` if element was inserted, `false` otherwise). */
		template<typename... Args>
		std::pair<iterator, bool> emplace(Args &&...args) { return m_table.emplace(std::forward<Args>(args)...); }
		/** @copybrief emplace
		 * @param args Arguments passed to constructor of `value_type`.
		 * @return Iterator to the inserted or existing element.
		 * @note \p hint has no effect, this overload exists for API compatibility. */
		template<typename... Args>
		iterator emplace_hint(const_iterator hint, Args &&...args) { return m_table.emplace_hint(hint, std::forward<Args>(args)...); }

		/** Removes the specified element from the set.
		 * @param key Key of the element to remove.
		 * @return Iterator to the element following the erased one, or `end()`. */
		iterator erase(const key_type &key) { return m_table.erase(key); }
		/** @copydoc erase
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		iterator erase(const K &key) { return m_table.erase(key); }

		/** Removes the specified element from the set.
		 * @param pos Iterator pointing to the element to remove.
		 * @return Iterator to the element following the erased one, or `end()`. */
		iterator erase(const_iterator pos) { return m_table.erase(pos); }

		/** Removes a range of elements from the set.
		 * @param first Iterator to the first element of the to-be removed range.
		 * @param last Iterator one past the last element of the to-be removed range.
		 * @return Iterator to the element following the erased range, or `end()`. */
		iterator erase(const_iterator first, const_iterator last) { return m_table.erase(first, last); }

		/** Searches for the specified element within the set.
		 * @param key Key of the element to search for.
		 * @return Iterator to the specified element, or `end()`. */
		[[nodiscard]] iterator find(const key_type &key) const { return m_table.find(key); }
		/** @copydoc find
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] iterator find(const K &key) const { return m_table.find(key); }

		/** Checks if the specified element is present within the set as if by `find(key) != end()`.
		 * @param key Key of the element to search for.
		 * @return `true` if the element is present within the set, `false` otherwise. */
		[[nodiscard]] bool contains(const key_type &key) const { return m_table.contains(key); }
		/** @copydoc contains
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] bool contains(const K &key) const { return m_table.contains(key); }

		/** Returns the current amount of buckets of the set. */
		[[nodiscard]] constexpr size_type bucket_count() const noexcept { return m_table.bucket_count(); }
		/** Returns the maximum amount of buckets of the set. */
		[[nodiscard]] constexpr size_type max_bucket_count() const noexcept { return m_table.max_bucket_count(); }

		/** Reserves space for at least `n` elements. */
		void reserve(size_type n) { m_table.reserve(n); }
		/** Reserves space for at least `n` buckets and rehashes the set if necessary.
		 * @note The new amount of buckets is clamped to be at least `size() / max_load_factor()`. */
		void rehash(size_type n) { m_table.rehash(n); }
		/** Returns the maximum load factor. */
		[[nodiscard]] constexpr float max_load_factor() const noexcept { return m_table.max_load_factor(); }

		[[nodiscard]] allocator_type get_allocator() const { return allocator_type{m_table.get_allocator()}; }
		[[nodiscard]] hasher hash_function() const { return m_table.get_hash(); }
		[[nodiscard]] key_equal key_eq() const { return m_table.get_cmp(); }

		[[nodiscard]] bool operator==(const sparse_set &other) const
		{
			return std::is_permutation(begin(), end(), other.begin(), other.end());
		}
#if (__cplusplus < 202002L && (!defined(_MSVC_LANG) || _MSVC_LANG < 202002L))
		[[nodiscard]] bool operator!=(const sparse_set &other) const
		{
			return !std::is_permutation(begin(), end(), other.begin(), other.end());
		}
#endif

		void swap(sparse_set &other) noexcept(std::is_nothrow_swappable_v<table_t>) { m_table.swap(other.m_table); }

	private:
		table_t m_table;
	};

	/** Erases all elements from the set \p set that satisfy the predicate \p pred.
	 * @return Amount of elements erased. */
	template<typename K, typename H, typename C, typename A, typename P>
	inline typename sparse_set<K, H, C, A>::size_type erase_if(sparse_set<K, H, C, A> &set, P pred)
	{
		typename sparse_set<K, H, C, A>::size_type result = 0;
		for (auto i = set.cbegin(), last = set.cend(); i != last;)
		{
			if (pred(*i))
			{
				i = set.erase(i);
				++result;
			}
			else
				++i;
		}
		return result;
	}

	template<typename K, typename H, typename C, typename A>
	inline void swap(sparse_set<K, H, C, A> &a, sparse_set<K, H, C, A> &b) noexcept(std::is_nothrow_swappable_v<sparse_set<K, H, C, A>>) { a.swap(b); }

	template<typename I, typename Hash = detail::default_hash<detail::iter_key_t<I>>, typename Cmp = std::equal_to<detail::iter_key_t<I>>, typename Alloc = std::allocator<detail::iter_key_t<I>>>
	sparse_set(I, I, typename detail::deduce_set_t<sparse_set, I, Hash, Cmp, Alloc>::size_type = 0, Hash = Hash{}, Cmp = Cmp{}, Alloc = Alloc{})
	-> sparse_set<detail::iter_key_t<I>, Hash, Cmp, Alloc>;
	template<typename I, typename Hash, typename Alloc>
	sparse_set(I, I, typename detail::deduce_set_t<sparse_set, I, Hash, std::equal_to<detail::iter_key_t<I>>, Alloc>::size_type, Hash, Alloc)
	-> sparse_set<detail::iter_key_t<I>, Hash, std::equal_to<detail::iter_key_t<I>>, Alloc>;
	template<typename I, typename Alloc>
	sparse_set(I, I, typename detail::deduce_set_t<sparse_set, I, detail::default_hash<detail::iter_key_t<I>>, std::equal_to<detail::iter_key_t<I>>, Alloc>::size_type, Alloc)
	-> sparse_set<detail::iter_key_t<I>, detail::default_hash<detail::iter_key_t<I>>, std::equal_to<detail::iter_key_t<I>>, Alloc>;
	template<typename I, typename Alloc>
	sparse_set(I, I, Alloc) -> sparse_set<detail::iter_key_t<I>, detail::default_hash<detail::iter_key_t<I>>, std::equal_to<detail::iter_key_t<I>>, Alloc>;

	template<typename K, typename Hash = detail::default_hash<K>, typename Cmp = std::equal_to<K>, typename Alloc = std::allocator<K>>
	sparse_set(std::initializer_list<K>, typename sparse_set<K, Hash, Cmp, Alloc>::size_type = 0, Hash = Hash{}, Cmp = Cmp{}, Alloc = Alloc{}) -> sparse_set<K, Hash, Cmp, Alloc>;
	template<typename K, typename Hash, typename Alloc>
	sparse_set(std::initializer_list<K>, typename sparse_set<K, Hash, std::equal_to<K>, Alloc>::size_type, Hash, Alloc) -> sparse_set<K, Hash, std::equal_to<K>, Alloc>;
	template<typename K, typename Alloc>
	sparse_set(std::initializer_list<K>, typename sparse_set<K, detail::default_hash<K>, std::equal_to<K>, Alloc>::size_type, Alloc) -> sparse_set<K, detail::default_hash<K>, std::equal_to<K>, Alloc>;
	template<typename K, typename Alloc>
	sparse_set(std::initializer_list<K>, Alloc) -> sparse_set<K, detail::default_hash<K>, std::equal_to<K>, Alloc>;

	/** @brief Ordered hash set based on SwissHash open addressing hash table.
	 *
	 * Internally, ordered sparse set stores it's elements in open-addressing element and metadata buffers with additional ordering
	 * links between elements. Insert and erase operations on an ordered sparse set may invalidate iterators to it's elements
	 * due to the table being rehashed. Reference implementation details can be found at <a href="https://abseil.io/about/design/swisstables">https://abseil.io/about/design/swisstables</a>.
	 *
	 * @tparam Key Key type stored by the set.
	 * @tparam KeyHash Hash functor used by the set.
	 * @tparam KeyCmp Compare functor used by the set.
	 * @tparam Alloc Allocator used by the set. */
	template<typename Key, typename KeyHash = detail::default_hash<Key>, typename KeyCmp = std::equal_to<Key>, typename Alloc = std::allocator<Key>>
	class ordered_sparse_set
	{
		struct traits_t
		{
			using link_type = detail::ordered_link;

			using pointer = const Key *;
			using const_pointer = const Key *;
			using reference = const Key &;
			using const_reference = const Key &;

			template<typename T>
			static constexpr auto &get_key(T &value) noexcept { return value; }
		};

		using table_t = detail::swiss_table<Key, Key, Key, KeyHash, KeyCmp, Alloc, traits_t>;

	public:
		using insert_type = typename table_t::insert_type;
		using value_type = typename table_t::value_type;
		using key_type = typename table_t::key_type;

		/** Reference to `const value_type`. */
		using reference = typename table_t::reference;
		/** @copydoc reference */
		using const_reference = typename table_t::const_reference;

		/** Pointer to `const value_type`. */
		using pointer = typename table_t::pointer;
		/** @copydoc pointer */
		using const_pointer = typename table_t::const_pointer;

		/** Bidirectional iterator to `const value_type`. */
		using iterator = typename table_t::const_iterator;
		/** @copydoc const_iterator */
		using const_iterator = typename table_t::const_iterator;

		using size_type = typename table_t::size_type;
		using difference_type = typename table_t::difference_type;

		using hasher = typename table_t::hasher;
		using key_equal = typename table_t::key_equal;
		using allocator_type = typename table_t::allocator_type;

	public:
		/** Initializes the set with default capacity. */
		ordered_sparse_set() = default;
		/** Initializes the set with default capacity using the specified allocator. */
		explicit ordered_sparse_set(const allocator_type &alloc) : m_table(alloc) {}

		/** Copy-constructs the set. */
		ordered_sparse_set(const ordered_sparse_set &other) = default;
		/** Copy-constructs the set using the specified allocator. */
		ordered_sparse_set(const ordered_sparse_set &other, const allocator_type &alloc) : m_table(other.m_table, alloc) {}

		/** Move-constructs the set. */
		ordered_sparse_set(ordered_sparse_set &&other) noexcept(std::is_nothrow_move_constructible_v<table_t>) = default;
		/** Move-constructs the set using the specified allocator. */
		ordered_sparse_set(ordered_sparse_set &&other, const allocator_type &alloc) noexcept(std::is_nothrow_constructible_v<table_t, table_t &&, allocator_type>) : m_table(std::move(other.m_table), alloc) {}

		/** Initializes the set with the specified bucket count, hasher, comparator and allocator. */
		explicit ordered_sparse_set(size_type bucket_count, const hasher &hash = hasher{}, const key_equal &cmp = key_equal{}, const allocator_type &alloc = allocator_type{}) : m_table(bucket_count, hash, cmp, alloc) {}
		/** Initializes the set with the specified bucket count, hasher and allocator. */
		ordered_sparse_set(size_type bucket_count, const hasher &hash, const allocator_type &alloc) : ordered_sparse_set(bucket_count, hash, key_equal{}, alloc) {}
		/** Initializes the set with the specified bucket count and allocator. */
		ordered_sparse_set(size_type bucket_count, const allocator_type &alloc) : ordered_sparse_set(bucket_count, hasher{}, alloc) {}

		/** Initializes the set with an initializer list of elements and the specified bucket count, hasher, comparator and allocator. */
		ordered_sparse_set(std::initializer_list<value_type> il, size_type bucket_count = 0, const hasher &hash = hasher{}, const key_equal &cmp = key_equal{}, const allocator_type &alloc = allocator_type{})
				: ordered_sparse_set(il.begin(), il.end(), bucket_count, hash, cmp, alloc) {}
		/** @copydoc sparse_set */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		ordered_sparse_set(std::initializer_list<T> il, size_type bucket_count = 0, const hasher &hash = hasher{}, const key_equal &cmp = key_equal{}, const allocator_type &alloc = allocator_type{})
				: ordered_sparse_set(il.begin(), il.end(), bucket_count, hash, cmp, alloc) {}

		/** Initializes the set with an initializer list of elements and the specified bucket count, hasher and allocator. */
		ordered_sparse_set(std::initializer_list<value_type> il, size_type bucket_count, const hasher &hash, const allocator_type &alloc) : ordered_sparse_set(il.begin(), il.end(), bucket_count, hash, key_equal{}, alloc) {}
		/** @copydoc sparse_set */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		ordered_sparse_set(std::initializer_list<T> il, size_type bucket_count, const hasher &hash, const allocator_type &alloc) : ordered_sparse_set(il.begin(), il.end(), bucket_count, hash, key_equal{}, alloc) {}

		/** Initializes the set with an initializer list of elements and the specified bucket count and allocator. */
		ordered_sparse_set(std::initializer_list<value_type> il, size_type bucket_count, const allocator_type &alloc) : ordered_sparse_set(il.begin(), il.end(), bucket_count, hasher{}, alloc) {}
		/** @copydoc sparse_set */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		ordered_sparse_set(std::initializer_list<T> il, size_type bucket_count, const allocator_type &alloc) : ordered_sparse_set(il.begin(), il.end(), bucket_count, hasher{}, alloc) {}

		/** Initializes the set with a range of elements and the specified bucket count, hasher, comparator and allocator. */
		template<typename I>
		ordered_sparse_set(I first, I last, size_type bucket_count = 0, const hasher &hash = hasher{}, const key_equal &cmp = key_equal{}, const allocator_type &alloc = allocator_type{})
				: m_table(first, last, bucket_count, hash, cmp, alloc) {}
		/** Initializes the set with a range of elements and the specified bucket count, hasher and allocator. */
		template<typename I>
		ordered_sparse_set(I first, I last, size_type bucket_count, const hasher &hash, const allocator_type &alloc) : ordered_sparse_set(first, last, bucket_count, hash, key_equal{}, alloc) {}
		/** Initializes the set with a range of elements and the specified bucket count and allocator. */
		template<typename I>
		ordered_sparse_set(I first, I last, size_type bucket_count, const allocator_type &alloc) : ordered_sparse_set(first, last, bucket_count, hasher{}, alloc) {}

		/** Copy-assigns the set. */
		ordered_sparse_set &operator=(const ordered_sparse_set &) = default;
		/** Move-assigns the set. */
		ordered_sparse_set &operator=(ordered_sparse_set &&) noexcept(std::is_nothrow_move_assignable_v<table_t>) = default;

		/** Replaces elements of the set with elements of the initializer list. */
		ordered_sparse_set &operator=(std::initializer_list<value_type> il)
		{
			m_table.assign(il.begin(), il.end());
			return *this;
		}
		/** @copydoc operator= */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		ordered_sparse_set &operator=(std::initializer_list<T> il)
		{
			m_table.assign(il.begin(), il.end());
			return *this;
		}

		/** Returns iterator to the first element of the set. */
		[[nodiscard]] constexpr const_iterator begin() const noexcept { return m_table.begin(); }
		/** @copydoc begin */
		[[nodiscard]] constexpr const_iterator cbegin() const noexcept { return begin(); }
		/** Returns iterator one past the last element of the set. */
		[[nodiscard]] constexpr const_iterator end() const noexcept { return m_table.end(); }
		/** @copydoc end */
		[[nodiscard]] constexpr const_iterator cend() const noexcept { return end(); }

		/** Returns reference to the first element of the set. */
		[[nodiscard]] constexpr const_reference front() const noexcept { return m_table.front(); }
		/** Returns reference to the last element of the set. */
		[[nodiscard]] constexpr const_reference back() const noexcept { return m_table.back(); }

		/** Returns the total number of elements within the set. */
		[[nodiscard]] constexpr size_type size() const noexcept { return m_table.size(); }
		/** Checks if the set is empty (`size() == 0`). */
		[[nodiscard]] constexpr bool empty() const noexcept { return m_table.size() == 0; }
		/** Returns the current capacity of the set (taking into account the maximum load factor). */
		[[nodiscard]] constexpr size_type capacity() const noexcept { return m_table.capacity(); }
		/** Returns the absolute maximum size of the set (taking into account the maximum load factor). */
		[[nodiscard]] constexpr size_type max_size() const noexcept { return m_table.max_size(); }
		/** Returns the current load factor of the set as if via `size() / bucket_count()`. */
		[[nodiscard]] constexpr float load_factor() const noexcept { return m_table.load_factor(); }

		/** Erases all elements from the set. */
		void clear() { m_table.clear(); }

		/** @brief Inserts an element (of `value_type`) into the set if it does not exist yet.
		 * @param value Value of the to-be inserted element.
		 * @return Pair where `first` is the iterator to the inserted or existing element, and `second` is a boolean
		 * indicating whether insertion took place (`true` if element was inserted, `false` otherwise). */
		std::pair<iterator, bool> insert(const insert_type &value) { return m_table.insert(value); }
		/** @copydoc insert */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		std::pair<iterator, bool> insert(const T &value) { return m_table.insert(value); }
		/** @copydoc insert */
		std::pair<iterator, bool> insert(insert_type &&value) { return m_table.insert(std::forward<insert_type>(value)); }
		/** @copydoc insert */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, T &&>>>
		std::pair<iterator, bool> insert(T &&value) { return m_table.insert(std::forward<T>(value)); }
		/** @copybrief insert
		 * @param hint New position of the inserted element.
		 * @param value Value of the to-be inserted element.
		 * @return Iterator to the inserted or existing element. */
		iterator insert(const_iterator hint, const insert_type &value) { return m_table.insert(hint, value); }
		/** @copydoc insert */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		iterator insert(const_iterator hint, const T &value) { return m_table.insert(hint, value); }
		/** @copydoc insert */
		iterator insert(const_iterator hint, insert_type &&value) { return m_table.insert(hint, std::forward<insert_type>(value)); }
		/** @copydoc insert */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, T &&>>>
		iterator insert(const_iterator hint, T &&value) { return m_table.insert(hint, std::forward<T>(value)); }

		/** Inserts all elements from the range `[first, last)` into the set.
		 * @param first Iterator to the first element of the source range.
		 * @param last Iterator one past the last element of the source range. */
		template<typename I>
		void insert(I first, I last) { return m_table.insert(first, last); }
		/** Inserts all elements of an initializer list into the set. */
		void insert(std::initializer_list<value_type> il) { return insert(il.begin(), il.end()); }
		/** @copydoc insert */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		void insert(std::initializer_list<T> il) { return insert(il.begin(), il.end()); }

		/** @brief Inserts an in-place constructed element (of `value_type`) into the set if it does not exist yet.
		 * @param args Arguments passed to constructor of `value_type`.
		 * @return Pair where `first` is the iterator to the inserted or existing element, and `second` is a boolean
		 * indicating whether insertion took place (`true` if element was inserted, `false` otherwise). */
		template<typename... Args>
		std::pair<iterator, bool> emplace(Args &&...args) { return m_table.emplace(std::forward<Args>(args)...); }
		/** @copybrief emplace
		 * @param hint New position of the inserted element.
		 * @param args Arguments passed to constructor of `value_type`.
		 * @return Iterator to the inserted or existing element. */
		template<typename... Args>
		iterator emplace_hint(const_iterator hint, Args &&...args) { return m_table.emplace_hint(hint, std::forward<Args>(args)...); }

		/** Removes the specified element from the set.
		 * @param key Key of the element to remove.
		 * @return Iterator to the element following the erased one, or `end()`. */
		iterator erase(const key_type &key) { return m_table.erase(key); }
		/** @copydoc erase
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		iterator erase(const K &key) { return m_table.erase(key); }

		/** Removes the specified element from the set.
		 * @param pos Iterator pointing to the element to remove.
		 * @return Iterator to the element following the erased one, or `end()`. */
		iterator erase(const_iterator pos) { return m_table.erase(pos); }

		/** Removes a range of elements from the set.
		 * @param first Iterator to the first element of the to-be removed range.
		 * @param last Iterator one past the last element of the to-be removed range.
		 * @return Iterator to the element following the erased range, or `end()`. */
		iterator erase(const_iterator first, const_iterator last) { return m_table.erase(first, last); }

		/** Searches for the specified element within the set.
		 * @param key Key of the element to search for.
		 * @return Iterator to the specified element, or `end()`. */
		[[nodiscard]] iterator find(const key_type &key) const { return m_table.find(key); }
		/** @copydoc find
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] iterator find(const K &key) const { return m_table.find(key); }

		/** Checks if the specified element is present within the set as if by `find(key) != end()`.
		 * @param key Key of the element to search for.
		 * @return `true` if the element is present within the set, `false` otherwise. */
		[[nodiscard]] bool contains(const key_type &key) const { return m_table.contains(key); }
		/** @copydoc contains
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] bool contains(const K &key) const { return m_table.contains(key); }

		/** Returns the current amount of buckets of the set. */
		[[nodiscard]] constexpr size_type bucket_count() const noexcept { return m_table.bucket_count(); }
		/** Returns the maximum amount of buckets of the set. */
		[[nodiscard]] constexpr size_type max_bucket_count() const noexcept { return m_table.max_bucket_count(); }

		/** Reserves space for at least `n` elements. */
		void reserve(size_type n) { m_table.reserve(n); }
		/** Reserves space for at least `n` buckets and rehashes the set if necessary.
		 * @note The new amount of buckets is clamped to be at least `size() / max_load_factor()`. */
		void rehash(size_type n) { m_table.rehash(n); }
		/** Returns the maximum load factor. */
		[[nodiscard]] constexpr float max_load_factor() const noexcept { return m_table.max_load_factor(); }

		[[nodiscard]] allocator_type get_allocator() const { return allocator_type{m_table.get_allocator()}; }
		[[nodiscard]] hasher hash_function() const { return m_table.get_hash(); }
		[[nodiscard]] key_equal key_eq() const { return m_table.get_cmp(); }

		[[nodiscard]] bool operator==(const ordered_sparse_set &other) const
		{
			return std::is_permutation(begin(), end(), other.begin(), other.end());
		}
#if (__cplusplus < 202002L && (!defined(_MSVC_LANG) || _MSVC_LANG < 202002L))
		[[nodiscard]] bool operator!=(const ordered_sparse_set &other) const
		{
			return !std::is_permutation(begin(), end(), other.begin(), other.end());
		}
#endif

		void swap(ordered_sparse_set &other) noexcept(std::is_nothrow_swappable_v<table_t>) { m_table.swap(other.m_table); }

	private:
		table_t m_table;
	};

	/** Erases all elements from the set \p set that satisfy the predicate \p pred.
	 * @return Amount of elements erased. */
	template<typename K, typename H, typename C, typename A, typename P>
	inline typename ordered_sparse_set<K, H, C, A>::size_type erase_if(ordered_sparse_set<K, H, C, A> &set, P pred)
	{
		typename sparse_set<K, H, C, A>::size_type result = 0;
		for (auto i = set.cbegin(), last = set.cend(); i != last;)
		{
			if (pred(*i))
			{
				i = set.erase(i);
				++result;
			}
			else
				++i;
		}
		return result;
	}

	template<typename K, typename H, typename C, typename A>
	inline void swap(ordered_sparse_set<K, H, C, A> &a, ordered_sparse_set<K, H, C, A> &b) noexcept(std::is_nothrow_swappable_v<ordered_sparse_set<K, H, C, A>>) { a.swap(b); }

	template<typename I, typename Hash = detail::default_hash<detail::iter_key_t<I>>, typename Cmp = std::equal_to<detail::iter_key_t<I>>, typename Alloc = std::allocator<detail::iter_key_t<I>>>
	ordered_sparse_set(I, I, typename detail::deduce_set_t<ordered_sparse_set, I, Hash, Cmp, Alloc>::size_type = 0, Hash = Hash{}, Cmp = Cmp{}, Alloc = Alloc{})
	-> ordered_sparse_set<detail::iter_key_t<I>, Hash, Cmp, Alloc>;
	template<typename I, typename Hash, typename Alloc>
	ordered_sparse_set(I, I, typename detail::deduce_set_t<ordered_sparse_set, I, Hash, std::equal_to<detail::iter_key_t<I>>, Alloc>::size_type, Hash, Alloc)
	-> ordered_sparse_set<detail::iter_key_t<I>, Hash, std::equal_to<detail::iter_key_t<I>>, Alloc>;
	template<typename I, typename Alloc>
	ordered_sparse_set(I, I, typename detail::deduce_set_t<ordered_sparse_set, I, detail::default_hash<detail::iter_key_t<I>>, std::equal_to<detail::iter_key_t<I>>, Alloc>::size_type, Alloc)
	-> ordered_sparse_set<detail::iter_key_t<I>, detail::default_hash<detail::iter_key_t<I>>, std::equal_to<detail::iter_key_t<I>>, Alloc>;
	template<typename I, typename Alloc>
	ordered_sparse_set(I, I, Alloc) -> ordered_sparse_set<detail::iter_key_t<I>, detail::default_hash<detail::iter_key_t<I>>, std::equal_to<detail::iter_key_t<I>>, Alloc>;

	template<typename K, typename Hash = detail::default_hash<K>, typename Cmp = std::equal_to<K>, typename Alloc = std::allocator<K>>
	ordered_sparse_set(std::initializer_list<K>, typename ordered_sparse_set<K, Hash, Cmp, Alloc>::size_type = 0, Hash = Hash{}, Cmp = Cmp{}, Alloc = Alloc{}) -> ordered_sparse_set<K, Hash, Cmp, Alloc>;
	template<typename K, typename Hash, typename Alloc>
	ordered_sparse_set(std::initializer_list<K>, typename ordered_sparse_set<K, Hash, std::equal_to<K>, Alloc>::size_type, Hash, Alloc) -> ordered_sparse_set<K, Hash, std::equal_to<K>, Alloc>;
	template<typename K, typename Alloc>
	ordered_sparse_set(std::initializer_list<K>, typename ordered_sparse_set<K, detail::default_hash<K>, std::equal_to<K>, Alloc>::size_type, Alloc) -> ordered_sparse_set<K, detail::default_hash<K>, std::equal_to<K>, Alloc>;
	template<typename K, typename Alloc>
	ordered_sparse_set(std::initializer_list<K>, Alloc) -> ordered_sparse_set<K, detail::default_hash<K>, std::equal_to<K>, Alloc>;
}