/*
 * Created by switchblade on 11/5/22.
 */

#pragma once

#include "detail/dense_table.hpp"

namespace tpp
{
	/** @brief Hash map based on dense hash table.
	 *
	 * Internally, dense map stores it's elements in a contiguous unordered vector.
	 * Insert and erase operations on a dense map may invalidate references to it's
	 * elements due to the internal element vector being reordered.<br><br>
	 * Dense map iterators return a pair of references, as opposed to reference to a pair like STL maps do.
	 * This is required as the internal storage of dense map elements can be reordered, and as such elements are
	 * stored as `std::pair<Key, Mapped>` instead of `std::pair<const Key, Mapped>` to enable move-assignment and
	 * avoid copies. Because of this, elements must be converted to the const-qualified representation later on.
	 * Since a reference to `std::pair<T0, T1>` cannot be implicitly (and safely) converted to a reference to
	 * `std::pair<const T0, T1>`, this conversion is preformed element-wise, and a pair of references is returned instead.
	 *
	 * @tparam Key Key type stored by the map.
	 * @tparam Mapped Mapped type associated with map keys.
	 * @tparam KeyHash Hash functor used by the map.
	 * @tparam KeyCmp Compare functor used by the map.
	 * @tparam Alloc Allocator used by the map. */
	template<typename Key, typename Mapped, typename KeyHash = detail::default_hash<Key>, typename KeyCmp = std::equal_to<Key>,
	         typename Alloc = std::allocator<std::pair<Key, Mapped>>>
	class dense_map
	{
	public:
		using key_type = Key;
		using mapped_type = Mapped;
		using insert_type = std::pair<key_type, mapped_type>;
		using value_type = std::pair<const key_type, mapped_type>;

	private:
		struct traits_t
		{
			using link_type = detail::empty_link;

			using pointer = detail::packed_map_ptr<const key_type, mapped_type>;
			using const_pointer = detail::packed_map_ptr<const key_type, const mapped_type>;
			using reference = typename pointer::reference;
			using const_reference = typename const_pointer::reference;

			template<std::size_t, typename T>
			constexpr static auto &get_key(T &value) noexcept { return value.first; }
			template<typename T>
			constexpr static auto get_key(T &value) noexcept { return std::forward_as_tuple(value.first); }

			template<typename T>
			constexpr static auto &get_mapped(T &value) noexcept { return value.second; }

			constexpr static std::size_t key_size = 1;
		};

		using table_t = detail::dense_table<insert_type, value_type, key_type, KeyHash, KeyCmp, Alloc, traits_t>;

	public:
		/** Pair of references to `const key_type` and `mapped_type`. */
		using reference = typename table_t::reference;
		/** Pair of references to `const key_type` and `const mapped_type`. */
		using const_reference = typename table_t::const_reference;

		/** Fancy pointer to elements of the map, who's `operator->` returns `reference *`, and `operator*` returns `reference`. */
		using pointer = typename table_t::pointer;
		/** Fancy pointer to elements of the map, who's `operator->` returns `const_reference *`, and `operator*` returns `const_reference`. */
		using const_pointer = typename table_t::const_pointer;

		/** Random access iterator to elements of the map, who's `operator->` returns `pointer`, and `operator*` returns `reference`. */
		using iterator = typename table_t::iterator;
		/** Random access iterator to elements of the map, who's `operator->` returns `const_pointer`, and `operator*` returns `const_reference`. */
		using const_iterator = typename table_t::const_iterator;

		/** Random access iterator to elements of the map, equivalent to `std::reverse_iterator<iterator>`. */
		using reverse_iterator = typename table_t::reverse_iterator;
		/** Random access iterator to elements of the map, equivalent to `std::reverse_iterator<const_iterator>`. */
		using const_reverse_iterator = typename table_t::const_reverse_iterator;

		/** Forward iterator used to iterate through elements of a single bucket, who's `operator->` returns `pointer`, and `operator*` returns `reference`. */
		using local_iterator = typename table_t::local_iterator;
		/** Forward iterator used to iterate through elements of a single bucket, who's `operator->` returns `const_pointer`, and `operator*` returns `const_reference`. */
		using const_local_iterator = typename table_t::const_local_iterator;

		using size_type = typename table_t::size_type;
		using difference_type = typename table_t::difference_type;

		using hasher = typename table_t::hasher;
		using key_equal = typename table_t::key_equal;
		using allocator_type = typename table_t::allocator_type;

	public:
		/** Initializes the map with default capacity. */
		TPP_CXX20_CONSTEXPR dense_map() = default;
		/** Initializes the map with default capacity using the specified allocator. */
		TPP_CXX20_CONSTEXPR explicit dense_map(const allocator_type &alloc) : m_table(alloc) {}

		/** Copy-constructs the map. */
		TPP_CXX20_CONSTEXPR dense_map(const dense_map &other) = default;
		/** Copy-constructs the map using the specified allocator. */
		TPP_CXX20_CONSTEXPR dense_map(const dense_map &other, const allocator_type &alloc) : m_table(other.m_table, alloc) {}

		/** Move-constructs the map. */
		TPP_CXX20_CONSTEXPR dense_map(dense_map &&other) noexcept(std::is_nothrow_move_constructible_v<table_t>) = default;
		/** Move-constructs the map using the specified allocator. */
		TPP_CXX20_CONSTEXPR dense_map(dense_map &&other, const allocator_type &alloc)
		noexcept(std::is_nothrow_constructible_v<table_t, table_t &&, allocator_type>) : m_table(std::move(other.m_table), alloc) {}

		/** Initializes the map with the specified bucket count, hasher, comparator and allocator. */
		TPP_CXX20_CONSTEXPR explicit dense_map(size_type bucket_count, const hasher &hash = hasher{}, const key_equal &cmp = key_equal{},
		                                       const allocator_type &alloc = allocator_type{})
				: m_table(bucket_count, hash, cmp, alloc) {}
		/** Initializes the map with the specified bucket count, hasher and allocator. */
		TPP_CXX20_CONSTEXPR dense_map(size_type bucket_count, const hasher &hash, const allocator_type &alloc)
				: dense_map(bucket_count, hash, key_equal{}, alloc) {}
		/** Initializes the map with the specified bucket count and allocator. */
		TPP_CXX20_CONSTEXPR dense_map(size_type bucket_count, const allocator_type &alloc)
				: dense_map(bucket_count, hasher{}, alloc) {}

		/** Initializes the map with an initializer list of elements and the specified bucket count, hasher, comparator and allocator. */
		TPP_CXX20_CONSTEXPR dense_map(std::initializer_list<value_type> il, size_type bucket_count = table_t::initial_capacity,
		                              const hasher &hash = hasher{}, const key_equal &cmp = key_equal{},
		                              const allocator_type &alloc = allocator_type{})
				: dense_map(il.begin(), il.end(), bucket_count, hash, cmp, alloc) {}
		/** @copydoc dense_map */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		TPP_CXX20_CONSTEXPR dense_map(std::initializer_list<T> il, size_type bucket_count = table_t::initial_capacity,
		                              const hasher &hash = hasher{}, const key_equal &cmp = key_equal{},
		                              const allocator_type &alloc = allocator_type{})
				: dense_map(il.begin(), il.end(), bucket_count, hash, cmp, alloc) {}

		/** Initializes the map with an initializer list of elements and the specified bucket count, hasher and allocator. */
		TPP_CXX20_CONSTEXPR dense_map(std::initializer_list<value_type> il, size_type bucket_count, const hasher &hash, const allocator_type &alloc)
				: dense_map(il.begin(), il.end(), bucket_count, hash, key_equal{}, alloc) {}
		/** @copydoc dense_map */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		TPP_CXX20_CONSTEXPR dense_map(std::initializer_list<T> il, size_type bucket_count, const hasher &hash, const allocator_type &alloc)
				: dense_map(il.begin(), il.end(), bucket_count, hash, key_equal{}, alloc) {}

