/*
 * Created by switchblade on 2022-11-5.
 */

#pragma once

#include "detail/dense_table.hpp"
#include "detail/multikey.hpp"

namespace tpp
{
	template<typename Mk, typename Mapped, typename = detail::multikey_hash<Mk>, typename = detail::multikey_eq<Mk>, typename = detail::multikey_alloc_t<Mk, Mapped>>
	class dense_multimap;

	/** @brief Hash multimap based on dense hash table.
	 *
	 * Multimap is a one-to-one hashtable container that allows the user to associate an entry with multiple independent keys.
	 * Internally, dense multimap stores it's elements in a contiguous unordered vector. Insert and erase operations on a
	 * dense multimap may invalidate references to it's elements due to the internal element vector being reordered.<br><br>
	 * Dense multimap iterators return a pair of references, as opposed to reference to a pair like STL maps do.
	 * This is required as the internal storage of dense multimap elements can be reordered, and as such elements are
	 * stored as `std::pair<std::tuple<Keys...>, Mapped>` instead of `std::pair<const std::tuple<Keys...>, Mapped>`
	 * to enable move-assignment and avoid copies. Because of this, elements must be converted to the const-qualified
	 * representation later on. Since a reference to `std::pair<T0, T1>` cannot be implicitly (and safely) converted
	 * to a reference to `std::pair<const T0, T1>`, this conversion is preformed element-wise, and a pair of references
	 * is returned instead.
	 *
	 * @tparam Keys Key types of the multimap.
	 * @tparam Mapped Mapped type associated with multimap keys.
	 * @tparam KeyHash Hash functor used by the multimap. The functor must be invocable for all key types.
	 * @tparam KeyCmp Compare functor used by the multimap. The functor must be invocable for all key types.
	 * @tparam Alloc Allocator used by the multimap. */
	template<typename... Keys, typename Mapped, typename KeyHash, typename KeyCmp, typename Alloc>
	class dense_multimap<multikey<Keys...>, Mapped, KeyHash, KeyCmp, Alloc>
	{
	public:
		using key_type = std::tuple<Keys...>;
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

			template<std::size_t I, typename T>
			static constexpr auto &get_key(T &value) noexcept { return std::get<I>(value.first); }
			template<typename T>
			static constexpr auto &get_key(T &value) noexcept { return value.first; }

			template<typename T>
			static constexpr auto &get_mapped(T &value) noexcept { return value.second; }

			static constexpr std::size_t key_size = std::tuple_size_v<key_type>;
		};

		using table_t = detail::dense_table<insert_type, value_type, key_type, KeyHash, KeyCmp, Alloc, traits_t>;

	public:
		using reference = typename table_t::reference;
		using const_reference = typename table_t::const_reference;
		using pointer = typename table_t::pointer;
		using const_pointer = typename table_t::const_pointer;

		using iterator = typename table_t::iterator;
		using const_iterator = typename table_t::const_iterator;
		using reverse_iterator = typename table_t::reverse_iterator;
		using const_reverse_iterator = typename table_t::const_reverse_iterator;
		using local_iterator = typename table_t::local_iterator;
		using const_local_iterator = typename table_t::const_local_iterator;

		using size_type = typename table_t::size_type;
		using difference_type = typename table_t::difference_type;

		using hasher = typename table_t::hasher;
		using key_equal = typename table_t::key_equal;
		using allocator_type = typename table_t::allocator_type;

		static constexpr auto key_size = table_t::key_size;

	public:
		/** Initializes the multimap with default capacity. */
		dense_multimap() = default;
		/** Initializes the multimap with default capacity using the specified allocator. */
		explicit dense_multimap(const allocator_type &alloc) : m_table(alloc) {}

		/** Copy-constructs the multimap. */
		dense_multimap(const dense_multimap &other) = default;
		/** Copy-constructs the multimap using the specified allocator. */
		dense_multimap(const dense_multimap &other, const allocator_type &alloc) : m_table(other.m_table, alloc) {}

		/** Move-constructs the multimap. */
		dense_multimap(dense_multimap &&other) noexcept(std::is_nothrow_move_constructible_v<table_t>) = default;
		/** Move-constructs the multimap using the specified allocator. */
		dense_multimap(dense_multimap &&other, const allocator_type &alloc) noexcept(std::is_nothrow_constructible_v<table_t, table_t &&, allocator_type>) : m_table(std::move(other.m_table), alloc) {}

