/*
 * Created by switchblade on 11/5/22.
 */

#pragma once

#include "detail/dense_table.hpp"
#include "detail/multikey.hpp"

namespace tpp
{
	template<typename Mk, typename = detail::multikey_hash<Mk>, typename = detail::multikey_eq<Mk>, typename = detail::multikey_alloc_t<Mk>>
	class dense_multiset;

	/** @brief Hash multiset based on dense hash table.
	 *
	 * Multiset is a one-to-one hashtable container that allows the user to associate an entry with multiple independent keys.
	 * Internally, dense multiset stores it's elements in a contiguous unordered vector. Insert and erase operations on a
	 * dense multiset may invalidate references to it's elements due to the internal element vector being reordered.
	 *
	 * @tparam Keys Key types of the multiset.
	 * @tparam KeyHash Hash functor used by the multiset. The functor must be invocable for both all types.
	 * @tparam KeyCmp Compare functor used by the multiset. The functor must be invocable for both all types.
	 * @tparam Alloc Allocator used by the multiset. */
	template<typename... Keys, typename KeyHash, typename KeyCmp, typename Alloc>
	class dense_multiset<multikey<Keys...>, KeyHash, KeyCmp, Alloc>
	{
	public:
		using key_type = std::tuple<Keys...>;

	private:
		struct traits_t
		{
			using link_type = detail::empty_link;

			using pointer = const key_type *;
			using const_pointer = const key_type *;
			using reference = const key_type &;
			using const_reference = const key_type &;

			template<std::size_t I, typename T>
			constexpr static auto &key_get(T &value) noexcept { return std::get<I>(value); }
			template<typename T>
			constexpr static auto &key_get(T &value) noexcept { return value; }

			constexpr static std::size_t key_size = std::tuple_size_v<key_type>;
		};

		using table_t = detail::dense_table<key_type, key_type, KeyHash, KeyCmp, Alloc, traits_t>;

	public:
		using insert_type = typename table_t::insert_type;
		using value_type = typename table_t::value_type;

		using reference = typename table_t::reference;
		using const_reference = typename table_t::const_reference;
		using pointer = typename table_t::pointer;
		using const_pointer = typename table_t::const_pointer;

		using iterator = typename table_t::const_iterator;
		using const_iterator = typename table_t::const_iterator;
		using reverse_iterator = typename table_t::const_reverse_iterator;
		using const_reverse_iterator = typename table_t::const_reverse_iterator;
		using local_iterator = typename table_t::const_local_iterator;
		using const_local_iterator = typename table_t::const_local_iterator;

		using size_type = typename table_t::size_type;
		using difference_type = typename table_t::difference_type;

		using hasher = typename table_t::hasher;
		using key_equal = typename table_t::key_equal;
		using allocator_type = typename table_t::allocator_type;

		constexpr static auto key_size = table_t::key_size;

	public:
		/** Initializes the multiset with default capacity. */
		TPP_CXX20_CONSTEXPR dense_multiset() = default;
		/** Initializes the multiset with default capacity using the specified allocator. */
		TPP_CXX20_CONSTEXPR explicit dense_multiset(const allocator_type &alloc) : m_table(alloc) {}

		/** Copy-constructs the multiset. */
		TPP_CXX20_CONSTEXPR dense_multiset(const dense_multiset &other) = default;
		/** Copy-constructs the multiset using the specified allocator. */
		TPP_CXX20_CONSTEXPR dense_multiset(const dense_multiset &other, const allocator_type &alloc) : m_table(other.m_table, alloc) {}

		/** Move-constructs the multiset. */
		TPP_CXX20_CONSTEXPR dense_multiset(dense_multiset &&other) noexcept(std::is_nothrow_move_constructible_v<table_t>) = default;
		/** Move-constructs the multiset using the specified allocator. */
		TPP_CXX20_CONSTEXPR dense_multiset(dense_multiset &&other, const allocator_type &alloc)
		noexcept(std::is_nothrow_constructible_v<table_t, table_t &&, allocator_type>) : m_table(std::move(other.m_table), alloc) {}