		/** Initializes the map with an initializer list of elements and the specified bucket count and allocator. */
		TPP_CXX20_CONSTEXPR dense_map(std::initializer_list<value_type> il, size_type bucket_count, const allocator_type &alloc)
				: dense_map(il.begin(), il.end(), bucket_count, hasher{}, alloc) {}
		/** @copydoc dense_map */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		TPP_CXX20_CONSTEXPR dense_map(std::initializer_list<T> il, size_type bucket_count, const allocator_type &alloc)
				: dense_map(il.begin(), il.end(), bucket_count, hasher{}, alloc) {}

		/** Initializes the map with a range of elements and the specified bucket count, hasher, comparator and allocator. */
		template<typename I>
		TPP_CXX20_CONSTEXPR dense_map(I first, I last, size_type bucket_count = table_t::initial_capacity,
		                              const hasher &hash = hasher{}, const key_equal &cmp = key_equal{},
		                              const allocator_type &alloc = allocator_type{})
				: m_table(first, last, bucket_count, hash, cmp, alloc) {}
		/** Initializes the map with a range of elements and the specified bucket count, hasher and allocator. */
		template<typename I>
		TPP_CXX20_CONSTEXPR dense_map(I first, I last, size_type bucket_count, const hasher &hash, const allocator_type &alloc)
				: dense_map(first, last, bucket_count, hash, key_equal{}, alloc) {}
		/** Initializes the map with a range of elements and the specified bucket count and allocator. */
		template<typename I>
		TPP_CXX20_CONSTEXPR dense_map(I first, I last, size_type bucket_count, const allocator_type &alloc)
				: dense_map(first, last, bucket_count, hasher{}, alloc) {}

		/** Copy-assigns the map. */
		TPP_CXX20_CONSTEXPR dense_map &operator=(const dense_map &) = default;
		/** Move-assigns the map. */
		TPP_CXX20_CONSTEXPR dense_map &operator=(dense_map &&) noexcept(std::is_nothrow_move_assignable_v<table_t>) = default;

		/** Replaces elements of the map with elements of the initializer list. */
		TPP_CXX20_CONSTEXPR dense_map &operator=(std::initializer_list<value_type> il)
		{
			m_table.assign(il.begin(), il.end());
			return *this;
		}

		/** Returns iterator to the first element of the map.
		 * @note Elements are stored in no particular order. */
		/** @copydoc begin */
		[[nodiscard]] constexpr iterator begin() noexcept { return m_table.begin(); }
		/** @copydoc begin */
		[[nodiscard]] constexpr const_iterator begin() const noexcept { return m_table.begin(); }
		/** @copydoc begin */
		[[nodiscard]] constexpr const_iterator cbegin() const noexcept { return begin(); }
		/** Returns iterator one past the last element of the map.
		 * @note Elements are stored in no particular order. */
		[[nodiscard]] constexpr iterator end() noexcept { return m_table.end(); }
		/** @copydoc end */
		[[nodiscard]] constexpr const_iterator end() const noexcept { return m_table.end(); }
		/** @copydoc end */
		[[nodiscard]] constexpr const_iterator cend() const noexcept { return end(); }

