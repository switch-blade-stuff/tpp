/*
 * Created by switchblade on 11/5/22.
 */

#pragma once

#include "detail/dense_table.hpp"
#include "stl_hash.hpp"

namespace tpp
{
	/** @brief Hash set based on dense hash table.
	 *
	 * Internally, dense set stores it's elements in a contiguous unordered vector.
	 * Insert and erase operations on a dense set may invalidate references to it's
	 * elements due to the internal element vector being reordered.
	 *
	 * @tparam Key Key type stored by the set.
	 * @tparam KeyHash Hash functor used by the set.
	 * @tparam KeyCmp Compare functor used by the set.
	 * @tparam Alloc Allocator used by the set. */
	template<typename Key, typename KeyHash = detail::default_hash<Key>, typename KeyCmp = std::equal_to<Key>, typename Alloc = std::allocator<Key>>
	class dense_set
	{
		using table_t = detail::dense_table<Key, Key, KeyHash, KeyCmp, detail::key_identity, Alloc>;

	public:
		typedef typename table_t::value_type value_type;
		typedef typename table_t::key_type key_type;

		typedef value_type &reference;
		typedef const value_type &const_reference;
		typedef value_type *pointer;
		typedef const value_type *const_pointer;

		typedef typename table_t::const_iterator iterator;
		typedef typename table_t::const_iterator const_iterator;
		typedef typename table_t::const_reverse_iterator reverse_iterator;
		typedef typename table_t::const_reverse_iterator const_reverse_iterator;
		typedef typename table_t::const_local_iterator local_iterator;
		typedef typename table_t::const_local_iterator const_local_iterator;

		typedef typename table_t::size_type size_type;
		typedef typename table_t::difference_type difference_type;

		typedef typename table_t::hasher hasher;
		typedef typename table_t::key_equal key_equal;
		typedef typename table_t::allocator_type allocator_type;

	private:
		constexpr dense_set() = default;

		/** Returns iterator to the first element of the set.
		 * @note Elements are stored in no particular order. */
		[[nodiscard]] constexpr const_iterator begin() const noexcept { return m_table.begin(); }
		/** @copydoc end */
		[[nodiscard]] constexpr const_iterator cbegin() const noexcept { return begin(); }
		/** Returns iterator one past the last element of the set.
		 * @note Elements are stored in no particular order. */
		[[nodiscard]] constexpr const_iterator end() const noexcept { return m_table.end(); }
		/** @copydoc end */
		[[nodiscard]] constexpr const_iterator cend() const noexcept { return end(); }

