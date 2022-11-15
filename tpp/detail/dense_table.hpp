/*
 * Created by switchblade on 11/7/22.
 */

#pragma once

#include "table_util.hpp"

#ifndef TPP_USE_IMPORT

#include <vector>
#include <limits>

#endif

namespace tpp::detail
{
	template<typename V, typename KGet, template<typename> typename L>
	struct dense_bucket_node : ebo_container<V>, ebo_container<L<dense_bucket_node<V, KGet, L>>>
	{
		using link_base = ebo_container<L<dense_bucket_node>>;
		using value_base = ebo_container<V>;

		using value_base::value;

		template<typename... Args>
		constexpr dense_bucket_node(Args &&...args) noexcept(nothrow_ctor<V, Args...>) : value_base(std::forward<Args>(args)...) {}

		[[nodiscard]] constexpr V *get() noexcept { return &value(); }
		[[nodiscard]] constexpr const V *get() const noexcept { return &value(); }
		[[nodiscard]] constexpr decltype(auto) key() const noexcept { return KGet{}(value()); }

		std::size_t hash = 0;
		std::size_t next = std::numeric_limits<std::size_t>::max();
	};

	template<typename V, typename K, typename KHash, typename KCmp, typename Alloc>
	struct dense_table_traits
	{
		typedef V value_type;
		typedef K key_type;

		typedef KHash key_hash;
		typedef KCmp key_equal;
		typedef Alloc allocator_type;

		typedef std::size_t size_type;
		typedef std::ptrdiff_t difference_type;

		constexpr static float initial_load_factor = .875f;

		constexpr static size_type npos = std::numeric_limits<size_type>::max();
		constexpr static size_type initial_capacity = 8;
	};

	template<typename V, typename K, typename KHash, typename KCmp, typename KGet, typename Alloc, template<typename> typename Link = empty_link>
	class dense_table : ebo_container<KHash>, ebo_container<KCmp>, table_header<dense_bucket_node<V, KGet, Link>, Link>
	{
		using traits_t = dense_table_traits<V, K, KHash, KCmp, Alloc>;

	public:
		typedef typename traits_t::value_type value_type;
		typedef typename traits_t::key_type key_type;

		typedef typename traits_t::key_hash key_hash;
		typedef typename traits_t::key_equal key_equal;
		typedef typename traits_t::allocator_type allocator_type;

		typedef typename traits_t::size_type size_type;
		typedef typename traits_t::difference_type difference_type;

		typedef std::conjunction<detail::is_transparent<key_hash>, detail::is_transparent<key_equal>> is_transparent;
		typedef detail::is_ordered<Link> is_ordered;

		using traits_t::initial_load_factor;
		using traits_t::initial_capacity;
		using traits_t::npos;

	private:
		using bucket_node = dense_bucket_node<value_type, KGet, Link>;
		using bucket_link = Link<bucket_node>;

		using sparse_alloc_t = typename std::allocator_traits<Alloc>::template rebind_alloc<size_type>;
		using sparse_t = std::vector<size_type, sparse_alloc_t>;

		using dense_alloc_t = typename std::allocator_traits<Alloc>::template rebind_alloc<size_type>;
		using dense_t = std::vector<bucket_node, dense_alloc_t>;

		using header_base = table_header<bucket_node, Link>;
		using hash_base = ebo_container<key_hash>;
		using cmp_base = ebo_container<key_equal>;

		using node_iterator = std::conditional_t<is_ordered::value, ordered_iterator<value_type, bucket_node>, bucket_node *>;
		using const_node_iterator = std::conditional_t<is_ordered::value, ordered_iterator<const value_type, bucket_node>, const bucket_node *>;

	public:
		typedef table_iterator<value_type, node_iterator> iterator;
		typedef table_iterator<const value_type, const_node_iterator> const_iterator;
		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	private:
		[[nodiscard]] constexpr static iterator to_iter(bucket_node *node) noexcept { return iterator{node_iterator{node}}; }
		[[nodiscard]] constexpr static const_iterator to_iter(const bucket_node *node) noexcept { return const_iterator{const_node_iterator{node}}; }

	public:
		constexpr dense_table() = default;

		constexpr dense_table(const dense_table &other)
				: hash_base(other), cmp_base(other),
				  m_sparse(other.m_sparse),
				  m_dense(other.m_dense),
				  max_load_factor(other.max_load_factor)
		{
			relink_header();
		}
		constexpr dense_table(const dense_table &other, const allocator_type &alloc)
				: hash_base(other), cmp_base(other),
				  m_sparse(other.m_sparse, sparse_alloc_t{alloc}),
				  m_dense(other.m_dense, dense_alloc_t{alloc}),
				  max_load_factor(other.max_load_factor)
		{
			relink_header();
		}