		/** Initializes the multiset with the specified bucket count, hasher, comparator and allocator. */
		TPP_CXX20_CONSTEXPR explicit dense_multiset(size_type bucket_count, const hasher &hash = hasher{}, const key_equal &cmp = key_equal{},
		                                            const allocator_type &alloc = allocator_type{})
				: m_table(bucket_count, hash, cmp, alloc) {}
		/** Initializes the multiset with the specified bucket count, hasher and allocator. */
		TPP_CXX20_CONSTEXPR dense_multiset(size_type bucket_count, const hasher &hash, const allocator_type &alloc)
				: dense_multiset(bucket_count, hash, key_equal{}, alloc) {}
		/** Initializes the multiset with the specified bucket count and allocator. */
		TPP_CXX20_CONSTEXPR dense_multiset(size_type bucket_count, const allocator_type &alloc)
				: dense_multiset(bucket_count, hasher{}, alloc) {}

		/** Initializes the multiset with an initializer list of elements and the specified bucket count, hasher, comparator and allocator. */
		TPP_CXX20_CONSTEXPR dense_multiset(std::initializer_list<value_type> il, size_type bucket_count = table_t::initial_capacity,
		                                   const hasher &hash = hasher{}, const key_equal &cmp = key_equal{},
		                                   const allocator_type &alloc = allocator_type{})
				: dense_multiset(il.begin(), il.end(), bucket_count, hash, cmp, alloc) {}
		/** @copydoc dense_set */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		TPP_CXX20_CONSTEXPR dense_multiset(std::initializer_list<T> il, size_type bucket_count = table_t::initial_capacity, const hasher &hash = hasher{},
		                                   const key_equal &cmp = key_equal{}, const allocator_type &alloc = allocator_type{})
				: dense_multiset(il.begin(), il.end(), bucket_count, hash, cmp, alloc) {}

		/** Initializes the multiset with an initializer list of elements and the specified bucket count, hasher and allocator. */
		TPP_CXX20_CONSTEXPR dense_multiset(std::initializer_list<value_type> il, size_type bucket_count, const hasher &hash, const allocator_type &alloc)
				: dense_multiset(il.begin(), il.end(), bucket_count, hash, key_equal{}, alloc) {}
		/** @copydoc dense_set */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		TPP_CXX20_CONSTEXPR dense_multiset(std::initializer_list<T> il, size_type bucket_count, const hasher &hash, const allocator_type &alloc)
				: dense_multiset(il.begin(), il.end(), bucket_count, hash, key_equal{}, alloc) {}

		/** Initializes the multiset with an initializer list of elements and the specified bucket count and allocator. */
		TPP_CXX20_CONSTEXPR dense_multiset(std::initializer_list<value_type> il, size_type bucket_count, const allocator_type &alloc)
				: dense_multiset(il.begin(), il.end(), bucket_count, hasher{}, alloc) {}
		/** @copydoc dense_set */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		TPP_CXX20_CONSTEXPR dense_multiset(std::initializer_list<T> il, size_type bucket_count, const allocator_type &alloc)
				: dense_multiset(il.begin(), il.end(), bucket_count, hasher{}, alloc) {}

		/** Initializes the multiset with a range of elements and the specified bucket count, hasher, comparator and allocator. */
		template<typename I>
		TPP_CXX20_CONSTEXPR dense_multiset(I first, I last, size_type bucket_count = table_t::initial_capacity, const hasher &hash = hasher{},
		                                   const key_equal &cmp = key_equal{}, const allocator_type &alloc = allocator_type{})
				: m_table(first, last, bucket_count, hash, cmp, alloc) {}
		/** Initializes the multiset with a range of elements and the specified bucket count, hasher and allocator. */
		template<typename I>
		TPP_CXX20_CONSTEXPR dense_multiset(I first, I last, size_type bucket_count, const hasher &hash, const allocator_type &alloc)
				: dense_multiset(first, last, bucket_count, hash, key_equal{}, alloc) {}
		/** Initializes the multiset with a range of elements and the specified bucket count and allocator. */
		template<typename I>
		TPP_CXX20_CONSTEXPR dense_multiset(I first, I last, size_type bucket_count, const allocator_type &alloc)
				: dense_multiset(first, last, bucket_count, hasher{}, alloc) {}