		/** Initializes the multimap with the specified bucket count, hasher, comparator and allocator. */
		explicit dense_multimap(size_type bucket_count, const hasher &hash = hasher{}, const key_equal &cmp = key_equal{}, const allocator_type &alloc = allocator_type{}) : m_table(bucket_count, hash, cmp, alloc) {}
		/** Initializes the multimap with the specified bucket count, hasher and allocator. */
		dense_multimap(size_type bucket_count, const hasher &hash, const allocator_type &alloc) : dense_multimap(bucket_count, hash, key_equal{}, alloc) {}
		/** Initializes the multimap with the specified bucket count and allocator. */
		dense_multimap(size_type bucket_count, const allocator_type &alloc) : dense_multimap(bucket_count, hasher{}, alloc) {}

		/** Initializes the multimap with an initializer list of elements and the specified bucket count, hasher, comparator and allocator. */
		dense_multimap(std::initializer_list<value_type> il, size_type bucket_count = 0, const hasher &hash = hasher{}, const key_equal &cmp = key_equal{}, const allocator_type &alloc = allocator_type{})
				: dense_multimap(il.begin(), il.end(), bucket_count, hash, cmp, alloc) {}
		/** @copydoc dense_multimap */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		dense_multimap(std::initializer_list<T> il, size_type bucket_count = 0, const hasher &hash = hasher{}, const key_equal &cmp = key_equal{}, const allocator_type &alloc = allocator_type{})
				: dense_multimap(il.begin(), il.end(), bucket_count, hash, cmp, alloc) {}

		/** Initializes the multimap with an initializer list of elements and the specified bucket count, hasher and allocator. */
		dense_multimap(std::initializer_list<value_type> il, size_type bucket_count, const hasher &hash, const allocator_type &alloc) : dense_multimap(il.begin(), il.end(), bucket_count, hash, key_equal{}, alloc) {}
		/** @copydoc dense_multimap */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		dense_multimap(std::initializer_list<T> il, size_type bucket_count, const hasher &hash, const allocator_type &alloc) : dense_multimap(il.begin(), il.end(), bucket_count, hash, key_equal{}, alloc) {}

		/** Initializes the multimap with an initializer list of elements and the specified bucket count and allocator. */
		dense_multimap(std::initializer_list<value_type> il, size_type bucket_count, const allocator_type &alloc) : dense_multimap(il.begin(), il.end(), bucket_count, hasher{}, alloc) {}
		/** @copydoc dense_multimap */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		dense_multimap(std::initializer_list<T> il, size_type bucket_count, const allocator_type &alloc) : dense_multimap(il.begin(), il.end(), bucket_count, hasher{}, alloc) {}

		/** Initializes the multimap with a range of elements and the specified bucket count, hasher, comparator and allocator. */
		template<typename I>
		dense_multimap(I first, I last, size_type bucket_count = 0, const hasher &hash = hasher{}, const key_equal &cmp = key_equal{}, const allocator_type &alloc = allocator_type{})
				: m_table(first, last, bucket_count, hash, cmp, alloc) {}
		/** Initializes the multimap with a range of elements and the specified bucket count, hasher and allocator. */
		template<typename I>
		dense_multimap(I first, I last, size_type bucket_count, const hasher &hash, const allocator_type &alloc) : dense_multimap(first, last, bucket_count, hash, key_equal{}, alloc) {}
		/** Initializes the multimap with a range of elements and the specified bucket count and allocator. */
		template<typename I>
		dense_multimap(I first, I last, size_type bucket_count, const allocator_type &alloc) : dense_multimap(first, last, bucket_count, hasher{}, alloc) {}

		/** Copy-assigns the multimap. */
		dense_multimap &operator=(const dense_multimap &) = default;
		/** Move-assigns the multimap. */
		dense_multimap &operator=(dense_multimap &&) noexcept(std::is_nothrow_move_assignable_v<table_t>) = default;

		/** Replaces elements of the multimap with elements of the initializer list. */
		dense_multimap &operator=(std::initializer_list<value_type> il)
		{
			m_table.assign(il.begin(), il.end());
			return *this;
		}