		/** Returns reverse iterator to the last element of the set.
		 * @note Elements are stored in no particular order. */
		[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return m_table.rbegin(); }
		/** @copydoc rbegin */
		[[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
		/** Returns reverse iterator one past the first element of the set.
		 * @note Elements are stored in no particular order. */
		[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return m_table.rend(); }
		/** @copydoc rend */
		[[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return rend(); }

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
		constexpr void clear() { m_table.clear(); }

		/** @brief Inserts an element (of `value_type`) into the set if it does not exist yet, or returns iterator to an existing element.
		 * @param value Value of the to-be inserted element.
		 * @return Pair where `first` is the iterator to the inserted or existing element, and `second` is a boolean
		 * indicating whether insertion took place (`true` if element was inserted, `false` otherwise). */
		constexpr std::pair<iterator, bool> insert(const value_type &value) { return m_table.insert(value); }
		/** @copydoc insert */
		constexpr std::pair<iterator, bool> insert(value_type &&value) { return m_table.insert(std::forward<value_type>(value)); }
		/** @copybrief insert
		 * @param value Value of the to-be inserted element.
		 * @return Iterator to the inserted or existing element.
		 * @note \p hint has no effect, this overload exists for API compatibility. */
		constexpr iterator insert(const_iterator hint, const value_type &value) { return m_table.insert(hint, value); }
		/** @copydoc insert */
		constexpr iterator insert(const_iterator hint, value_type &&value) { return m_table.insert(hint, std::forward<value_type>(value)); }

		/** Inserts all elements from the range `[first, last)` into the set.
		 * @param first Iterator to the first element of the source range.
		 * @param last Iterator one past the last element of the source range. */
		template<typename I>
		constexpr void insert(I first, I last) { return m_table.insert(first, last); }
		/** Inserts all elements of an initializer list into the set. */
		constexpr void insert(std::initializer_list<value_type> il) { return insert(il.begin(), il.end()); }

		/** @brief Inserts an in-place constructed element (of `value_type`) into the set if it does not exist yet, or returns iterator to an existing element.
		 * @param args Arguments passed to constructor of `value_type`.
		 * @return Pair where `first` is the iterator to the inserted or existing element, and `second` is a boolean
		 * indicating whether insertion took place (`true` if element was inserted, `false` otherwise). */
		template<typename... Args>
		constexpr std::pair<iterator, bool> emplace(Args &&...args) { return m_table.emplace(std::forward<Args>(args)...); }
		/** @copybrief emplace
		 * @param args Arguments passed to constructor of `value_type`.
		 * @return Iterator to the inserted or existing element.
		 * @note \p hint has no effect, this overload exists for API compatibility. */
		template<typename... Args>
		constexpr iterator emplace_hint(const_iterator hint, Args &&...args) { return m_table.emplace_hint(hint, std::forward<Args>(args)...); }

		/** Removes the specified element from the set.
		 * @param key Key of the element to remove.
		 * @return Iterator to the element following the erased one, or `end()`. */
		constexpr iterator erase(const key_type &key) { return m_table.erase(key); }
		/** @copydoc erase
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value>>
		constexpr iterator erase(const K &key) { return m_table.erase(key); }
		/** Removes the specified element from the set.
		 * @param pos Iterator pointing to the element to remove.
		 * @return Iterator to the element following the erased one, or `end()`. */
		constexpr iterator erase(const_iterator pos) { return m_table.erase(pos); }
		/** Removes a range of elements from the set.
		 * @param first Iterator to the first element of the to-be removed range.
		 * @param last Iterator one past the last element of the to-be removed range.
		 * @return Iterator to the element following the erased range, or `end()`. */
		constexpr iterator erase(const_iterator first, const_iterator last) { return m_table.erase(first, last); }

		/** Searches for the specified element within the set.
		 * @param key Key of the element to search for.
		 * @return Iterator to the found element, or `end()`. */
		constexpr iterator find(const key_type &key) const { return m_table.find(key); }
		/** @copydoc find
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value>>
		constexpr iterator find(const K &key) const { return m_table.find(key); }
		/** Checks if the specified element is present within the set as if by `find(key) != end()`.
		 * @param key Key of the element to search for.
		 * @return `true` if the element is present within the set, `false` otherwise. */
		constexpr bool contains(const key_type &key) const { return m_table.contains(key); }
		/** @copydoc contains
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value>>
		constexpr bool contains(const K &key) const { return m_table.contains(key); }

		/** Returns forward iterator to the first element of the specified bucket. */
		[[nodiscard]] constexpr const_local_iterator begin(size_type n) const noexcept { return m_table.begin(n); }
		/** @copydoc begin */
		[[nodiscard]] constexpr const_local_iterator cbegin(size_type n) const noexcept { return m_table.begin(n); }
		/** Returns a sentinel iterator for the specified bucket. */
		[[nodiscard]] constexpr const_local_iterator end(size_type n) const noexcept { return m_table.end(n); }
		/** @copydoc end */
		[[nodiscard]] constexpr const_local_iterator cend(size_type n) const noexcept { return m_table.end(n); }

		/** Returns the bucket index of the specified element. */
		[[nodiscard]] constexpr size_type bucket(const key_type &key) const { return m_table.bucket(key); }
		/** @copydoc bucket
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value>>
		[[nodiscard]] constexpr size_type bucket(const K &key) const { return m_table.bucket(key); }

		/** Returns the current amount of buckets of the set. */
		[[nodiscard]] constexpr size_type bucket_count() const noexcept { return m_table.bucket_count(); }
		/** Returns the maximum amount of buckets of the set. */
		[[nodiscard]] constexpr size_type max_bucket_count() const noexcept { return m_table.max_bucket_count(); }
		/** Returns the amount of elements within the specified bucket. */
		[[nodiscard]] constexpr size_type bucket_size(size_type n) const noexcept { return m_table.bucket_size(n); }

		/** Reserves space for at least `n` elements. */
		constexpr void reserve(size_type n) { m_table.reserve(n); }
		/** Reserves space for at least `n` buckets and rehashes the set if necessary.
		 * @note The new amount of buckets is clamped to be at least `size() / max_load_factor()`. */
		constexpr void rehash(size_type n) { m_table.rehash(n); }
		/** Returns the current maximum load factor. */
		[[nodiscard]] constexpr float max_load_factor() const noexcept { return m_table.max_load_factor; }
		/** Sets the current maximum load factor. */
		constexpr void max_load_factor(float f) const noexcept { m_table.max_load_factor = f; }

		[[nodiscard]] constexpr allocator_type get_allocator() const { return allocator_type{m_table.get_allocator()}; }
		[[nodiscard]] constexpr hasher hash_function() const { return m_table.get_hash(); }
		[[nodiscard]] constexpr key_equal key_eq() const { return m_table.get_cmp(); }

		[[nodiscard]] constexpr bool operator==(const dense_set &other) const { return std::is_permutation(begin(), end(), other.begin(), other.end()); }
#if __cplusplus < 202002L
		[[nodiscard]] constexpr bool operator!=(const dense_set &other) const { return !std::is_permutation(begin(), end(), other.begin(), other.end()); }
#endif

		constexpr void swap(dense_set &other) noexcept(std::is_nothrow_swappable_v<table_t>) { m_table.swap(other.m_table); }

	private:
		table_t m_table;
	};

	/** Erases all elements from the set \p set that satisfy the predicate \p pred.
	 * @return Amount of elements erased. */
	template<typename K, typename H, typename C, typename A, typename P>
	constexpr typename dense_set<K, H, C, A>::size_type erase_if(dense_set<K, H, C, A> &set, P pred)
	{
		typename dense_set<K, H, C, A>::size_type result = 0;
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
	constexpr void swap(dense_set<K, H, C, A> &a, dense_set<K, H, C, A> &b) noexcept(std::is_nothrow_swappable_v<dense_set<K, H, C, A>>) { a.swap(b); }

	/** @brief Ordered hash set based on dense hash table.
	 *
	 * Internally, ordered dense set stores it's elements in a contiguous vector with additional ordering
	 * links between elements. Insert and erase operations on a dense set may invalidate references to it's
	 * elements due to the internal element vector being reordered.
	 *
	 * @tparam Key Key type stored by the set.
	 * @tparam KeyHash Hash functor used by the set.
	 * @tparam KeyCmp Compare functor used by the set.
	 * @tparam Alloc Allocator used by the set. */
	template<typename Key, typename KeyHash = detail::default_hash<Key>, typename KeyCmp = std::equal_to<Key>, typename Alloc = std::allocator<Key>>
	class ordered_dense_set
	{
		using table_t = detail::dense_table<Key, Key, KeyHash, KeyCmp, detail::key_identity, Alloc, detail::ordered_link>;

	public:
		typedef typename table_t::value_type value_type;
		typedef typename table_t::key_type key_type;

		typedef value_type &reference;
		typedef const value_type &const_reference;
		typedef value_type *pointer;
		typedef const value_type *const_pointer;

		typedef typename table_t::const_iterator iterator;
		typedef typename table_t::const_iterator const_iterator;
		typedef typename table_t::const_reverse_iterator reverse_iterator;
		typedef typename table_t::const_reverse_iterator const_reverse_iterator;
		typedef typename table_t::const_local_iterator local_iterator;
		typedef typename table_t::const_local_iterator const_local_iterator;

		typedef typename table_t::size_type size_type;
		typedef typename table_t::difference_type difference_type;

		typedef typename table_t::hasher hasher;
		typedef typename table_t::key_equal key_equal;
		typedef typename table_t::allocator_type allocator_type;

	private:
		constexpr ordered_dense_set() = default;

		/** Returns iterator to the first element of the set. */
		[[nodiscard]] constexpr const_iterator begin() const noexcept { return m_table.begin(); }
		/** @copydoc end */
		[[nodiscard]] constexpr const_iterator cbegin() const noexcept { return begin(); }
		/** Returns iterator one past the last element of the set. */
		[[nodiscard]] constexpr const_iterator end() const noexcept { return m_table.end(); }
		/** @copydoc end */
		[[nodiscard]] constexpr const_iterator cend() const noexcept { return end(); }

		/** Returns reverse iterator to the last element of the set. */
		[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return m_table.rbegin(); }
		/** @copydoc rbegin */
		[[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
		/** Returns reverse iterator one past the first element of the set. */
		[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return m_table.rend(); }
		/** @copydoc rend */
		[[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return rend(); }

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
		constexpr void clear() { m_table.clear(); }

		/** @brief Inserts an element (of `value_type`) into the set if it does not exist yet, or returns iterator to an existing element.
		 * @param value Value of the to-be inserted element.
		 * @return Pair where `first` is the iterator to the inserted or existing element, and `second` is a boolean
		 * indicating whether insertion took place (`true` if element was inserted, `false` otherwise). */
		constexpr std::pair<iterator, bool> insert(const value_type &value) { return m_table.insert(value); }
		/** @copydoc insert */
		constexpr std::pair<iterator, bool> insert(value_type &&value) { return m_table.insert(std::forward<value_type>(value)); }
		/** @copybrief insert
		 * @param value Value of the to-be inserted element.
		 * @return Iterator to the inserted or existing element.
		 * @note \p hint has no effect, this overload exists for API compatibility. */
		constexpr iterator insert(const_iterator hint, const value_type &value) { return m_table.insert(hint, value); }
		/** @copydoc insert */
		constexpr iterator insert(const_iterator hint, value_type &&value) { return m_table.insert(hint, std::forward<value_type>(value)); }

		/** Inserts all elements from the range `[first, last)` into the set.
		 * @param first Iterator to the first element of the source range.
		 * @param last Iterator one past the last element of the source range. */
		template<typename I>
		constexpr void insert(I first, I last) { return m_table.insert(first, last); }
		/** Inserts all elements of an initializer list into the set. */
		constexpr void insert(std::initializer_list<value_type> il) { return insert(il.begin(), il.end()); }

		/** @brief Inserts an in-place constructed element (of `value_type`) into the set if it does not exist yet, or returns iterator to an existing element.
		 * @param args Arguments passed to constructor of `value_type`.
		 * @return Pair where `first` is the iterator to the inserted or existing element, and `second` is a boolean
		 * indicating whether insertion took place (`true` if element was inserted, `false` otherwise). */
		template<typename... Args>
		constexpr std::pair<iterator, bool> emplace(Args &&...args) { return m_table.emplace(std::forward<Args>(args)...); }
		/** @copybrief emplace
		 * @param args Arguments passed to constructor of `value_type`.
		 * @return Iterator to the inserted or existing element.
		 * @note \p hint has no effect, this overload exists for API compatibility. */
		template<typename... Args>
		constexpr iterator emplace_hint(const_iterator hint, Args &&...args) { return m_table.emplace_hint(hint, std::forward<Args>(args)...); }

		/** Removes the specified element from the set.
		 * @param key Key of the element to remove.
		 * @return Iterator to the element following the erased one, or `end()`. */
		constexpr iterator erase(const key_type &key) { return m_table.erase(key); }
		/** @copydoc erase
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value>>
		constexpr iterator erase(const K &key) { return m_table.erase(key); }
		/** Removes the specified element from the set.
		 * @param pos Iterator pointing to the element to remove.
		 * @return Iterator to the element following the erased one, or `end()`. */
		constexpr iterator erase(const_iterator pos) { return m_table.erase(pos); }
		/** Removes a range of elements from the set.
		 * @param first Iterator to the first element of the to-be removed range.
		 * @param last Iterator one past the last element of the to-be removed range.
		 * @return Iterator to the element following the erased range, or `end()`. */
		constexpr iterator erase(const_iterator first, const_iterator last) { return m_table.erase(first, last); }

		/** Searches for the specified element within the set.
		 * @param key Key of the element to search for.
		 * @return Iterator to the found element, or `end()`. */
		constexpr iterator find(const key_type &key) const { return m_table.find(key); }
		/** @copydoc find
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value>>
		constexpr iterator find(const K &key) const { return m_table.find(key); }
		/** Checks if the specified element is present within the set as if by `find(key) != end()`.
		 * @param key Key of the element to search for.
		 * @return `true` if the element is present within the set, `false` otherwise. */
		constexpr bool contains(const key_type &key) const { return m_table.contains(key); }
		/** @copydoc contains
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value>>
		constexpr bool contains(const K &key) const { return m_table.contains(key); }

		/** Returns forward iterator to the first element of the specified bucket. */
		[[nodiscard]] constexpr const_local_iterator begin(size_type n) const noexcept { return m_table.begin(n); }
		/** @copydoc begin */
		[[nodiscard]] constexpr const_local_iterator cbegin(size_type n) const noexcept { return m_table.begin(n); }
		/** Returns a sentinel iterator for the specified bucket. */
		[[nodiscard]] constexpr const_local_iterator end(size_type n) const noexcept { return m_table.end(n); }
		/** @copydoc end */
		[[nodiscard]] constexpr const_local_iterator cend(size_type n) const noexcept { return m_table.end(n); }

		/** Returns the bucket index of the specified element. */
		[[nodiscard]] constexpr size_type bucket(const key_type &key) const { return m_table.bucket(key); }
		/** @copydoc bucket
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value>>
		[[nodiscard]] constexpr size_type bucket(const K &key) const { return m_table.bucket(key); }

		/** Returns the current amount of buckets of the set. */
		[[nodiscard]] constexpr size_type bucket_count() const noexcept { return m_table.bucket_count(); }
		/** Returns the maximum amount of buckets of the set. */
		[[nodiscard]] constexpr size_type max_bucket_count() const noexcept { return m_table.max_bucket_count(); }
		/** Returns the amount of elements within the specified bucket. */
		[[nodiscard]] constexpr size_type bucket_size(size_type n) const noexcept { return m_table.bucket_size(n); }

		/** Reserves space for at least `n` elements. */
		constexpr void reserve(size_type n) { m_table.reserve(n); }
		/** Reserves space for at least `n` buckets and rehashes the set if necessary.
		 * @note The new amount of buckets is clamped to be at least `size() / max_load_factor()`. */
		constexpr void rehash(size_type n) { m_table.rehash(n); }
		/** Returns the current maximum load factor. */
		[[nodiscard]] constexpr float max_load_factor() const noexcept { return m_table.max_load_factor; }
		/** Sets the current maximum load factor. */
		constexpr void max_load_factor(float f) const noexcept { m_table.max_load_factor = f; }

		[[nodiscard]] constexpr allocator_type get_allocator() const { return allocator_type{m_table.get_allocator()}; }
		[[nodiscard]] constexpr hasher hash_function() const { return m_table.get_hash(); }
		[[nodiscard]] constexpr key_equal key_eq() const { return m_table.get_cmp(); }

		[[nodiscard]] constexpr bool operator==(const ordered_dense_set &other) const
		{
			return std::is_permutation(begin(), end(), other.begin(), other.end());
		}
#if __cplusplus < 202002L
		[[nodiscard]] constexpr bool operator!=(const ordered_dense_set &other) const
		{
			return !std::is_permutation(begin(), end(), other.begin(), other.end());
		}
#endif

		constexpr void swap(ordered_dense_set &other) noexcept(std::is_nothrow_swappable_v<table_t>) { m_table.swap(other.m_table); }

	private:
		table_t m_table;
	};

	/** Erases all elements from the set \p set that satisfy the predicate \p pred.
	 * @return Amount of elements erased. */
	template<typename K, typename H, typename C, typename A, typename P>
	constexpr typename ordered_dense_set<K, H, C, A>::size_type erase_if(ordered_dense_set<K, H, C, A> &set, P pred)
	{
		typename dense_set<K, H, C, A>::size_type result = 0;
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
	constexpr void swap(ordered_dense_set<K, H, C, A> &a, ordered_dense_set<K, H, C, A> &b) noexcept(std::is_nothrow_swappable_v<ordered_dense_set<K, H, C, A>>)
	{
		a.swap(b);
	}
}