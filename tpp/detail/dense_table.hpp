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
	template<typename V, typename K, typename KHash, typename KCmp, typename Alloc>
	struct dense_table_traits : table_traits<V, K, Alloc>
	{
		typedef KHash key_hash;
		typedef KCmp key_equal;
	};

	template<typename V, typename K, typename KGet, typename Alloc, typename Link>
	struct dense_bucket_node : Link, ebo_container<V>
	{
		using value_base = ebo_container<V>;

		using size_type = typename table_traits<V, K, Alloc>::size_type;
		using difference_type = typename table_traits<V, K, Alloc>::difference_type;

		constexpr static auto npos = table_traits<V, K, Alloc>::npos;

		using value_base::value;

		template<typename... Args>
		constexpr dense_bucket_node(Args &&...args) noexcept(nothrow_ctor<V, Args...>) : value_base(std::forward<Args>(args)...) {}

		template<typename T>
		constexpr void replace(T &&new_value, std::size_t new_hash)
		{
			/* If move-assign is possible, do that. Otherwise, re-init the value. */
			if constexpr (std::is_assignable_v<V, T &&>)
				value() = std::forward<T>(new_value);
			else
			{
				std::destroy_at(get());
				std::construct_at(get(), std::forward<T>(new_value));
			}
			hash = new_hash;
		}

		[[nodiscard]] constexpr V *get() noexcept { return &value(); }
		[[nodiscard]] constexpr const V *get() const noexcept { return &value(); }
		[[nodiscard]] constexpr const K &key() const noexcept { return KGet{}(value()); }

		std::size_t hash = 0;
		size_type next = npos;
	};

	template<typename V, typename K, typename KHash, typename KCmp, typename KGet, typename Alloc, typename Link = empty_link>
	class dense_table : Link, ebo_container<KHash>, ebo_container<KCmp>
	{
		using traits_t = dense_table_traits<V, K, KHash, KCmp, Alloc>;

	public:
		typedef typename traits_t::value_type value_type;
		typedef typename traits_t::key_type key_type;

		typedef typename traits_t::key_hash key_hash;
		typedef typename traits_t::key_equal key_equal;
		typedef typename traits_t::allocator_type allocator_type;

		typedef std::conjunction<detail::is_transparent<key_hash>, detail::is_transparent<key_equal>> is_transparent;
		typedef detail::is_ordered<Link> is_ordered;

		constexpr static auto initial_load_factor = traits_t::initial_load_factor;
		constexpr static auto initial_capacity = traits_t::initial_capacity;
		constexpr static auto npos = traits_t::npos;

	private:
		using bucket_node = dense_bucket_node<value_type, key_type, KGet, allocator_type, Link>;
		using bucket_link = Link;

		using dense_alloc_t = typename std::allocator_traits<Alloc>::template rebind_alloc<bucket_node>;
		using dense_t = std::vector<bucket_node, dense_alloc_t>;

	public:
		typedef typename dense_t::size_type size_type;
		typedef typename dense_t::difference_type difference_type;

	private:
		using sparse_alloc_t = typename std::allocator_traits<Alloc>::template rebind_alloc<size_type>;
		using sparse_t = std::vector<size_type, sparse_alloc_t>;

		using hash_base = ebo_container<key_hash>;
		using cmp_base = ebo_container<key_equal>;
		using header_base = bucket_link;

		using node_iterator = std::conditional_t<is_ordered::value, ordered_iterator<bucket_node>, bucket_node *>;
		using const_node_iterator = std::conditional_t<is_ordered::value, ordered_iterator<const bucket_node>, const bucket_node *>;

	public:

		typedef table_iterator<value_type, node_iterator> iterator;
		typedef table_iterator<const value_type, const_node_iterator> const_iterator;
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	private:
		[[nodiscard]] constexpr static auto to_iter(bucket_node *node) noexcept { return iterator{node_iterator{node}}; }
		[[nodiscard]] constexpr static auto to_iter(const bucket_node *node) noexcept { return const_iterator{const_node_iterator{node}}; }

	public:
		constexpr dense_table() = default;

		constexpr dense_table(const dense_table &other)
				: hash_base(other), cmp_base(other), header_base(other),
				  m_sparse(other.m_sparse),
				  m_dense(other.m_dense),
				  max_load_factor(other.max_load_factor) {}
		constexpr dense_table(const dense_table &other, const allocator_type &alloc)
				: hash_base(other), cmp_base(other), header_base(other),
				  m_sparse(other.m_sparse, sparse_alloc_t{alloc}),
				  m_dense(other.m_dense, dense_alloc_t{alloc}),
				  max_load_factor(other.max_load_factor) {}

		constexpr dense_table(dense_table &&other) noexcept
				: hash_base(std::move(other)), cmp_base(std::move(other)), header_base(std::move(other)),
				  m_sparse(std::move(other.m_sparse)),
				  m_dense(std::move(other.m_dense)),
				  max_load_factor(other.max_load_factor) {}
		constexpr dense_table(dense_table &&other, const allocator_type &alloc) noexcept
				: hash_base(std::move(other)), cmp_base(std::move(other)), header_base(std::move(other)),
				  m_sparse(std::move(other.m_sparse), sparse_alloc_t{alloc}),
				  m_dense(std::move(other.m_dense), dense_alloc_t{alloc}),
				  max_load_factor(other.max_load_factor) {}

		constexpr dense_table(const allocator_type &alloc)
				: m_sparse(initial_capacity, npos, sparse_alloc_t{alloc}),
				  m_dense(dense_alloc_t{alloc}) {}
		constexpr dense_table(size_type bucket_count, const key_hash &hash, const key_equal &cmp, const allocator_type &alloc)
				: hash_base(hash), cmp_base(cmp),
				  m_sparse(bucket_count, npos, sparse_alloc_t{alloc}),
				  m_dense(dense_alloc_t{alloc}) {}

		template<typename I>
		constexpr dense_table(I first, I last, size_type bucket_count, const key_hash &hash, const key_equal &cmp, const allocator_type &alloc)
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
				max_load_factor = other.max_load_factor;
			}
			return *this;
		}
		constexpr dense_table &operator=(dense_table &&other)
		noexcept(nothrow_assign<key_hash, key_hash &&> && nothrow_assign<key_equal, key_equal &&> &&
		         nothrow_assign<sparse_t, sparse_t &&> && nothrow_assign<dense_t, dense_t &&>)
		{
			if (this != &other)
			{
				header_base::operator=(std::move(other));
				hash_base::operator=(std::move(other));
				cmp_base::operator=(std::move(other));

				m_sparse = std::move(other.m_sparse);
				m_dense = std::move(other.m_dense);
				max_load_factor = other.max_load_factor;
			}
			return *this;
		}

		[[nodiscard]] constexpr iterator begin() noexcept { return to_iter(begin_node()); }
		[[nodiscard]] constexpr const_iterator begin() const noexcept { return to_iter(begin_node()); }
		[[nodiscard]] constexpr iterator end() noexcept { return to_iter(end_node()); }
		[[nodiscard]] constexpr const_iterator end() const noexcept { return to_iter(end_node()); }

		[[nodiscard]] constexpr reverse_iterator rbegin() noexcept { return reverse_iterator{end()}; }
		[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator{end()}; }
		[[nodiscard]] constexpr reverse_iterator rend() noexcept { return reverse_iterator{begin()}; }
		[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator{begin()}; }

		[[nodiscard]] constexpr value_type &front() noexcept { return front_node()->value(); }
		[[nodiscard]] constexpr const value_type &front() const noexcept { return front_node()->value(); }
		[[nodiscard]] constexpr value_type &back() noexcept { return back_node()->value(); }
		[[nodiscard]] constexpr const value_type &back() const noexcept { return back_node()->value(); }

		[[nodiscard]] constexpr size_type size() const noexcept { return m_dense.size(); }
		[[nodiscard]] constexpr size_type capacity() const noexcept { return to_capacity(bucket_count()); }
		[[nodiscard]] constexpr size_type max_size() const noexcept { return to_capacity(std::min(m_dense.max_size(), npos - 1)); }
		[[nodiscard]] constexpr float load_factor() const noexcept { return static_cast<float>(size()) / static_cast<float>(bucket_count()); }

		[[nodiscard]] constexpr size_type bucket_count() const noexcept { return m_sparse.size(); }
		[[nodiscard]] constexpr size_type max_bucket_count() const noexcept { return m_sparse.max_size(); }

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
			rehash(static_cast<size_type>(static_cast<float>(n) / max_load_factor));

			if constexpr (is_ordered::value)
			{
				/* Update header link only if it does not point to itself. */
				if (!m_dense.empty())
				{
					auto *new_front = m_dense.data() + front_pos;
					auto *new_back = m_dense.data() + back_pos;
					header_link()->relink(new_front, new_back);
				}
			}
		}

		[[nodiscard]] constexpr bool contains(const auto &key) const noexcept { return *find_node(hash(key), key).second != npos; }

		[[nodiscard]] constexpr iterator find(const auto &key) noexcept { return to_iter(find_node(hash(key), key).first); }
		[[nodiscard]] constexpr const_iterator find(const auto &key) const noexcept { return to_iter(find_node(hash(key), key).first); }

		constexpr std::pair<iterator, bool> insert(const value_type &value) { return insert_impl({}, KGet{}(value), value); }
		constexpr std::pair<iterator, bool> insert(value_type &&value) { return insert_impl({}, KGet{}(value), std::move(value)); }
		constexpr iterator insert(const_iterator hint, const value_type &value) { return insert_impl(to_underlying(hint), KGet{}(value), value).first; }
		constexpr iterator insert(const_iterator hint, value_type &&value) { return insert_impl(to_underlying(hint), KGet{}(value), std::move(value)).first; }

		template<typename T, typename = std::enable_if_t<!std::is_convertible_v<T &&, value_type &&>>>
		constexpr std::pair<iterator, bool> insert(T &&value) TPP_REQUIRES((std::is_constructible_v<V, T>))
		{
			return emplace_impl({}, std::forward<T>(value));
		}
		template<typename T, typename = std::enable_if_t<!std::is_convertible_v<T &&, value_type &&>>>
		constexpr iterator insert(const_iterator hint, T &&value) TPP_REQUIRES((std::is_constructible_v<V, T>))
		{
			return emplace_impl(to_underlying(hint), std::forward<T>(value)).first;
		}

		template<typename T, typename U, typename = std::enable_if_t<!std::is_same_v<std::decay_t<K>, std::decay_t<V>>>>
		constexpr std::pair<iterator, bool> insert_or_assign(const T &key, U &&value) TPP_REQUIRES((std::is_constructible_v<V, T, U>))
		{
			return insert_or_assign_impl({}, key, std::forward<U>(value));
		}
		template<typename T, typename U, typename = std::enable_if_t<!std::is_same_v<std::decay_t<K>, std::decay_t<V>>>>
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

		template<typename... Args, typename = std::enable_if_t<!std::is_same_v<std::decay_t<K>, std::decay_t<V>>>>
		constexpr std::pair<iterator, bool> emplace_or_replace(Args &&...args) TPP_REQUIRES((std::is_constructible_v<V, Args...>))
		{
			return emplace_or_replace_impl({}, std::forward<Args>(args)...);
		}
		template<typename... Args, typename = std::enable_if_t<!std::is_same_v<std::decay_t<K>, std::decay_t<V>>>>
		constexpr iterator emplace_or_replace(const_iterator hint, Args &&...args) TPP_REQUIRES((std::is_constructible_v<V, Args...>))
		{
			return emplace_or_replace_impl(to_underlying(hint), std::forward<Args>(args)...).first;
		}

		template<typename U, typename... Args, typename = std::enable_if_t<!std::is_same_v<std::decay_t<K>, std::decay_t<V>>>>
		constexpr std::pair<iterator, bool> try_emplace(U &&key, Args &&...args) TPP_REQUIRES(
				(std::is_constructible_v<V, std::piecewise_construct_t, std::tuple<U &&>, std::tuple<Args && ...>>))
		{
			return emplace_impl({}, hash(key), key, std::piecewise_construct,
			                    std::forward_as_tuple(std::forward<U>(key)),
			                    std::forward_as_tuple(std::forward<Args>(args)...));
		}
		template<typename U, typename... Args, typename = std::enable_if_t<!std::is_same_v<std::decay_t<K>, std::decay_t<V>>>>
		constexpr iterator try_emplace(const_iterator hint, U &&key, Args &&...args) TPP_REQUIRES(
				(std::is_constructible_v<V, std::piecewise_construct_t, std::tuple<U &&>, std::tuple<Args && ...>>))
		{
			return emplace_impl(to_underlying(hint), hash(key), key, std::piecewise_construct,
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

		constexpr void rehash(size_type new_cap)
		{
			using std::max;

			/* Adjust the capacity to be at least large enough to fit the current size. */
			new_cap = max(max(static_cast<size_type>(static_cast<float>(size()) / max_load_factor), new_cap), initial_capacity);
			if (new_cap != m_sparse.capacity()) rehash_impl(new_cap);
		}

		[[nodiscard]] constexpr auto allocator() const noexcept { return m_dense.get_allocator(); }
		[[nodiscard]] constexpr auto &get_hash() const noexcept { return hash_base::value(); }
		[[nodiscard]] constexpr auto &get_cmp() const noexcept { return cmp_base::value(); }

		constexpr void swap(dense_table &other)
		noexcept(std::is_nothrow_swappable_v<key_hash> && std::is_nothrow_swappable_v<key_equal> &&
		         std::is_nothrow_swappable_v<sparse_t> && std::is_nothrow_swappable_v<dense_t>)
		{
			key_hash::swap(other);
			key_equal::swap(other);

			std::swap(m_sparse, other.m_sparse);
			std::swap(m_dense, other.m_dense);
			std::swap(initial_load_factor, other.initial_load_factor);
		}

	private:
		[[nodiscard]] constexpr auto to_capacity(size_type n) const noexcept { return static_cast<size_type>(static_cast<float>(n) * max_load_factor); }

		[[nodiscard]] constexpr auto hash(const auto &k) const { return get_hash()(k); }
		[[nodiscard]] constexpr auto cmp(const auto &a, const auto &b) const { return get_cmp()(a, b); }

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

		/* Same reason for `const_cast` as with node getters above. */
		[[nodiscard]] constexpr auto *get_chain(std::size_t h) const noexcept { return const_cast<size_type *>(m_sparse.data()) + (h % bucket_count()); }
		[[nodiscard]] constexpr auto find_node(std::size_t h, const auto &key) const noexcept -> std::pair<bucket_node *, size_type *>
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

		constexpr iterator commit_node([[maybe_unused]] const_node_iterator hint, size_type pos, auto *chain_idx, std::size_t h, bucket_node *node)
		{
			/* Create the bucket and insertion order links. */
			if constexpr (is_ordered::value) node->link(hint.link ? hint.link : back_node());
			*chain_idx = pos;

			/* Initialize the hash & rehash the table. Doing it now so
			 * that any temporary nodes do not affect table hashing. */
			node->hash = h;
			maybe_rehash();

			return to_iter(node);
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
		template<typename... Args>
		constexpr auto emplace_or_replace_impl(const_node_iterator hint, Args &&...args) -> std::pair<iterator, bool>
		{
			/* Create a temporary object to check if it already exists within the table. */
			const auto pos = size();
			auto &tmp = m_dense.emplace_back(std::forward<Args>(args)...);
			const auto h = hash(tmp.key());

			/* If there is no conflict, commit the temporary to the table. Otherwise, replace the existing with the temporary. */
			if (const auto [candidate, chain_idx] = find_node(h, tmp.key()); *chain_idx == npos)
				return {commit_node(hint, pos, chain_idx, h, &tmp), true};
			else
			{
				candidate->replace(std::move(tmp.value()), h);
				m_dense.pop_back();
				return {to_iter(candidate), false};
			}
		}

		template<typename... Args>
		constexpr iterator emplace_at(const_node_iterator hint, auto *chain_idx, std::size_t h, Args &&...args)
		{
			const auto pos = size();
			auto &node = m_dense.emplace_back(std::forward<Args>(args)...);
			return commit_node(hint, pos, chain_idx, h, &node);
		}

		template<typename T>
		constexpr auto insert_impl(const_node_iterator hint, const auto &key, T &&value) -> std::pair<iterator, bool>
		{
			/* If a candidate was found, do nothing. Otherwise, emplace a new entry. */
			const auto h = hash(key);
			if (const auto [candidate, chain_idx] = find_node(h, key); *chain_idx == npos)
				return {emplace_at(hint, chain_idx, h, std::forward<T>(value)), true};
			else
				return {to_iter(candidate), false};
		}
		template<typename T>
		constexpr auto insert_or_assign_impl(const_node_iterator hint, const auto &key, T &&value) -> std::pair<iterator, bool>
		{
			/* If a candidate was found, replace the entry. Otherwise, emplace a new entry. */
			const auto h = hash(key);
			if (const auto [candidate, chain_idx] = find_node(h, key); *chain_idx == npos)
				return {emplace_at(hint, chain_idx, h, key, std::forward<T>(value)), true};
			else
			{
				candidate->replace(V{key, std::forward<T>(value)}, h);
				return {to_iter(candidate), false};
			}
		}

		constexpr iterator erase_impl(std::size_t h, const auto &key)
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
						*entry_ptr = std::move(m_dense.back());

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
			if (load_factor() > max_load_factor)
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

	public:
		float max_load_factor = initial_load_factor;
	};
}