		/** Returns iterator to the first element of the multimap.
		 * @note Elements are stored in no particular order. */
		/** @copydoc begin */
		[[nodiscard]] constexpr iterator begin() noexcept { return m_table.begin(); }
		/** @copydoc begin */
		[[nodiscard]] constexpr const_iterator begin() const noexcept { return m_table.begin(); }
		/** @copydoc begin */
		[[nodiscard]] constexpr const_iterator cbegin() const noexcept { return begin(); }
		/** Returns iterator one past the last element of the multimap.
		 * @note Elements are stored in no particular order. */
		[[nodiscard]] constexpr iterator end() noexcept { return m_table.end(); }
		/** @copydoc end */
		[[nodiscard]] constexpr const_iterator end() const noexcept { return m_table.end(); }
		/** @copydoc end */
		[[nodiscard]] constexpr const_iterator cend() const noexcept { return end(); }

		/** Returns reverse iterator to the last element of the multimap.
		 * @note Elements are stored in no particular order. */
		[[nodiscard]] constexpr reverse_iterator rbegin() noexcept { return m_table.rbegin(); }
		/** @copydoc rbegin */
		[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return m_table.rbegin(); }
		/** @copydoc rbegin */
		[[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept { return rbegin(); }
		/** Returns reverse iterator one past the first element of the multimap.
		 * @note Elements are stored in no particular order. */
		[[nodiscard]] constexpr reverse_iterator rend() noexcept { return m_table.rend(); }
		/** @copydoc rend */
		[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return m_table.rend(); }
		/** @copydoc rend */
		[[nodiscard]] constexpr const_reverse_iterator crend() const noexcept { return rend(); }

		/** Returns the total number of elements within the multimap. */
		[[nodiscard]] constexpr size_type size() const noexcept { return m_table.size(); }
		/** Checks if the multimap is empty (`size() == 0`). */
		[[nodiscard]] constexpr bool empty() const noexcept { return m_table.size() == 0; }
		/** Returns the current capacity of the multimap (taking into account the maximum load factor). */
		[[nodiscard]] constexpr size_type capacity() const noexcept { return m_table.capacity(); }
		/** Returns the absolute maximum size of the multimap (taking into account the maximum load factor). */
		[[nodiscard]] constexpr size_type max_size() const noexcept { return m_table.max_size(); }
		/** Returns the current load factor of the multimap as if via `size() / bucket_count()`. */
		[[nodiscard]] constexpr float load_factor() const noexcept { return m_table.load_factor(); }

		/** Erases all elements from the multimap. */
		void clear() { m_table.clear(); }

		/** @brief Inserts an element (of `value_type`) into the multimap if it does not exist yet.
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

		/** Inserts all elements from the range `[first, last)` into the multimap.
		 * @param first Iterator to the first element of the source range.
		 * @param last Iterator one past the last element of the source range. */
		template<typename I>
		void insert(I first, I last) { return m_table.insert(first, last); }

		/** Inserts all elements of an initializer list into the multimap. */
		void insert(std::initializer_list<value_type> il) { return insert(il.begin(), il.end()); }
		/** @copydoc insert */
		template<typename T, typename = std::enable_if_t<std::is_constructible_v<value_type, const T &>>>
		void insert(std::initializer_list<T> il) { return insert(il.begin(), il.end()); }

		/** @brief Inserts an in-place constructed element (of `value_type`) into the multimap if it does not exist yet.
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

		/** Attempts to emplace piecewise constructed element (of `value_type`) at the specified key into the map  if it does not exist yet.
		 * @param key Key of the element to insert.
		 * @param args Arguments passed to constructor of `mapped_type`.
		 * @return Pair where `first` is the iterator to the inserted or existing element, and `second` is a boolean
		 * indicating whether insertion took place (`true` if element was inserted, `false` otherwise). */
		template<typename... Args>
		std::pair<iterator, bool> try_emplace(const key_type &key, Args &&...args)
		{
			return m_table.try_emplace(key, std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace */
		template<typename... Args>
		std::pair<iterator, bool> try_emplace(key_type &&key, Args &&...args)
		{
			return m_table.try_emplace(std::move(key), std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename... Ks, typename... Args, typename = std::enable_if_t<
				table_t::is_transparent::value && std::conjunction_v<std::is_invocable<hasher, Ks>...>>>
		std::pair<iterator, bool> try_emplace(std::tuple<Ks...> key, Args &&...args)
		{
			return m_table.try_emplace(std::move(key), std::forward<Args>(args)...);
		}

		/** @copybrief try_emplace
		 * @param key Key of the element to insert.
		 * @param args Arguments passed to constructor of `mapped_type`.
		 * @return Iterator to the inserted or existing element.
		 * @note \p hint has no effect, this overload exists for API compatibility. */
		template<typename... Args>
		iterator try_emplace(const_iterator hint, const key_type &key, Args &&...args)
		{
			return m_table.try_emplace(hint, key, std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace */
		template<typename... Args>
		iterator try_emplace(iterator hint, const key_type &key, Args &&...args)
		{
			return try_emplace(const_iterator{hint}, key, std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace */
		template<typename... Args>
		iterator try_emplace(const_iterator hint, key_type &&key, Args &&...args)
		{
			return m_table.try_emplace(hint, std::move(key), std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace */
		template<typename... Args>
		iterator try_emplace(iterator hint, key_type &&key, Args &&...args)
		{
			return try_emplace(const_iterator{hint}, std::move(key), std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<typename... Ks, typename... Args, typename = std::enable_if_t<
				table_t::is_transparent::value && std::conjunction_v<std::is_invocable<hasher, Ks>...>>>
		iterator try_emplace(const_iterator hint, std::tuple<Ks...> key, Args &&...args)
		{
			return m_table.try_emplace(hint, std::move(key), std::forward<Args>(args)...);
		}
		/** @copydoc try_emplace */
		template<typename... Ks, typename... Args, typename = std::enable_if_t<
				table_t::is_transparent::value && std::conjunction_v<std::is_invocable<hasher, Ks>...>>>
		iterator try_emplace(iterator hint, std::tuple<Ks...> key, Args &&...args)
		{
			return try_emplace(const_iterator{hint}, std::move(key), std::forward<Args>(args)...);
		}

		/** Removes the specified element from the multimap.
		 * @tparam I Index of the key.
		 * @param key `I`th key of the element to remove.
		 * @return Iterator to the element following the erased one, or `end()`. */
		template<std::size_t I>
		iterator erase(const std::tuple_element_t<I, key_type> &key) { return m_table.template erase<I>(key); }
		/** @copydoc erase
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<std::size_t I, typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		iterator erase(const K &key) { return m_table.template erase<I>(key); }

		/** Removes the specified element from the multimap.
		 * @param pos Iterator pointing to the element to remove.
		 * @return Iterator to the element following the erased one, or `end()`. */
		iterator erase(const_iterator pos) { return m_table.erase(pos); }
		/** @copydoc erase */
		iterator erase(iterator pos) { return erase(const_iterator{pos}); }

		/** Removes a range of elements from the multimap.
		 * @param first Iterator to the first element of the to-be removed range.
		 * @param last Iterator one past the last element of the to-be removed range.
		 * @return Iterator to the element following the erased range, or `end()`. */
		iterator erase(const_iterator first, const_iterator last) { return m_table.erase(first, last); }

		/** Searches for the specified element within the multimap.
		 * @tparam I Index of the key.
		 * @param key `I`th key of the element to search for.
		 * @return Iterator to the specified element, or `end()`. */
		template<std::size_t I>
		[[nodiscard]] iterator find(const std::tuple_element_t<I, key_type> &key) { return m_table.template find<I>(key); }
		/** @copydoc find */
		template<std::size_t I>
		[[nodiscard]] const_iterator find(const std::tuple_element_t<I, key_type> &key) const { return m_table.template find<I>(key); }
		/** @copydoc find
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<std::size_t I, typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] iterator find(const K &key) { return m_table.template find<I>(key); }
		/** @copydoc find */
		template<std::size_t I, typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] const_iterator find(const K &key) const { return m_table.template find<I>(key); }

		/** Checks if the specified element is present within the multimap as if by `find(key) != end()`.
		 * @tparam I Index of the key.
		 * @param key `I`th key of the element to search for.
		 * @return `true` if the element is present within the multimap, `false` otherwise. */
		template<std::size_t I>
		[[nodiscard]] bool contains(const std::tuple_element_t<I, key_type> &key) const { return m_table.template contains<I>(key); }
		/** @copydoc contains
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<std::size_t I, typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] bool contains(const K &key) const { return m_table.template contains<I>(key); }

		/** Returns reference to the specified element.
		 * @tparam I Index of the key.
		 * @param key `I`th key of the element to search for.
		 * @return Reference to the specified element.
		 * @throw std::out_of_range If no such element exists within the multimap. */
		template<std::size_t I>
		[[nodiscard]] mapped_type &at(const std::tuple_element_t<I, key_type> &key) { return guard_at(find<I>(key))->second; }
		/** @copydoc at */
		template<std::size_t I>
		[[nodiscard]] const mapped_type &at(const std::tuple_element_t<I, key_type> &key) const { return guard_at(find<I>(key))->second; }
		/** @copydoc at
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<std::size_t I, typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] mapped_type &at(const K &key) { return guard_at(find<I>(key))->second; }
		/** @copydoc at */
		template<std::size_t I, typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] const mapped_type &at(const K &key) const { return guard_at(find<I>(key))->second; }

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
		template<std::size_t I>
		[[nodiscard]] size_type bucket(const std::tuple_element_t<I, key_type> &key) const { return m_table.bucket(key); }
		/** @copydoc bucket
		 * @note This overload is available only if the hash & compare functors are transparent. */
		template<std::size_t I, typename K, typename = std::enable_if_t<table_t::is_transparent::value && std::is_invocable_v<hasher, K>>>
		[[nodiscard]] size_type bucket(const K &key) const { return m_table.bucket(key); }

		/** Returns the current amount of buckets of the multimap. */
		[[nodiscard]] constexpr size_type bucket_count() const noexcept { return m_table.bucket_count(); }
		/** Returns the maximum amount of buckets of the multimap. */
		[[nodiscard]] constexpr size_type max_bucket_count() const noexcept { return m_table.max_bucket_count(); }
		/** Returns the amount of elements within the specified bucket. */
		[[nodiscard]] constexpr size_type bucket_size(size_type n) const noexcept { return m_table.bucket_size(n); }

		/** Reserves space for at least `n` elements. */
		void reserve(size_type n) { m_table.reserve(n); }
		/** Reserves space for at least `n` buckets and rehashes the multimap if necessary.
		 * @note The new amount of buckets is clamped to be at least `size() / max_load_factor()`. */
		void rehash(size_type n) { m_table.rehash(n); }
		/** Returns the current maximum load factor. */
		[[nodiscard]] constexpr float max_load_factor() const noexcept { return m_table.max_load_factor(); }
		/** Sets the current maximum load factor. */
		constexpr void max_load_factor(float f) noexcept { m_table.max_load_factor(f); }

		[[nodiscard]] allocator_type get_allocator() const { return allocator_type{m_table.get_allocator()}; }
		[[nodiscard]] hasher hash_function() const { return m_table.get_hash(); }
		[[nodiscard]] key_equal key_eq() const { return m_table.get_cmp(); }

		[[nodiscard]] bool operator==(const dense_multimap &other) const
		{
			return std::is_permutation(begin(), end(), other.begin(), other.end());
		}
#if (__cplusplus < 202002L && (!defined(_MSVC_LANG) || _MSVC_LANG < 202002L))
		[[nodiscard]] bool operator!=(const dense_multimap &other) const
		{
			return !std::is_permutation(begin(), end(), other.begin(), other.end());
		}
#endif

		void swap(dense_multimap &other) noexcept(std::is_nothrow_swappable_v<table_t>) { m_table.swap(other.m_table); }

	private:
		template<typename I>
		[[nodiscard]] inline auto guard_at(I iter) const
		{
			if (const_iterator{iter} == end())
				throw std::out_of_range("`dense_multimap::at` - invalid key");
			else
				return iter;
		}

		table_t m_table;
	};

	/** Erases all elements from the map \p map that satisfy the predicate \p pred.
	 * @return Amount of elements erased. */
	template<typename Mk, typename M, typename H, typename C, typename A, typename P>
	inline typename dense_multimap<Mk, M, H, C, A>::size_type erase_if(dense_multimap<Mk, M, H, C, A> &map, P pred)
	{
		typename dense_multimap<Mk, M, H, C, A>::size_type result = 0;
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

	template<typename Mk, typename M, typename H, typename C, typename A>
	inline void swap(dense_multimap<Mk, M, H, C, A> &a, dense_multimap<Mk, M, H, C, A> &b) noexcept(std::is_nothrow_swappable_v<dense_multimap<Mk, H, C, A>>)
	{
		a.swap(b);
	}
}