		/** Copy-assigns the multiset. */
		TPP_CXX20_CONSTEXPR dense_multiset &operator=(const dense_multiset &) = default;
		/** Move-assigns the multiset. */
		TPP_CXX20_CONSTEXPR dense_multiset &operator=(dense_multiset &&) noexcept(std::is_nothrow_move_assignable_v<table_t>) = default;

		/** Replaces elements of the multiset with elements of the initializer list. */
		TPP_CXX20_CONSTEXPR dense_multiset &operator=(std::initializer_list<value_type> il)
		{
			m_table.assign(il.begin(), il.end());
			return *this;
		}
		/** @copydoc operator= */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		TPP_CXX20_CONSTEXPR dense_multiset &operator=(std::initializer_list<T> il)
		{
			m_table.assign(il.begin(), il.end());
			return *this;
		}

		/** Returns iterator to the first element of the multiset.
		 * @note Elements are stored in no particular order. */
		[[nodiscard]] constexpr const_iterator begin() const noexcept { return m_table.begin(); }
		/** @copydoc begin */
		[[nodiscard]] constexpr const_iterator cbegin() const noexcept { return begin(); }
		/** Returns iterator one past the last element of the multiset.
		 * @note Elements are stored in no particular order. */
		[[nodiscard]] constexpr const_iterator end() const noexcept { return m_table.end(); }
		/** @copydoc end */
		[[nodiscard]] constexpr const_iterator cend() const noexcept { return end(); }