		constexpr dense_table(dense_table &&other) noexcept
				: hash_base(std::move(other)), cmp_base(std::move(other)),
				  m_sparse(std::move(other.m_sparse)),
				  m_dense(std::move(other.m_dense)),
				  max_load_factor(other.max_load_factor)
		{
			relink_header();
		}
		constexpr dense_table(dense_table &&other, const allocator_type &alloc) noexcept
				: hash_base(std::move(other)), cmp_base(std::move(other)),
				  m_sparse(std::move(other.m_sparse), sparse_alloc_t{alloc}),
				  m_dense(std::move(other.m_dense), dense_alloc_t{alloc}),
				  max_load_factor(other.max_load_factor)
		{
			relink_header();
		}

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
				hash_base::operator=(other);
				cmp_base::operator=(other);

				m_sparse = other.m_sparse;
				m_dense = other.m_dense;
				max_load_factor = other.max_load_factor;

				relink_header();
			}
			return *this;
		}
		constexpr dense_table &operator=(dense_table &&other)
		noexcept(nothrow_assign<key_hash, key_hash &&> && nothrow_assign<key_equal, key_equal &&> &&
		         nothrow_assign<sparse_t, sparse_t &&> && nothrow_assign<dense_t, dense_t &&>)
		{
			if (this != &other)
			{
				hash_base::operator=(std::move(other));
				cmp_base::operator=(std::move(other));

				m_sparse = std::move(other.m_sparse);
				m_dense = std::move(other.m_dense);
				max_load_factor = other.max_load_factor;

				relink_header();
			}
			return *this;
		}

		[[nodiscard]] constexpr iterator begin() noexcept
		{
			if constexpr (is_ordered::value)
				return iterator{node_iterator{header().next}};
			else
				return iterator{m_dense.data()};
		}
		[[nodiscard]] constexpr const_iterator begin() const noexcept
		{
			if constexpr (is_ordered::value)
				return const_iterator{node_iterator{header().next}};
			else
				return const_iterator{m_dense.data()};
		}
		[[nodiscard]] constexpr iterator end() noexcept
		{
			if constexpr (is_ordered::value)
				return iterator{node_iterator{&header()}};
			else
				return iterator{m_dense.data() + size()};
		}
		[[nodiscard]] constexpr const_iterator end() const noexcept
		{
			if constexpr (is_ordered::value)
				return const_iterator{node_iterator{&header()}};
			else
				return const_iterator{m_dense.data() + size()};
		}

		[[nodiscard]] constexpr reverse_iterator rbegin() noexcept { return reverse_iterator{end()}; }
		[[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept { return const_reverse_iterator{end()}; }
		[[nodiscard]] constexpr reverse_iterator rend() noexcept { return reverse_iterator{begin()}; }
		[[nodiscard]] constexpr const_reverse_iterator rend() const noexcept { return const_reverse_iterator{begin()}; }

		[[nodiscard]] constexpr size_type size() const noexcept { return m_dense.size(); }
		[[nodiscard]] constexpr size_type capacity() const noexcept { return to_capacity(bucket_count()); }
		[[nodiscard]] constexpr size_type max_size() const noexcept { return to_capacity(std::min(m_dense.max_size(), npos - 1)); }
		[[nodiscard]] constexpr float load_factor() const noexcept
		{
			return static_cast<float>(size()) / static_cast<float>(bucket_count());
		}

		[[nodiscard]] constexpr size_type bucket_count() const noexcept { return m_sparse.size(); }
		[[nodiscard]] constexpr size_type max_bucket_count() const noexcept { return m_sparse.max_size(); }

		[[nodiscard]] constexpr iterator find(const auto &key) noexcept { return to_iter(m_dense.data() + find_impl(key_hash(key), key)); }
		[[nodiscard]] constexpr const_iterator find(const auto &key) const noexcept { return to_iter(m_dense.data() + find_impl(key_hash(key), key)); }

		constexpr void clear()
		{
			std::fill_n(m_sparse.data(), bucket_count(), npos);
			m_dense.clear();
		}

		constexpr void rehash(size_type new_cap)
		{
			using std::max;

			/* Adjust the capacity to be at least large enough to fit the current size. */
			new_cap = max(max(static_cast<size_type>(static_cast<float>(size()) / max_load_factor), new_cap), initial_capacity);
			if (new_cap != m_sparse.capacity()) rehash_impl(new_cap);
		}
		constexpr void reserve(size_type n)
		{
			m_dense.reserve(n);
			rehash(static_cast<size_type>(static_cast<float>(n) / max_load_factor));
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
		[[nodiscard]] constexpr bucket_link &header() noexcept { return header_base::value(); }
		[[nodiscard]] constexpr const bucket_link &header() const noexcept { return header_base::value(); }

		constexpr void relink_header() noexcept
		{
			if constexpr (is_ordered::value)
			{
				if (auto &h = header(); !m_dense.empty())
					h.relink(h.next, h.prev);
			}
		}

		[[nodiscard]] constexpr auto hash(const auto &k) const { return get_hash()(k); }
		[[nodiscard]] constexpr auto cmp(const auto &a, const auto &b) const { return get_cmp()(a, b); }

		[[nodiscard]] constexpr size_type to_capacity(size_type n) const noexcept
		{
			return static_cast<size_type>(static_cast<float>(n) * max_load_factor);
		}

		[[nodiscard]] constexpr size_type *get_chain(std::size_t h) noexcept { return m_sparse.data() + (h % bucket_count()); }
		[[nodiscard]] constexpr const size_type *get_chain(std::size_t h) const noexcept { return m_sparse.data() + (h % bucket_count()); }

		[[nodiscard]] constexpr size_type find_impl(std::size_t h, const auto &key) const noexcept
		{
			for (auto *idx = get_chain(h); *idx != npos;)
				if (auto &entry = m_dense[*idx]; entry.hash == h && cmp(key, entry.key()))
					return *idx;
				else
					idx = &entry.next;
			return m_dense.size();
		}

		template<typename... Args>
		[[nodiscard]] constexpr iterator insert_new(std::size_t h, auto *chain_idx, Args &&...args)
		{
			const auto pos = *chain_idx = size();
			m_dense.emplace_back(std::forward<Args>(args)...).hash = h;
			maybe_rehash();
			return to_iter(m_dense.data() + pos);
		}
		template<typename T>
		[[nodiscard]] constexpr std::pair<iterator, bool> emplace_impl(const auto &key, T &&value)
		{
			/* See if we can replace any entry. */
			const auto h = hash(key);
			auto *chain_idx = get_chain(h);
			while (*chain_idx != npos)
				if (auto &candidate = m_dense[*chain_idx]; candidate.hash == h && cmp(key, candidate.key()))
				{
					/* Found a candidate for replacing, replace the value & hash. */
					if constexpr (requires { candidate.value() = std::forward<T>(value); })
						candidate.value() = std::forward<T>(value);
					else
					{
						std::destroy_at(candidate.get());
						std::construct_at(candidate.get(), std::forward<T>(value));
					}
					candidate.hash = h;
					return {to_iter(m_dense.data() + *chain_idx), false};
				}
				else
					chain_idx = &candidate.next;

			/* No candidate for replacement found, create new entry. */
			return {insert_new(h, chain_idx, std::forward<T>(value)), true};
		}
		template<typename... Args>
		[[nodiscard]] constexpr std::pair<iterator, bool> try_emplace_impl(const auto &key, Args &&...args)
		{
			/* See if an entry already exists. */
			const auto h = hash(key);
			auto *chain_idx = get_chain(h);
			while (*chain_idx != npos)
				if (auto &existing = m_dense[*chain_idx]; existing.hash == h && cmp(key, existing.key()))
					return {to_iter(m_dense.data() + *chain_idx), false};
				else
					chain_idx = &existing.next;

			/* No existing entry found, create new entry. */
			return {insert_new(h, chain_idx, std::forward<Args>(args)...), true};
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
			for (size_type i = 0; i < m_dense.size(); ++i)
			{
				auto &entry = m_dense[i];
				auto *chain_idx = get_chain(entry.hash);
				entry.next = *chain_idx;
				*chain_idx = i;
			}
		}

		constexpr iterator erase_impl(std::size_t h, const auto &key)
		{
			for (auto *chain_idx = get_chain(h); *chain_idx != npos;)
			{
				const auto pos = *chain_idx;
				auto *entry_ptr = m_dense.data() + static_cast<difference_type>(pos);
				if (entry_ptr->hash == h && key_comp(key, entry_ptr->key()))
				{
					bucket_node *next = entry_ptr; /* Save the next pointer for returning. */

					/* Unlink the entry from both bucket & insertion chains. */
					if constexpr (is_ordered::value)
					{
						next = entry_ptr->bucket_link::next;
						entry_ptr->unlink();
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

		sparse_t m_sparse = {initial_capacity, npos};
		dense_t m_dense;

	public:
		float max_load_factor = initial_load_factor;
	};
}