		/** Returns reverse iterator to the last element of the map.
		 * @note Elements are stored in no particular order. */
		[[nodiscard]] constexpr reverse_iterator rbegin() noexcept { return m_table.rbegin(); }
		/** @copydoc rbegin */
		[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return m_table.rbegin(); }
		/** @copydoc rbegin */
		[[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
		/** Returns reverse iterator one past the first element of the map.
		 * @note Elements are stored in no particular order. */
		[[nodiscard]] constexpr reverse_iterator rend() noexcept { return m_table.rend(); }
		/** @copydoc rend */
		[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return m_table.rend(); }
		/** @copydoc rend */
		[[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return rend(); }

		/** Returns the total number of elements within the map. */
		[[nodiscard]] constexpr size_type size() const noexcept { return m_table.size(); }
		/** Checks if the map is empty (`size() == 0`). */
		[[nodiscard]] constexpr bool empty() const noexcept { return m_table.size() == 0; }
		/** Returns the current capacity of the map (taking into account the maximum load factor). */
		[[nodiscard]] constexpr size_type capacity() const noexcept { return m_table.capacity(); }
		/** Returns the absolute maximum size of the map (taking into account the maximum load factor). */
		[[nodiscard]] constexpr size_type max_size() const noexcept { return m_table.max_size(); }
		/** Returns the current load factor of the map as if via `size() / bucket_count()`. */
		[[nodiscard]] constexpr float load_factor() const noexcept { return m_table.load_factor(); }

		/** Erases all elements from the map. */
		TPP_CXX20_CONSTEXPR void clear() { m_table.clear(); }

		/** @brief Inserts an element (of `value_type`) into the map if it does not exist yet.
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

		/** Inserts all elements from the range `[first, last)` into the map.
		 * @param first Iterator to the first element of the source range.
		 * @param last Iterator one past the last element of the source range. */
		template<typename I>
		TPP_CXX20_CONSTEXPR void insert(I first, I last) { return m_table.insert(first, last); }

		/** Inserts all elements of an initializer list into the map. */
		TPP_CXX20_CONSTEXPR void insert(std::initializer_list<value_type> il) { return insert(il.begin(), il.end()); }
		/** @copydoc insert */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		TPP_CXX20_CONSTEXPR void insert(std::initializer_list<T> il) { return insert(il.begin(), il.end()); }

		/** @brief If the specified key is not present within the map, inserts a new element. Otherwise, assigns value of the existing element.
		 * @param key Key of the element to insert or assign.
		 * @param value Value to be assigned to the key.
		 * @return Pair where `first` is the iterator to the inserted or existing element, and `second` is a boolean
		 * indicating whether insertion took place (`true` if element was inserted, `false` otherwise). */
		template<typename T>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> insert_or_assign(const key_type &key, T &&value)
		{
			return m_table.insert_or_assign(key, std::forward<T>(value));
		}
		/** @copydoc insert_or_assign */
		template<typename T>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> insert_or_assign(key_type &&key, T &&value)
		{
			return m_table.insert_or_assign(std::forward<key_type>(key), std::forward<T>(value));
		}
		/** @copydoc insert_or_assign
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename T, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> insert_or_assign(const K &key, T &&value)
		{
			return m_table.insert_or_assign(key, std::forward<T>(value));
		}
		/** @copydoc insert_or_assign */
		template<typename K, typename T, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> insert_or_assign(K &&key, T &&value)
		{
			return m_table.insert_or_assign(std::forward<K>(key), std::forward<T>(value));
		}

		/** @copybrief insert_or_assign
		 * @param key Key of the element to insert or assign.
		 * @param value Value to be assigned to the key.
		 * @return Iterator to the inserted or existing element.
		 * @note \p hint has no effect, this overload exists for API compatibility. */
		template<typename T>
		TPP_CXX20_CONSTEXPR iterator insert_or_assign(const_iterator hint, const key_type &key, T &&value)
		{
			return m_table.insert_or_assign(hint, key, std::forward<T>(value));
		}
		/** @copydoc insert_or_assign */
		template<typename T>
		TPP_CXX20_CONSTEXPR iterator insert_or_assign(const_iterator hint, key_type &&key, T &&value)
		{
			return m_table.insert_or_assign(hint, std::forward<key_type>(key), std::forward<T>(value));
		}
		/** @copydoc insert_or_assign
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename T, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR iterator insert_or_assign(const_iterator hint, const K &key, T &&value)
		{
			return m_table.insert_or_assign(hint, key, std::forward<T>(value));
		}
		/** @copydoc insert_or_assign */
		template<typename K, typename T, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR iterator insert_or_assign(const_iterator hint, K &&key, T &&value)
		{
			return m_table.insert_or_assign(hint, std::forward<K>(key), std::forward<T>(value));
		}

		/** @brief Inserts an in-place constructed element (of `value_type`) into the map if it does not exist yet.
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

		/** @brief Inserts an in-place constructed element (of `value_type`) into the map if it does not exist yet, or replaces an existing element.
		 * @param args Arguments passed to constructor of `value_type`.
		 * @return Pair where `first` is the iterator to the inserted or replaced element, and `second` is a boolean
		 * indicating whether insertion took place (`true` if element was inserted, `false` otherwise). */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> emplace_or_replace(const key_type &key, Args &&...args)
		{
			return m_table.emplace_or_replace(key, std::forward<Args>(args)...);
		}
		/** @copydoc emplace_or_replace */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> emplace_or_replace(key_type &&key, Args &&...args)
		{
			return m_table.emplace_or_replace(std::forward<key_type>(key), std::forward<Args>(args)...);
		}
		/** @copydoc emplace_or_replace
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename... Args, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> emplace_or_replace(const K &key, Args &&...args)
		{
			return m_table.emplace_or_replace(key, std::forward<Args>(args)...);
		}
		/** @copydoc emplace_or_replace */
		template<typename K, typename... Args, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> emplace_or_replace(K &&key, Args &&...args)
		{
			return m_table.emplace_or_replace(std::forward<K>(key), std::forward<Args>(args)...);
		}

		/** @copybrief emplace_or_replace
		 * @param args Arguments passed to constructor of `value_type`.
		 * @return Iterator to the inserted or replaced element.
		 * @note \p hint has no effect, this overload exists for API compatibility. */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR iterator emplace_or_replace(const_iterator hint, const key_type &key, Args &&...args)
		{
			return m_table.emplace_or_replace(hint, key, std::forward<Args>(args)...);
		}
		/** @copydoc emplace_or_replace */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR iterator emplace_or_replace(iterator hint, const key_type &key, Args &&...args)
		{
			return emplace_or_replace(const_iterator{hint}, key, std::forward<Args>(args)...);
		}
		/** @copydoc emplace_or_replace */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR iterator emplace_or_replace(const_iterator hint, key_type &&key, Args &&...args)
		{
			return m_table.emplace_or_replace(hint, std::forward<key_type>(key), std::forward<Args>(args)...);
		}
		/** @copydoc emplace_or_replace */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR iterator emplace_or_replace(iterator hint, key_type &&key, Args &&...args)
		{
			return emplace_or_replace(const_iterator{hint}, std::forward<key_type>(key), std::forward<Args>(args)...);
		}
		/** @copydoc emplace_or_replace
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename... Args, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR iterator emplace_or_replace(const_iterator hint, const K &key, Args &&...args)
		{
			return m_table.emplace_or_replace(hint, key, std::forward<Args>(args)...);
		}
		/** @copydoc emplace_or_replace */
		template<typename K, typename... Args, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR iterator emplace_or_replace(iterator hint, const K &key, Args &&...args)
		{
			return emplace_or_replace(const_iterator{hint}, key, std::forward<Args>(args)...);
		}
		/** @copydoc emplace_or_replace */
		template<typename K, typename... Args, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR iterator emplace_or_replace(const_iterator hint, K &&key, Args &&...args)
		{
			return m_table.emplace_or_replace(hint, std::forward<K>(key), std::forward<Args>(args)...);
		}
		/** @copydoc emplace_or_replace */
		template<typename K, typename... Args, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR iterator emplace_or_replace(iterator hint, K &&key, Args &&...args)
		{
			return emplace_or_replace(const_iterator{hint}, std::forward<K>(key), std::forward<Args>(args)...);
		}

		/** Attempts a piecewise constructed element (of `value_type`) at the specified key into the map  if it does not exist yet.
		 * @param key Key of the element to insert.
		 * @param args Arguments passed to constructor of `mapped_type`.
		 * @return Pair where `first` is the iterator to the inserted or existing element, and `second` is a boolean
		 * indicating whether insertion took place (`true` if element was inserted, `false` otherwise). */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> try_emplace(const key_type &key, Args &&...args)
		{
			return m_table.try_emplace(std::forward_as_tuple(key), std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> try_emplace(key_type &&key, Args &&...args)
		{
			return m_table.try_emplace(std::forward_as_tuple(std::move(key)), std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename... Args, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> try_emplace(const K &key, Args &&...args)
		{
			return m_table.try_emplace(std::forward_as_tuple(key), std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace */
		template<typename K, typename... Args, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> try_emplace(K &&key, Args &&...args)
		{
			return m_table.try_emplace(std::forward_as_tuple(std::forward<K>(key)), std::forward<Args>(args)...);
		}

		/** @copybrief try_emplace
		 * @param key Key of the element to insert.
		 * @param args Arguments passed to constructor of `mapped_type`.
		 * @return Iterator to the inserted or existing element.
		 * @note \p hint has no effect, this overload exists for API compatibility. */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR iterator try_emplace(const_iterator hint, const key_type &key, Args &&...args)
		{
			return m_table.try_emplace(hint, key, std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR iterator try_emplace(iterator hint, const key_type &key, Args &&...args)
		{
			return try_emplace(const_iterator{hint}, key, std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR iterator try_emplace(const_iterator hint, key_type &&key, Args &&...args)
		{
			return m_table.try_emplace(hint, std::move(key), std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR iterator try_emplace(iterator hint, key_type &&key, Args &&...args)
		{
			return try_emplace(const_iterator{hint}, std::move(key), std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename... Args, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR iterator try_emplace(const_iterator hint, const K &key, Args &&...args)
		{
			return m_table.try_emplace(hint, key, std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace */
		template<typename K, typename... Args, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR iterator try_emplace(iterator hint, const K &key, Args &&...args)
		{
			return try_emplace(const_iterator{hint}, key, std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace */
		template<typename K, typename... Args, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR iterator try_emplace(const_iterator hint, K &&key, Args &&...args)
		{
			return m_table.try_emplace(hint, std::forward<K>(key), std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace */
		template<typename K, typename... Args, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR iterator try_emplace(iterator hint, K &&key, Args &&...args)
		{
			return try_emplace(const_iterator{hint}, std::forward<K>(key), std::forward<Args>(args)...);
		}

		/** Removes the specified element from the map.
		 * @param key Key of the element to remove.
		 * @return Iterator to the element following the erased one, or `end()`. */
		TPP_CXX20_CONSTEXPR iterator erase(const key_type &key) { return m_table.template erase<0>(key); }
		/** @copydoc erase
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR iterator erase(const K &key) { return m_table.template erase<0>(key); }

		/** Removes the specified element from the map.
		 * @param pos Iterator pointing to the element to remove.
		 * @return Iterator to the element following the erased one, or `end()`. */
		TPP_CXX20_CONSTEXPR iterator erase(const_iterator pos) { return m_table.erase(pos); }
		/** @copydoc erase */
		TPP_CXX20_CONSTEXPR iterator erase(iterator pos) { return erase(const_iterator{pos}); }

		/** Removes a range of elements from the map.
		 * @param first Iterator to the first element of the to-be removed range.
		 * @param last Iterator one past the last element of the to-be removed range.
		 * @return Iterator to the element following the erased range, or `end()`. */
		TPP_CXX20_CONSTEXPR iterator erase(const_iterator first, const_iterator last) { return m_table.erase(first, last); }

		/** Searches for the specified element within the map.
		 * @param key Key of the element to search for.
		 * @return Iterator to the specified element, or `end()`. */
		[[nodiscard]] TPP_CXX20_CONSTEXPR iterator find(const key_type &key) { return m_table.template find<0>(key); }
		/** @copydoc find */
		[[nodiscard]] TPP_CXX20_CONSTEXPR const_iterator find(const key_type &key) const { return m_table.template find<0>(key); }
		/** @copydoc find
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] TPP_CXX20_CONSTEXPR iterator find(const K &key) { return m_table.template find<0>(key); }
		/** @copydoc find */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] TPP_CXX20_CONSTEXPR const_iterator find(const K &key) const { return m_table.template find<0>(key); }

		/** Checks if the specified element is present within the map as if by `find(key) != end()`.
		 * @param key Key of the element to search for.
		 * @return `true` if the element is present within the map, `false` otherwise. */
		[[nodiscard]] TPP_CXX20_CONSTEXPR bool contains(const key_type &key) const { return m_table.template contains<0>(key); }
		/** @copydoc contains
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] TPP_CXX20_CONSTEXPR bool contains(const K &key) const { return m_table.template contains<0>(key); }

		/** Returns reference to the specified element.
		 * @param key Key of the element to search for.
		 * @return Reference to the specified element.
		 * @throw std::out_of_range If no such element exists within the map. */
		[[nodiscard]] reference at(const key_type &key) { return *guard_at(find(key)); }
		/** @copydoc at */
		[[nodiscard]] const_reference at(const key_type &key) const { return *guard_at(find(key)); }
		/** @copydoc at
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] reference at(const K &key) { return *guard_at(find(key)); }
		/** @copydoc at */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] const_reference at(const K &key) const { return *guard_at(find(key)); }

		/** Returns reference to the specified element. If the element is not present within the map, inserts a default-constructed instance.
		 * @param key Key of the element to search for.
		 * @return Reference to the specified element. */
		[[nodiscard]] TPP_CXX20_CONSTEXPR mapped_type &operator[](const key_type &key) { return try_emplace(key).first->second; }
		/** @copydoc operator[] */
		[[nodiscard]] TPP_CXX20_CONSTEXPR mapped_type &operator[](key_type &key) { return try_emplace(std::forward<key_type>(key)).first->second; }
		/** @copydoc operator[]
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] TPP_CXX20_CONSTEXPR mapped_type &operator[](const K &key) { return try_emplace(key).first->second; }
		/** @copydoc operator[] */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] TPP_CXX20_CONSTEXPR mapped_type &operator[](K &key) { return try_emplace(std::forward<K>(key)).first->second; }

		/** Returns forward iterator to the first element of the specified bucket. */
		[[nodiscard]] constexpr local_iterator begin(size_type n) noexcept { return m_table.begin(n); }
		/** @copydoc begin */
		[[nodiscard]] constexpr const_local_iterator begin(size_type n) const noexcept { return m_table.begin(n); }
		/** @copydoc begin */
		[[nodiscard]] constexpr const_local_iterator cbegin(size_type n) const noexcept { return m_table.begin(n); }
		/** Returns a sentinel iterator for the specified bucket. */
		[[nodiscard]] constexpr local_iterator end(size_type n) noexcept { return m_table.end(n); }
		/** @copydoc end */
		[[nodiscard]] constexpr const_local_iterator end(size_type n) const noexcept { return m_table.end(n); }
		/** @copydoc end */
		[[nodiscard]] constexpr const_local_iterator cend(size_type n) const noexcept { return m_table.end(n); }

		/** Returns the bucket index of the specified element. */
		[[nodiscard]] TPP_CXX20_CONSTEXPR size_type bucket(const key_type &key) const { return m_table.bucket(key); }
		/** @copydoc bucket
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] TPP_CXX20_CONSTEXPR size_type bucket(const K &key) const { return m_table.bucket(key); }

		/** Returns the current amount of buckets of the map. */
		[[nodiscard]] constexpr size_type bucket_count() const noexcept { return m_table.bucket_count(); }
		/** Returns the maximum amount of buckets of the map. */
		[[nodiscard]] constexpr size_type max_bucket_count() const noexcept { return m_table.max_bucket_count(); }
		/** Returns the amount of elements within the specified bucket. */
		[[nodiscard]] constexpr size_type bucket_size(size_type n) const noexcept { return m_table.bucket_size(n); }

		/** Reserves space for at least `n` elements. */
		TPP_CXX20_CONSTEXPR void reserve(size_type n) { m_table.reserve(n); }
		/** Reserves space for at least `n` buckets and rehashes the map if necessary.
		 * @note The new amount of buckets is clamped to be at least `size() / max_load_factor()`. */
		TPP_CXX20_CONSTEXPR void rehash(size_type n) { m_table.rehash(n); }
		/** Returns the current maximum load factor. */
		[[nodiscard]] constexpr float max_load_factor() const noexcept { return m_table.max_load_factor(); }
		/** Sets the current maximum load factor. */
		constexpr void max_load_factor(float f) noexcept { m_table.max_load_factor(f); }

		[[nodiscard]] TPP_CXX20_CONSTEXPR allocator_type get_allocator() const { return allocator_type{m_table.get_allocator()}; }
		[[nodiscard]] TPP_CXX20_CONSTEXPR hasher hash_function() const { return m_table.get_hash(); }
		[[nodiscard]] TPP_CXX20_CONSTEXPR key_equal key_eq() const { return m_table.get_cmp(); }

		[[nodiscard]] TPP_CXX20_CONSTEXPR bool operator==(const dense_map &other) const
		{
			return std::is_permutation(begin(), end(), other.begin(), other.end());
		}
#if (__cplusplus < 202002L || _MSVC_LANG < 202002L)
		[[nodiscard]] TPP_CXX20_CONSTEXPR bool operator!=(const dense_map &other) const
		{
			return !std::is_permutation(begin(), end(), other.begin(), other.end());
		}
#endif

		TPP_CXX20_CONSTEXPR void swap(dense_map &other) noexcept(std::is_nothrow_swappable_v<table_t>) { m_table.swap(other.m_table); }

	private:
		template<typename I>
		[[nodiscard]] inline auto guard_at(I iter) const
		{
			if (iter == end())
				throw std::out_of_range("`dense_map::at` - invalid key");
			else
				return iter;
		}

		table_t m_table;
	};

	/** Erases all elements from the map \p map that satisfy the predicate \p pred.
	 * @return Amount of elements erased. */
	template<typename K, typename M, typename H, typename C, typename A, typename P>
	TPP_CXX20_CONSTEXPR typename dense_map<K, M, H, C, A>::size_type erase_if(dense_map<K, M, H, C, A> &map, P pred)
	{
		typename dense_map<K, M, H, C, A>::size_type result = 0;
		for (auto i = map.cbegin(), last = map.cend(); i != last;)
		{
			if (pred(*i))
			{
				i = map.erase(i);
				++result;
			}
			else
				++i;
		}
		return result;
	}

	template<typename K, typename M, typename H, typename C, typename A>
	TPP_CXX20_CONSTEXPR void swap(dense_map<K, M, H, C, A> &a, dense_map<K, M, H, C, A> &b)
	noexcept(std::is_nothrow_swappable_v<dense_map<K, H, C, A>>) { a.swap(b); }

	/** @brief Ordered hash map based on dense hash table.
	 *
	 * Internally, ordered dense map stores it's elements in a contiguous vector with additional ordering
	 * links between elements. Insert and erase operations on a dense map may invalidate references to it's
	 * elements due to the internal element vector being reordered.<br><br>
	 * Dense map iterators return a pair of references, as opposed to reference to a pair like STL maps do.
	 * This is required as the internal storage of dense map elements can be reordered, and as such elements are
	 * stored as `std::pair<Key, Mapped>` instead of `std::pair<const Key, Mapped>` to enable move-assignment and
	 * avoid copies. Because of this, elements must be converted to the const-qualified representation later on.
	 * Since a reference to `std::pair<T0, T1>` cannot be implicitly (and safely) converted to a reference to
	 * `std::pair<const T0, T1>`, this conversion is preformed element-wise, and a pair of references is returned instead.
	 *
	 * @tparam Key Key type stored by the map.
	 * @tparam Mapped Mapped type associated with map keys.
	 * @tparam KeyHash Hash functor used by the map.
	 * @tparam KeyCmp Compare functor used by the map.
	 * @tparam Alloc Allocator used by the map. */
	template<typename Key, typename Mapped, typename KeyHash = detail::default_hash<Key>, typename KeyCmp = std::equal_to<Key>,
	         typename Alloc = std::allocator<std::pair<Key, Mapped>>>
	class ordered_dense_map
	{
	public:
		using key_type = Key;
		using mapped_type = Mapped;
		using insert_type = std::pair<key_type, mapped_type>;
		using value_type = std::pair<const key_type, mapped_type>;

	private:
		struct traits_t
		{
			using link_type = detail::ordered_link;

			using pointer = detail::packed_map_ptr<const key_type, mapped_type>;
			using const_pointer = detail::packed_map_ptr<const key_type, const mapped_type>;
			using reference = typename pointer::reference;
			using const_reference = typename const_pointer::reference;

			template<std::size_t, typename T>
			constexpr static auto &get_key(T &value) noexcept { return value.first; }
			template<typename T>
			constexpr static auto get_key(T &value) noexcept { return std::forward_as_tuple(value.first); }

			template<typename T>
			constexpr static auto &get_mapped(T &value) noexcept { return value.second; }

			constexpr static std::size_t key_size = 1;
		};

		using table_t = detail::dense_table<insert_type, value_type, key_type, KeyHash, KeyCmp, Alloc, traits_t>;

	public:
		/** Pair of references to `const key_type` and `mapped_type`. */
		using reference = typename table_t::reference;
		/** Pair of references to `const key_type` and `const mapped_type`. */
		using const_reference = typename table_t::const_reference;

		/** Fancy pointer to elements of the map, who's `operator->` returns `reference *`, and `operator*` returns `reference`. */
		using pointer = typename table_t::pointer;
		/** Fancy pointer to elements of the map, who's `operator->` returns `const_reference *`, and `operator*` returns `const_reference`. */
		using const_pointer = typename table_t::const_pointer;

		/** Bidirectional iterator to elements of the map, who's `operator->` returns `pointer`, and `operator*` returns `reference`. */
		using iterator = typename table_t::iterator;
		/** Bidirectional iterator to elements of the map, who's `operator->` returns `const_pointer`, and `operator*` returns `const_reference`. */
		using const_iterator = typename table_t::const_iterator;

		/** Bidirectional iterator to elements of the map, equivalent to `std::reverse_iterator<iterator>`. */
		using reverse_iterator = typename table_t::reverse_iterator;
		/** Bidirectional iterator to elements of the map, equivalent to `std::reverse_iterator<const_iterator>`. */
		using const_reverse_iterator = typename table_t::const_reverse_iterator;

		/** Forward iterator used to iterate through elements of a single bucket, who's `operator->` returns `pointer`, and `operator*` returns `reference`. */
		using local_iterator = typename table_t::local_iterator;
		/** Forward iterator used to iterate through elements of a single bucket, who's `operator->` returns `const_pointer`, and `operator*` returns `const_reference`. */
		using const_local_iterator = typename table_t::const_local_iterator;

		using size_type = typename table_t::size_type;
		using difference_type = typename table_t::difference_type;

		using hasher = typename table_t::hasher;
		using key_equal = typename table_t::key_equal;
		using allocator_type = typename table_t::allocator_type;

	public:
		/** Initializes the map with default capacity. */
		TPP_CXX20_CONSTEXPR ordered_dense_map() = default;
		/** Initializes the map with default capacity using the specified allocator. */
		TPP_CXX20_CONSTEXPR explicit ordered_dense_map(const allocator_type &alloc) : m_table(alloc) {}

		/** Copy-constructs the map. */
		TPP_CXX20_CONSTEXPR ordered_dense_map(const ordered_dense_map &other) = default;
		/** Copy-constructs the map using the specified allocator. */
		TPP_CXX20_CONSTEXPR ordered_dense_map(const ordered_dense_map &other, const allocator_type &alloc) : m_table(other.m_table, alloc) {}

		/** Move-constructs the map. */
		TPP_CXX20_CONSTEXPR ordered_dense_map(ordered_dense_map &&other) noexcept(std::is_nothrow_move_constructible_v<table_t>) = default;
		/** Move-constructs the map using the specified allocator. */
		TPP_CXX20_CONSTEXPR ordered_dense_map(ordered_dense_map &&other, const allocator_type &alloc) noexcept(std::is_nothrow_constructible_v<table_t,
		                                                                                                                                       table_t &&,
		                                                                                                                                       allocator_type>)
				: m_table(std::move(other.m_table), alloc) {}

		/** Initializes the map with the specified bucket count, hasher, comparator and allocator. */
		TPP_CXX20_CONSTEXPR explicit ordered_dense_map(size_type bucket_count, const hasher &hash = hasher{}, const key_equal &cmp = key_equal{},
		                                               const allocator_type &alloc = allocator_type{})
				: m_table(bucket_count, hash, cmp, alloc) {}
		/** Initializes the map with the specified bucket count, hasher and allocator. */
		TPP_CXX20_CONSTEXPR ordered_dense_map(size_type bucket_count, const hasher &hash, const allocator_type &alloc)
				: ordered_dense_map(bucket_count, hash, key_equal{}, alloc) {}
		/** Initializes the map with the specified bucket count and allocator. */
		TPP_CXX20_CONSTEXPR ordered_dense_map(size_type bucket_count, const allocator_type &alloc)
				: ordered_dense_map(bucket_count, hasher{}, alloc) {}

		/** Initializes the map with an initializer list of elements and the specified bucket count, hasher, comparator and allocator. */
		TPP_CXX20_CONSTEXPR ordered_dense_map(std::initializer_list<value_type> il, size_type bucket_count = table_t::initial_capacity,
		                                      const hasher &hash = hasher{}, const key_equal &cmp = key_equal{},
		                                      const allocator_type &alloc = allocator_type{})
				: ordered_dense_map(il.begin(), il.end(), bucket_count, hash, cmp, alloc) {}
		/** Initializes the map with an initializer list of elements and the specified bucket count, hasher and allocator. */
		TPP_CXX20_CONSTEXPR ordered_dense_map(std::initializer_list<value_type> il, size_type bucket_count, const hasher &hash, const allocator_type &alloc)
				: ordered_dense_map(il.begin(), il.end(), bucket_count, hash, key_equal{}, alloc) {}
		/** Initializes the map with an initializer list of elements and the specified bucket count and allocator. */
		TPP_CXX20_CONSTEXPR ordered_dense_map(std::initializer_list<value_type> il, size_type bucket_count, const allocator_type &alloc)
				: ordered_dense_map(il.begin(), il.end(), bucket_count, hasher{}, alloc) {}

		/** Initializes the map with a range of elements and the specified bucket count, hasher, comparator and allocator. */
		template<typename I>
		TPP_CXX20_CONSTEXPR ordered_dense_map(I first, I last, size_type bucket_count = table_t::initial_capacity,
		                                      const hasher &hash = hasher{}, const key_equal &cmp = key_equal{},
		                                      const allocator_type &alloc = allocator_type{})
				: m_table(first, last, bucket_count, hash, cmp, alloc) {}
		/** Initializes the map with a range of elements and the specified bucket count, hasher and allocator. */
		template<typename I>
		TPP_CXX20_CONSTEXPR ordered_dense_map(I first, I last, size_type bucket_count, const hasher &hash, const allocator_type &alloc)
				: ordered_dense_map(first, last, bucket_count, hash, key_equal{}, alloc) {}
		/** Initializes the map with a range of elements and the specified bucket count and allocator. */
		template<typename I>
		TPP_CXX20_CONSTEXPR ordered_dense_map(I first, I last, size_type bucket_count, const allocator_type &alloc)
				: ordered_dense_map(first, last, bucket_count, hasher{}, alloc) {}

		/** Copy-assigns the map. */
		TPP_CXX20_CONSTEXPR ordered_dense_map &operator=(const ordered_dense_map &) = default;
		/** Move-assigns the map. */
		TPP_CXX20_CONSTEXPR ordered_dense_map &operator=(ordered_dense_map &&) noexcept(std::is_nothrow_move_assignable_v<table_t>) = default;

		/** Replaces elements of the map with elements of the initializer list. */
		TPP_CXX20_CONSTEXPR ordered_dense_map &operator=(std::initializer_list<value_type> il)
		{
			m_table.assign(il.begin(), il.end());
			return *this;
		}

		/** Returns iterator to the first element of the map.
		 * @note Elements are stored in no particular order. */
		/** @copydoc begin */
		[[nodiscard]] constexpr iterator begin() noexcept { return m_table.begin(); }
		/** @copydoc begin */
		[[nodiscard]] constexpr const_iterator begin() const noexcept { return m_table.begin(); }
		/** @copydoc begin */
		[[nodiscard]] constexpr const_iterator cbegin() const noexcept { return begin(); }
		/** Returns iterator one past the last element of the map.
		 * @note Elements are stored in no particular order. */
		[[nodiscard]] constexpr iterator end() noexcept { return m_table.end(); }
		/** @copydoc end */
		[[nodiscard]] constexpr const_iterator end() const noexcept { return m_table.end(); }
		/** @copydoc end */
		[[nodiscard]] constexpr const_iterator cend() const noexcept { return end(); }

		/** Returns reverse iterator to the last element of the map.
		 * @note Elements are stored in no particular order. */
		[[nodiscard]] constexpr reverse_iterator rbegin() noexcept { return m_table.rbegin(); }
		/** @copydoc rbegin */
		[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return m_table.rbegin(); }
		/** @copydoc rbegin */
		[[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
		/** Returns reverse iterator one past the first element of the map.
		 * @note Elements are stored in no particular order. */
		[[nodiscard]] constexpr reverse_iterator rend() noexcept { return m_table.rend(); }
		/** @copydoc rend */
		[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return m_table.rend(); }
		/** @copydoc rend */
		[[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return rend(); }

		/** Returns reference to the first element of the map. */
		[[nodiscard]] constexpr reference front() noexcept { return m_table.front(); }
		/** @copydoc front */
		[[nodiscard]] constexpr const_reference front() const noexcept { return m_table.front(); }
		/** Returns reference to the last element of the map. */
		[[nodiscard]] constexpr reference back() noexcept { return m_table.back(); }
		/** @copydoc back */
		[[nodiscard]] constexpr const_reference back() const noexcept { return m_table.back(); }

		/** Returns the total number of elements within the map. */
		[[nodiscard]] constexpr size_type size() const noexcept { return m_table.size(); }
		/** Checks if the map is empty (`size() == 0`). */
		[[nodiscard]] constexpr bool empty() const noexcept { return m_table.size() == 0; }
		/** Returns the current capacity of the map (taking into account the maximum load factor). */
		[[nodiscard]] constexpr size_type capacity() const noexcept { return m_table.capacity(); }
		/** Returns the absolute maximum size of the map (taking into account the maximum load factor). */
		[[nodiscard]] constexpr size_type max_size() const noexcept { return m_table.max_size(); }
		/** Returns the current load factor of the map as if via `size() / bucket_count()`. */
		[[nodiscard]] constexpr float load_factor() const noexcept { return m_table.load_factor(); }

		/** Erases all elements from the map. */
		TPP_CXX20_CONSTEXPR void clear() { m_table.clear(); }

		/** @brief Inserts an element (of `value_type`) into the map if it does not exist yet.
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
		 * @param hint New position of the inserted element.
		 * @param value Value of the to-be inserted element.
		 * @return Iterator to the inserted or existing element. */
		TPP_CXX20_CONSTEXPR iterator insert(const_iterator hint, const insert_type &value) { return m_table.insert(hint, value); }
		/** @copydoc insert */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		TPP_CXX20_CONSTEXPR iterator insert(const_iterator hint, const T &value) { return m_table.insert(hint, value); }
		/** @copydoc insert */
		TPP_CXX20_CONSTEXPR iterator insert(const_iterator hint, insert_type &&value) { return m_table.insert(hint, std::forward<insert_type>(value)); }
		/** @copydoc insert */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, T &&>>>
		TPP_CXX20_CONSTEXPR iterator insert(const_iterator hint, T &&value) { return m_table.insert(hint, std::forward<T>(value)); }

		/** Inserts all elements from the range `[first, last)` into the map.
		 * @param first Iterator to the first element of the source range.
		 * @param last Iterator one past the last element of the source range. */
		template<typename I>
		TPP_CXX20_CONSTEXPR void insert(I first, I last) { return m_table.insert(first, last); }

		/** Inserts all elements of an initializer list into the map. */
		TPP_CXX20_CONSTEXPR void insert(std::initializer_list<value_type> il) { return insert(il.begin(), il.end()); }
		/** @copydoc insert */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		TPP_CXX20_CONSTEXPR void insert(std::initializer_list<T> il) { return insert(il.begin(), il.end()); }

		/** @brief If the specified key is not present within the map, inserts a new element. Otherwise, assigns value of the existing element.
		 * @param key Key of the element to insert or assign.
		 * @param value Value to be assigned to the key.
		 * @return Pair where `first` is the iterator to the inserted or existing element, and `second` is a boolean
		 * indicating whether insertion took place (`true` if element was inserted, `false` otherwise). */
		template<typename T>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> insert_or_assign(const key_type &key, T &&value)
		{
			return m_table.insert_or_assign(key, std::forward<T>(value));
		}
		/** @copydoc insert_or_assign */
		template<typename T>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> insert_or_assign(key_type &&key, T &&value)
		{
			return m_table.insert_or_assign(std::forward<key_type>(key), std::forward<T>(value));
		}
		/** @copydoc insert_or_assign
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename T, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> insert_or_assign(const K &key, T &&value)
		{
			return m_table.insert_or_assign(key, std::forward<T>(value));
		}
		/** @copydoc insert_or_assign */
		template<typename K, typename T, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> insert_or_assign(K &&key, T &&value)
		{
			return m_table.insert_or_assign(std::forward<K>(key), std::forward<T>(value));
		}

		/** @copybrief insert_or_assign
		 * @param hint New position of the inserted element.
		 * @param key Key of the element to insert or assign.
		 * @param value Value to be assigned to the key.
		 * @return Iterator to the inserted or existing element. */
		template<typename T>
		TPP_CXX20_CONSTEXPR iterator insert_or_assign(const_iterator hint, const key_type &key, T &&value)
		{
			return m_table.insert_or_assign(hint, key, std::forward<T>(value));
		}
		/** @copydoc insert_or_assign */
		template<typename T>
		TPP_CXX20_CONSTEXPR iterator insert_or_assign(const_iterator hint, key_type &&key, T &&value)
		{
			return m_table.insert_or_assign(hint, std::forward<key_type>(key), std::forward<T>(value));
		}
		/** @copydoc insert_or_assign
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename T, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR iterator insert_or_assign(const_iterator hint, const K &key, T &&value)
		{
			return m_table.insert_or_assign(hint, key, std::forward<T>(value));
		}
		/** @copydoc insert_or_assign */
		template<typename K, typename T, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR iterator insert_or_assign(const_iterator hint, K &&key, T &&value)
		{
			return m_table.insert_or_assign(hint, std::forward<K>(key), std::forward<T>(value));
		}

		/** @brief Inserts an in-place constructed element (of `value_type`) into the map if it does not exist yet.
		 * @param args Arguments passed to constructor of `value_type`.
		 * @return Pair where `first` is the iterator to the inserted or existing element, and `second` is a boolean
		 * indicating whether insertion took place (`true` if element was inserted, `false` otherwise). */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> emplace(Args &&...args) { return m_table.emplace(std::forward<Args>(args)...); }
		/** @copybrief emplace
		 * @param hint New position of the inserted element.
		 * @param args Arguments passed to constructor of `value_type`.
		 * @return Iterator to the inserted or existing element. */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR iterator emplace_hint(const_iterator hint, Args &&...args) { return m_table.emplace_hint(hint, std::forward<Args>(args)...); }

		/** @brief Inserts an in-place constructed element (of `value_type`) into the map if it does not exist yet, or replaces an existing element.
		 * @param args Arguments passed to constructor of `value_type`.
		 * @return Pair where `first` is the iterator to the inserted or replaced element, and `second` is a boolean
		 * indicating whether insertion took place (`true` if element was inserted, `false` otherwise). */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> emplace_or_replace(const key_type &key, Args &&...args)
		{
			return m_table.emplace_or_replace(key, std::forward<Args>(args)...);
		}
		/** @copydoc emplace_or_replace */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> emplace_or_replace(key_type &&key, Args &&...args)
		{
			return m_table.emplace_or_replace(std::forward<key_type>(key), std::forward<Args>(args)...);
		}
		/** @copydoc emplace_or_replace
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename... Args, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> emplace_or_replace(const K &key, Args &&...args)
		{
			return m_table.emplace_or_replace(key, std::forward<Args>(args)...);
		}
		/** @copydoc emplace_or_replace */
		template<typename K, typename... Args, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> emplace_or_replace(K &&key, Args &&...args)
		{
			return m_table.emplace_or_replace(std::forward<K>(key), std::forward<Args>(args)...);
		}

		/** @copybrief emplace_or_replace
		 * @param hint New position of the inserted element.
		 * @param args Arguments passed to constructor of `value_type`.
		 * @return Iterator to the inserted or replaced element. */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR iterator emplace_or_replace(const_iterator hint, const key_type &key, Args &&...args)
		{
			return m_table.emplace_or_replace(hint, key, std::forward<Args>(args)...);
		}
		/** @copydoc emplace_or_replace */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR iterator emplace_or_replace(iterator hint, const key_type &key, Args &&...args)
		{
			return emplace_or_replace(const_iterator{hint}, key, std::forward<Args>(args)...);
		}
		/** @copydoc emplace_or_replace */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR iterator emplace_or_replace(const_iterator hint, key_type &&key, Args &&...args)
		{
			return m_table.emplace_or_replace(hint, std::forward<key_type>(key), std::forward<Args>(args)...);
		}
		/** @copydoc emplace_or_replace */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR iterator emplace_or_replace(iterator hint, key_type &&key, Args &&...args)
		{
			return emplace_or_replace(const_iterator{hint}, std::forward<key_type>(key), std::forward<Args>(args)...);
		}
		/** @copydoc emplace_or_replace
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename... Args, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR iterator emplace_or_replace(const_iterator hint, const K &key, Args &&...args)
		{
			return m_table.emplace_or_replace(hint, key, std::forward<Args>(args)...);
		}
		/** @copydoc emplace_or_replace */
		template<typename K, typename... Args, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR iterator emplace_or_replace(iterator hint, const K &key, Args &&...args)
		{
			return emplace_or_replace(const_iterator{hint}, key, std::forward<Args>(args)...);
		}
		/** @copydoc emplace_or_replace */
		template<typename K, typename... Args, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR iterator emplace_or_replace(const_iterator hint, K &&key, Args &&...args)
		{
			return m_table.emplace_or_replace(hint, std::forward<K>(key), std::forward<Args>(args)...);
		}
		/** @copydoc emplace_or_replace */
		template<typename K, typename... Args, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR iterator emplace_or_replace(iterator hint, K &&key, Args &&...args)
		{
			return emplace_or_replace(const_iterator{hint}, std::forward<K>(key), std::forward<Args>(args)...);
		}

		/** Attempts a piecewise constructed element (of `value_type`) at the specified key into the map  if it does not exist yet.
		 * @param key Key of the element to insert.
		 * @param args Arguments passed to constructor of `mapped_type`.
		 * @return Pair where `first` is the iterator to the inserted or existing element, and `second` is a boolean
		 * indicating whether insertion took place (`true` if element was inserted, `false` otherwise). */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> try_emplace(const key_type &key, Args &&...args)
		{
			return m_table.try_emplace(std::forward_as_tuple(key), std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> try_emplace(key_type &&key, Args &&...args)
		{
			return m_table.try_emplace(std::forward_as_tuple(std::move(key)), std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename... Args, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> try_emplace(const K &key, Args &&...args)
		{
			return m_table.try_emplace(std::forward_as_tuple(key), std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace */
		template<typename K, typename... Args, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR std::pair<iterator, bool> try_emplace(K &&key, Args &&...args)
		{
			return m_table.try_emplace(std::forward_as_tuple(std::forward<K>(key)), std::forward<Args>(args)...);
		}

		/** @copybrief try_emplace
		 * @param hint New position of the inserted element.
		 * @param key Key of the element to insert.
		 * @param args Arguments passed to constructor of `mapped_type`.
		 * @return Iterator to the inserted or existing element. */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR iterator try_emplace(const_iterator hint, const key_type &key, Args &&...args)
		{
			return m_table.try_emplace(hint, key, std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR iterator try_emplace(iterator hint, const key_type &key, Args &&...args)
		{
			return try_emplace(const_iterator{hint}, key, std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR iterator try_emplace(const_iterator hint, key_type &&key, Args &&...args)
		{
			return m_table.try_emplace(hint, std::move(key), std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace */
		template<typename... Args>
		TPP_CXX20_CONSTEXPR iterator try_emplace(iterator hint, key_type &&key, Args &&...args)
		{
			return try_emplace(const_iterator{hint}, std::move(key), std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename... Args, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR iterator try_emplace(const_iterator hint, const K &key, Args &&...args)
		{
			return m_table.try_emplace(hint, key, std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace */
		template<typename K, typename... Args, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR iterator try_emplace(iterator hint, const K &key, Args &&...args)
		{
			return try_emplace(const_iterator{hint}, key, std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace */
		template<typename K, typename... Args, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR iterator try_emplace(const_iterator hint, K &&key, Args &&...args)
		{
			return m_table.try_emplace(hint, std::forward<K>(key), std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace */
		template<typename K, typename... Args, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR iterator try_emplace(iterator hint, K &&key, Args &&...args)
		{
			return try_emplace(const_iterator{hint}, std::forward<K>(key), std::forward<Args>(args)...);
		}

		/** Removes the specified element from the map.
		 * @param key Key of the element to remove.
		 * @return Iterator to the element following the erased one, or `end()`. */
		TPP_CXX20_CONSTEXPR iterator erase(const key_type &key) { return m_table.template erase<0>(key); }
		/** @copydoc erase
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		TPP_CXX20_CONSTEXPR iterator erase(const K &key) { return m_table.template erase<0>(key); }

		/** Removes the specified element from the map.
		 * @param pos Iterator pointing to the element to remove.
		 * @return Iterator to the element following the erased one, or `end()`. */
		TPP_CXX20_CONSTEXPR iterator erase(const_iterator pos) { return m_table.erase(pos); }
		/** @copydoc erase */
		TPP_CXX20_CONSTEXPR iterator erase(iterator pos) { return erase(const_iterator{pos}); }

		/** Removes a range of elements from the map.
		 * @param first Iterator to the first element of the to-be removed range.
		 * @param last Iterator one past the last element of the to-be removed range.
		 * @return Iterator to the element following the erased range, or `end()`. */
		TPP_CXX20_CONSTEXPR iterator erase(const_iterator first, const_iterator last) { return m_table.erase(first, last); }

		/** Searches for the specified element within the map.
		 * @param key Key of the element to search for.
		 * @return Iterator to the specified element, or `end()`. */
		[[nodiscard]] TPP_CXX20_CONSTEXPR iterator find(const key_type &key) { return m_table.template find<0>(key); }
		/** @copydoc find */
		[[nodiscard]] TPP_CXX20_CONSTEXPR const_iterator find(const key_type &key) const { return m_table.template find<0>(key); }
		/** @copydoc find
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] TPP_CXX20_CONSTEXPR iterator find(const K &key) { return m_table.template find<0>(key); }
		/** @copydoc find */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] TPP_CXX20_CONSTEXPR const_iterator find(const K &key) const { return m_table.template find<0>(key); }

		/** Checks if the specified element is present within the map as if by `find(key) != end()`.
		 * @param key Key of the element to search for.
		 * @return `true` if the element is present within the map, `false` otherwise. */
		[[nodiscard]] TPP_CXX20_CONSTEXPR bool contains(const key_type &key) const { return m_table.template contains<0>(key); }
		/** @copydoc contains
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] TPP_CXX20_CONSTEXPR bool contains(const K &key) const { return m_table.template contains<0>(key); }

		/** Returns reference to the specified element.
		 * @param key Key of the element to search for.
		 * @return Reference to the specified element.
		 * @throw std::out_of_range If no such element exists within the map. */
		[[nodiscard]] reference at(const key_type &key) { return *guard_at(find(key)); }
		/** @copydoc at */
		[[nodiscard]] const_reference at(const key_type &key) const { return *guard_at(find(key)); }
		/** @copydoc at
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] reference at(const K &key) { return *guard_at(find(key)); }
		/** @copydoc at */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] const_reference at(const K &key) const { return *guard_at(find(key)); }

		/** Returns reference to the specified element. If the element is not present within the map, inserts a default-constructed instance.
		 * @param key Key of the element to search for.
		 * @return Reference to the specified element. */
		[[nodiscard]] TPP_CXX20_CONSTEXPR mapped_type &operator[](const key_type &key) { return try_emplace(key).first->second; }
		/** @copydoc operator[] */
		[[nodiscard]] TPP_CXX20_CONSTEXPR mapped_type &operator[](key_type &key) { return try_emplace(std::forward<key_type>(key)).first->second; }
		/** @copydoc operator[]
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] TPP_CXX20_CONSTEXPR mapped_type &operator[](const K &key) { return try_emplace(key).first->second; }
		/** @copydoc operator[] */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] TPP_CXX20_CONSTEXPR mapped_type &operator[](K &key) { return try_emplace(std::forward<K>(key)).first->second; }

		/** Returns forward iterator to the first element of the specified bucket. */
		[[nodiscard]] constexpr local_iterator begin(size_type n) noexcept { return m_table.begin(n); }
		/** @copydoc begin */
		[[nodiscard]] constexpr const_local_iterator begin(size_type n) const noexcept { return m_table.begin(n); }
		/** @copydoc begin */
		[[nodiscard]] constexpr const_local_iterator cbegin(size_type n) const noexcept { return m_table.begin(n); }
		/** Returns a sentinel iterator for the specified bucket. */
		[[nodiscard]] constexpr local_iterator end(size_type n) noexcept { return m_table.end(n); }
		/** @copydoc end */
		[[nodiscard]] constexpr const_local_iterator end(size_type n) const noexcept { return m_table.end(n); }
		/** @copydoc end */
		[[nodiscard]] constexpr const_local_iterator cend(size_type n) const noexcept { return m_table.end(n); }

		/** Returns the bucket index of the specified element. */
		[[nodiscard]] TPP_CXX20_CONSTEXPR size_type bucket(const key_type &key) const { return m_table.bucket(key); }
		/** @copydoc bucket
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] TPP_CXX20_CONSTEXPR size_type bucket(const K &key) const { return m_table.bucket(key); }

		/** Returns the current amount of buckets of the map. */
		[[nodiscard]] constexpr size_type bucket_count() const noexcept { return m_table.bucket_count(); }
		/** Returns the maximum amount of buckets of the map. */
		[[nodiscard]] constexpr size_type max_bucket_count() const noexcept { return m_table.max_bucket_count(); }
		/** Returns the amount of elements within the specified bucket. */
		[[nodiscard]] constexpr size_type bucket_size(size_type n) const noexcept { return m_table.bucket_size(n); }

		/** Reserves space for at least `n` elements. */
		TPP_CXX20_CONSTEXPR void reserve(size_type n) { m_table.reserve(n); }
		/** Reserves space for at least `n` buckets and rehashes the map if necessary.
		 * @note The new amount of buckets is clamped to be at least `size() / max_load_factor()`. */
		TPP_CXX20_CONSTEXPR void rehash(size_type n) { m_table.rehash(n); }
		/** Returns the current maximum load factor. */
		[[nodiscard]] constexpr float max_load_factor() const noexcept { return m_table.max_load_factor(); }
		/** Sets the current maximum load factor. */
		constexpr void max_load_factor(float f) noexcept { m_table.max_load_factor(f); }

		[[nodiscard]] TPP_CXX20_CONSTEXPR allocator_type get_allocator() const { return allocator_type{m_table.get_allocator()}; }
		[[nodiscard]] TPP_CXX20_CONSTEXPR hasher hash_function() const { return m_table.get_hash(); }
		[[nodiscard]] TPP_CXX20_CONSTEXPR key_equal key_eq() const { return m_table.get_cmp(); }

		[[nodiscard]] TPP_CXX20_CONSTEXPR bool operator==(const ordered_dense_map &other) const
		{
			return std::is_permutation(begin(), end(), other.begin(), other.end());
		}
#if (__cplusplus < 202002L || _MSVC_LANG < 202002L)
		[[nodiscard]] TPP_CXX20_CONSTEXPR bool operator!=(const ordered_dense_map &other) const
		{
			return !std::is_permutation(begin(), end(), other.begin(), other.end());
		}
#endif

		TPP_CXX20_CONSTEXPR void swap(ordered_dense_map &other) noexcept(std::is_nothrow_swappable_v<table_t>) { m_table.swap(other.m_table); }

	private:
		template<typename I>
		[[nodiscard]] inline auto guard_at(I iter) const
		{
			if (iter == end())
				throw std::out_of_range("`dense_map::at` - invalid key");
			else
				return iter;
		}

		table_t m_table;
	};

	/** Erases all elements from the map \p map that satisfy the predicate \p pred.
	 * @return Amount of elements erased. */
	template<typename K, typename M, typename H, typename C, typename A, typename P>
	TPP_CXX20_CONSTEXPR typename ordered_dense_map<K, M, H, C, A>::size_type erase_if(ordered_dense_map<K, M, H, C, A> &map, P pred)
	{
		typename dense_map<K, M, H, C, A>::size_type result = 0;
		for (auto i = map.cbegin(), last = map.cend(); i != last;)
		{
			if (pred(*i))
			{
				i = map.erase(i);
				++result;
			}
			else
				++i;
		}
		return result;
	}

	template<typename K, typename M, typename H, typename C, typename A>
	TPP_CXX20_CONSTEXPR void swap(ordered_dense_map<K, M, H, C, A> &a, ordered_dense_map<K, M, H, C, A> &b)
	noexcept(std::is_nothrow_swappable_v<ordered_dense_map<K, M, H, C, A>>) { a.swap(b); }
}