		/** Returns reverse iterator to the last element of the multiset.
		 * @note Elements are stored in no particular order. */
		[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return m_table.rbegin(); }
		/** @copydoc rbegin */
		[[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
		/** Returns reverse iterator one past the first element of the multiset.
		 * @note Elements are stored in no particular order. */
		[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return m_table.rend(); }
		/** @copydoc rend */
		[[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return rend(); }

		/** Returns the total number of elements within the multiset. */
		[[nodiscard]] constexpr size_type size() const noexcept { return m_table.size(); }
		/** Checks if the multiset is empty (`size() == 0`). */
		[[nodiscard]] constexpr bool empty() const noexcept { return m_table.size() == 0; }
		/** Returns the current capacity of the multiset (taking into account the maximum load factor). */
		[[nodiscard]] constexpr size_type capacity() const noexcept { return m_table.capacity(); }
		/** Returns the absolute maximum size of the multiset (taking into account the maximum load factor). */
		[[nodiscard]] constexpr size_type max_size() const noexcept { return m_table.max_size(); }
		/** Returns the current load factor of the multiset as if via `size() / bucket_count()`. */
		[[nodiscard]] constexpr float load_factor() const noexcept { return m_table.load_factor(); }

		/** Erases all elements from the multiset. */
		TPP_CXX20_CONSTEXPR void clear() { m_table.clear(); }

		/** @brief Inserts an element (of `value_type`) into the multiset if it does not exist yet.
		 * @param value Value of the to-be inserted element.
		 * @return Pair where `first` is the iterator to the inserted or existing element, and `second` is a boolean
		 * indicating whether insertion took place (`true` if element was inserted, `false` otherwise). */
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> insert(const insert_type &value) { return m_table.insert(value); }
		/** @copydoc insert */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> insert(const T &value) { return m_table.insert(value); }
		/** @copydoc insert */
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> insert(insert_type &&value) { return m_table.insert(std::forward<insert_type>(value)); }
		/** @copydoc insert */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, T &&>>>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> insert(T &&value) { return m_table.insert(std::forward<T>(value)); }
		/** @copybrief insert
		 * @param value Value of the to-be inserted element.
		 * @return Iterator to the inserted or existing element.
		 * @note \p hint has no effect, this overload exists for API compatibility. */
		TPP_CXX20_CONSTEXPR iterator insert(const_iterator hint, const insert_type &value) { return m_table.insert(hint, value); }
		/** @copydoc insert */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		TPP_CXX20_CONSTEXPR iterator insert(const_iterator hint, const T &value) { return m_table.insert(hint, value); }
		/** @copydoc insert */
		TPP_CXX20_CONSTEXPR iterator insert(const_iterator hint, insert_type &&value) { return m_table.insert(hint, std::forward<insert_type>(value)); }
		/** @copydoc insert */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, T &&>>>
		TPP_CXX20_CONSTEXPR iterator insert(const_iterator hint, T &&value) { return m_table.insert(hint, std::forward<T>(value)); }

		/** Inserts all elements from the range `[first, last)` into the multiset.
		 * @param first Iterator to the first element of the source range.
		 * @param last Iterator one past the last element of the source range. */
		template<typename I>
		TPP_CXX20_CONSTEXPR void insert(I first, I last) { return m_table.insert(first, last); }
		/** Inserts all elements of an initializer list into the multiset. */
		TPP_CXX20_CONSTEXPR void insert(std::initializer_list<value_type> il) { return insert(il.begin(), il.end()); }
		/** @copydoc insert */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		TPP_CXX20_CONSTEXPR void insert(std::initializer_list<T> il) { return insert(il.begin(), il.end()); }

		/** @brief Inserts an in-place constructed element (of `value_type`) into the multiset if it does not exist yet.
		 * @param args Arguments passed to constructor of `value_type`.
		 * @return Pair where `first` is the iterator to the inserted or existing element, and `second` is a boolean
		 * indicating whether insertion took place (`true` if element was inserted, `false` otherwise). */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> emplace(Args &&...args) { return m_table.emplace(std::forward<Args>(args)...); }
		/** @copybrief emplace
		 * @param args Arguments passed to constructor of `value_type`.
		 * @return Iterator to the inserted or existing element.
		 * @note \p hint has no effect, this overload exists for API compatibility. */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR iterator emplace_hint(const_iterator hint, Args &&...args) { return m_table.emplace_hint(hint, std::forward<Args>(args)...); }

		/** Removes the specified element from the multiset.
		 * @tparam I Index of the key.
		 * @param key `I`th key of the element to remove.
		 * @return Iterator to the element following the erased one, or `end()`. */
		template<std::size_t I>
		TPP_CXX20_CONSTEXPR iterator erase(const std::tuple_element_t<I, key_type> &key) { return m_table.template erase<I>(key); }
		/** @copydoc erase
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<std::size_t I, typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR iterator erase(const K &key) { return m_table.template erase<I>(key); }
		/** Removes the specified element from the multiset.
		 * @param pos Iterator pointing to the element to remove.
		 * @return Iterator to the element following the erased one, or `end()`. */
		TPP_CXX20_CONSTEXPR iterator erase(const_iterator pos) { return m_table.erase(pos); }
		/** Removes a range of elements from the multiset.
		 * @param first Iterator to the first element of the to-be removed range.
		 * @param last Iterator one past the last element of the to-be removed range.
		 * @return Iterator to the element following the erased range, or `end()`. */
		TPP_CXX20_CONSTEXPR iterator erase(const_iterator first, const_iterator last) { return m_table.erase(first, last); }

		/** Searches for the specified element within the multiset.
		 * @tparam I Index of the key.
		 * @param key `I`th key of the element to search for.
		 * @return Iterator to the specified element, or `end()`. */
		template<std::size_t I>
		[[nodiscard]] TPP_CXX20_CONSTEXPR iterator find(const std::tuple_element_t<I, key_type> &key) const { return m_table.template find<I>(key); }
		/** @copydoc find
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<std::size_t I, typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] TPP_CXX20_CONSTEXPR iterator find(const K &key) const { return m_table.template find<I>(key); }
		/** Checks if the specified element is present within the multiset as if by `find(key) != end()`.
		 * @tparam I Index of the key.
		 * @param key `I`th key of the element to search for.
		 * @return `true` if the element is present within the multiset, `false` otherwise. */
		template<std::size_t I>
		[[nodiscard]] TPP_CXX20_CONSTEXPR bool contains(const std::tuple_element_t<I, key_type> &key) const { return m_table.template contains<I>(key); }
		/** @copydoc contains
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<std::size_t I, typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] TPP_CXX20_CONSTEXPR bool contains(const K &key) const { return m_table.template contains<I>(key); }

		/** Returns forward iterator to the first element of the specified bucket. */
		[[nodiscard]] constexpr const_local_iterator begin(size_type n) const noexcept { return m_table.begin(n); }
		/** @copydoc begin */
		[[nodiscard]] constexpr const_local_iterator cbegin(size_type n) const noexcept { return m_table.begin(n); }
		/** Returns a sentinel iterator for the specified bucket. */
		[[nodiscard]] constexpr const_local_iterator end(size_type n) const noexcept { return m_table.end(n); }
		/** @copydoc end */
		[[nodiscard]] constexpr const_local_iterator cend(size_type n) const noexcept { return m_table.end(n); }

		/** Returns the bucket index of the specified element. */
		template<std::size_t I>
		[[nodiscard]] TPP_CXX20_CONSTEXPR size_type bucket(const std::tuple_element_t<I, key_type> &key) const { return m_table.bucket(key); }
		/** @copydoc bucket
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<std::size_t I, typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] TPP_CXX20_CONSTEXPR size_type bucket(const K &key) const { return m_table.bucket(key); }

		/** Returns the current amount of buckets of the multiset. */
		[[nodiscard]] constexpr size_type bucket_count() const noexcept { return m_table.bucket_count(); }
		/** Returns the maximum amount of buckets of the multiset. */
		[[nodiscard]] constexpr size_type max_bucket_count() const noexcept { return m_table.max_bucket_count(); }
		/** Returns the amount of elements within the specified bucket. */
		[[nodiscard]] constexpr size_type bucket_size(size_type n) const noexcept { return m_table.bucket_size(n); }

		/** Reserves space for at least `n` elements. */
		TPP_CXX20_CONSTEXPR void reserve(size_type n) { m_table.reserve(n); }
		/** Reserves space for at least `n` buckets and rehashes the multiset if necessary.
		 * @note The new amount of buckets is clamped to be at least `size() / max_load_factor()`. */
		TPP_CXX20_CONSTEXPR void rehash(size_type n) { m_table.rehash(n); }
		/** Returns the current maximum load factor. */
		[[nodiscard]] constexpr float max_load_factor() const noexcept { return m_table.max_load_factor(); }
		/** Sets the current maximum load factor. */
		constexpr void max_load_factor(float f) noexcept { m_table.max_load_factor(f); }

		[[nodiscard]] TPP_CXX20_CONSTEXPR allocator_type get_allocator() const { return allocator_type{m_table.get_allocator()}; }
		[[nodiscard]] TPP_CXX20_CONSTEXPR hasher hash_function() const { return m_table.get_hash(); }
		[[nodiscard]] TPP_CXX20_CONSTEXPR key_equal key_eq() const { return m_table.get_cmp(); }

		[[nodiscard]] TPP_CXX20_CONSTEXPR bool operator==(const dense_multiset &other) const
		{
			return std::is_permutation(begin(), end(), other.begin(), other.end());
		}
#if (__cplusplus < 202002L || _MSVC_LANG < 202002L)
		[[nodiscard]] TPP_CXX20_CONSTEXPR bool operator!=(const dense_multiset &other) const
		{
			return !std::is_permutation(begin(), end(), other.begin(), other.end());
		}
#endif

		TPP_CXX20_CONSTEXPR void swap(dense_multiset &other) noexcept(std::is_nothrow_swappable_v<table_t>) { m_table.swap(other.m_table); }

	private:
		table_t m_table;
	};

	/** Erases all elements from the set \p set that satisfy the predicate \p pred.
	 * @return Amount of elements erased. */
	template<typename Mk, typename H, typename C, typename A, typename P>
	TPP_CXX20_CONSTEXPR typename dense_multiset<Mk, H, C, A>::size_type erase_if(dense_multiset<Mk, H, C, A> &set, P pred)
	{
		typename dense_multiset<Mk, H, C, A>::size_type result = 0;
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

	template<typename Mk, typename H, typename C, typename A>
	TPP_CXX20_CONSTEXPR void swap(dense_multiset<Mk, H, C, A> &a, dense_multiset<Mk, H, C, A> &b)
	noexcept(std::is_nothrow_swappable_v<dense_multiset<Mk, H, C, A>>) { a.swap(b); }
}