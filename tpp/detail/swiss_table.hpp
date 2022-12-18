/*
 * Created by switchblade on 11/27/22.
 */

#pragma once

#include "utility.hpp"

#ifndef TPP_USE_IMPORT

#include <cstring>
#include <vector>
#include <limits>
#include <tuple>

#endif

#ifndef TPP_USE_IMPORT

#if defined(TPP_HAS_SSSE3)

#include <tmmintrin.h>

#elif defined(TPP_HAS_SSE2)

#include <emmintrin.h>

#endif

#ifdef _MSC_VER

#include <intrin.h>

#pragma intrinsic(_BitScanForward, _BitScanReverse)
#ifdef _WIN64
#pragma intrinsic(_BitScanForward64, _BitScanReverse64)
#endif

#endif

#endif

namespace tpp::detail
{
	template<typename V, typename K, typename Kh, typename Kc, typename Alloc, typename ValueTraits>
	class swiss_table;

	enum class meta_byte : std::uint8_t
	{
		SENTINEL = 0b1111'1111,
		HASH_MASK = 0b0111'1111,
		CONTROL_MASK = 0b1000'0000,

		EMPTY = 0b1000'0000,
		DELETED = 0b1111'1110,
		OCCUPIED = HASH_MASK,
	};

	template<typename T, std::size_t P>
	class basic_index_mask
	{
	public:
		/* Use the smallest possible unsigned word type. */
		using value_type = std::conditional_t<sizeof(T) <= sizeof(std::size_t), std::size_t, std::uint64_t>;

	public:
		constexpr basic_index_mask() noexcept = default;
		constexpr explicit basic_index_mask(value_type value) noexcept : m_value(value) {}

		[[nodiscard]] constexpr basic_index_mask next() const noexcept { return basic_index_mask{m_value & (m_value - 1)}; }
		[[nodiscard]] constexpr basic_index_mask next_msb() const noexcept { return basic_index_mask{m_value & (m_value - 1)}; }

		[[nodiscard]] constexpr bool empty() const noexcept { return m_value == 0; }
		[[nodiscard]] TPP_CXX20_CONSTEXPR std::size_t lsb_index() const noexcept;
		[[nodiscard]] TPP_CXX20_CONSTEXPR std::size_t msb_index() const noexcept;

		[[nodiscard]] constexpr bool operator==(const basic_index_mask &other) const noexcept { return m_value == other.m_value; }
#if (__cplusplus < 202002L || _MSVC_LANG < 202002L)
		[[nodiscard]] constexpr bool operator!=(const basic_index_mask &other) const noexcept { return m_value != other.m_value; }
#endif

	private:
		value_type m_value = 0;
	};

#ifdef TPP_HAS_CONSTEVAL
	template<typename T>
	[[nodiscard]] constexpr std::size_t generic_ctz(T value) noexcept
	{
		constexpr T mask = T{1};
		std::size_t result = 0;
		while ((value & (mask << result++)) == T{});
		return result;
	}
	template<typename T>
	[[nodiscard]] constexpr std::size_t generic_clz(T value) noexcept
	{
		constexpr T mask = T{1} << (std::numeric_limits<T>::digits - 1);
		std::size_t result = 0;
		while ((value & (mask >> result++)) == T{});
		return result;
	}
#endif

#if defined(__GNUC__) || defined(__clang__)
	template<typename T>
	[[nodiscard]] inline std::size_t builtin_ctz(T value) noexcept
	{
		if constexpr (sizeof(T) <= sizeof(unsigned int))
			return static_cast<std::size_t>(__builtin_ctz(static_cast<unsigned int>(value)));
		else if constexpr (sizeof(T) <= sizeof(unsigned long))
			return static_cast<std::size_t>(__builtin_ctzl(static_cast<unsigned long>(value)));
		else if constexpr (sizeof(T) <= sizeof(unsigned long long))
			return static_cast<std::size_t>(__builtin_ctzll(static_cast<unsigned long long>(value)));
		else
			return generic_ctz(value);
	}
	template<typename T>
	[[nodiscard]] inline std::size_t builtin_clz(T value) noexcept
	{
		if constexpr (sizeof(T) <= sizeof(unsigned int))
			return static_cast<std::size_t>(__builtin_clz(static_cast<unsigned int>(value)));
		else if constexpr (sizeof(T) <= sizeof(unsigned long))
			return static_cast<std::size_t>(__builtin_clzl(static_cast<unsigned long>(value)));
		else if constexpr (sizeof(T) <= sizeof(unsigned long long))
			return static_cast<std::size_t>(__builtin_clzll(static_cast<unsigned long long>(value)));
		else
			return generic_clz(value);
	}
#elif defined(_MSC_VER)
	template<typename T>
	[[nodiscard]] inline std::size_t builtin_ctz(T value) noexcept
	{
		if constexpr (sizeof(T) <= sizeof(unsigned long))
		{
			unsigned long result = 0;
			_BitScanForward(&result, static_cast<unsigned long>(value));
			return static_cast<std::size_t>(result);
		}
#ifdef _WIN64
			else if constexpr (sizeof(T) <= sizeof(unsigned __int64))
			{
				unsigned long result = 0;
				_BitScanForward64(&result, static_cast<unsigned __int64>(value));
				return static_cast<std::size_t>(result);
			}
#endif
		else
			return generic_ctz(value);
	}
	template<typename T>
	[[nodiscard]] inline std::size_t builtin_clz(T value) noexcept
	{
		if constexpr (sizeof(T) <= sizeof(unsigned long))
		{
			unsigned long result = 0;
			_BitScanReverse(&result, static_cast<unsigned long>(value));
			return static_cast<std::size_t>(result);
		}
#ifdef _WIN64
			else if constexpr (sizeof(T) <= sizeof(unsigned __int64))
			{
				unsigned long result = 0;
				_BitScanReverse64(&result, static_cast<unsigned __int64>(value));
				return static_cast<std::size_t>(result);
			}
#endif
		else
			return generic_clz(value);
	}
#else
	template<typename T>
	[[nodiscard]] inline std::size_t builtin_ctz(T value) noexcept { return generic_ctz(value); }
	template<typename T>
	[[nodiscard]] inline std::size_t builtin_clz(T value) noexcept { return generic_clz(value); }
#endif

	template<typename T>
	[[nodiscard]] TPP_CXX20_CONSTEXPR std::size_t ctz(T value) noexcept
	{
#ifdef TPP_HAS_CONSTEVAL
		if (TPP_IS_CONSTEVAL) return generic_ctz(value);
		else
#endif
			return builtin_ctz(value);
	}
	template<typename T>
	[[nodiscard]] TPP_CXX20_CONSTEXPR std::size_t clz(T value) noexcept
	{
#ifdef TPP_HAS_CONSTEVAL
		if (TPP_IS_CONSTEVAL) return generic_clz(value);
		else
#endif
			return builtin_clz(value);
	}

	template<typename T>
	[[nodiscard]] constexpr T align_mult(T value, T mult) noexcept
	{
		const auto rem = value % mult;
		return rem ? value + (mult - rem) : value;
	}

	template<typename T, std::size_t P>
	TPP_CXX20_CONSTEXPR std::size_t basic_index_mask<T, P>::lsb_index() const noexcept { return ctz(m_value) >> P; }
	template<typename T, std::size_t P>
	TPP_CXX20_CONSTEXPR std::size_t basic_index_mask<T, P>::msb_index() const noexcept { return clz(m_value) >> P; }

#ifdef TPP_HAS_SSE2
	using index_mask = basic_index_mask<std::uint16_t, 0>;
	using block_value = __m128i;
#else
	using index_mask = basic_index_mask<std::uint64_t, 3>;
	using block_value = std::uint64_t;
#endif

	struct alignas(block_value) meta_block : std::array<meta_byte, sizeof(block_value)>
	{
		using array_base = std::array<meta_byte, sizeof(block_value)>;

		constexpr meta_block() noexcept = default;
		constexpr meta_block(const meta_byte *bytes) noexcept { value() = *void_cast<const block_value>(bytes); }

		[[nodiscard]] TPP_CXX20_CONSTEXPR index_mask match_empty() const noexcept;
		[[nodiscard]] TPP_CXX20_CONSTEXPR index_mask match_sentinel() const noexcept;
		[[nodiscard]] TPP_CXX20_CONSTEXPR index_mask match_occupied() const noexcept;
		[[nodiscard]] TPP_CXX20_CONSTEXPR index_mask match_available() const noexcept;
		[[nodiscard]] TPP_CXX20_CONSTEXPR index_mask match_eq(meta_byte b) const noexcept;

		[[nodiscard]] constexpr void *raw_data() noexcept { return array_base::data(); }
		[[nodiscard]] constexpr const void *raw_data() const noexcept { return array_base::data(); }

		[[nodiscard]] constexpr block_value &value() noexcept { return *static_cast<block_value *>(raw_data()); }
		[[nodiscard]] constexpr const block_value &value() const noexcept { return *static_cast<const block_value *>(raw_data()); }
	};

#ifdef TPP_HAS_SSE2
	/* https://gcc.gnu.org/bugzilla/show_bug.cgi?id=87853 */
	[[nodiscard]] inline __m128i x86_cmpeq_epi8(__m128i a, __m128i b) noexcept
	{
#if defined(__GNUC__) && !defined(__clang__)
		if constexpr (std::is_unsigned_v<char>)
			return (__m128i) ((__v16qi) a == (__v16qi) b);
		else
#endif
		return _mm_cmpeq_epi8(a, b);
	}
	[[nodiscard]] inline __m128i x86_cmpgt_epi8(__m128i a, __m128i b) noexcept
	{
#if defined(__GNUC__) && !defined(__clang__)
		if constexpr (std::is_unsigned_v<char>)
			return (__m128i) ((__v16qi) a > (__v16qi) b);
		else
#endif
		return _mm_cmpgt_epi8(a, b);
	}

	TPP_CXX20_CONSTEXPR index_mask meta_block::match_empty() const noexcept
	{
#ifdef TPP_HAS_SSSE3
#ifdef TPP_HAS_CONSTEVAL
		if (!TPP_IS_CONSTEVAL)
#endif
			return index_mask{static_cast<index_mask::value_type>(_mm_movemask_epi8(_mm_sign_epi8(value(), value())))};
#ifdef TPP_HAS_CONSTEVAL
		else
#endif
#endif

#if !defined(TPP_HAS_SSSE3) || defined(TPP_HAS_CONSTEVAL)
		return match_eq(meta_byte::EMPTY);
#endif
	}
	TPP_CXX20_CONSTEXPR index_mask meta_block::match_sentinel() const noexcept
	{
		return match_eq(meta_byte::SENTINEL);
	}
	TPP_CXX20_CONSTEXPR index_mask meta_block::match_occupied() const noexcept
	{
		index_mask::value_type result = 0;
#ifdef TPP_HAS_CONSTEVAL
		if (TPP_IS_CONSTEVAL)
			for (std::size_t i = 0; i < sizeof(block_value); ++i)
			{
				const auto a = static_cast<std::int8_t>(meta_byte::SENTINEL);
				const auto b = static_cast<std::int8_t>((*this)[i]);
				result |= static_cast<index_mask::value_type>(a < b) << i;
			}
		else
#endif
		{
			const auto mask = _mm_set1_epi8(static_cast<char>(meta_byte::SENTINEL));
			result = static_cast<index_mask::value_type>(_mm_movemask_epi8(x86_cmpgt_epi8(value(), mask)));
		}
		return index_mask{result};
	}
	TPP_CXX20_CONSTEXPR index_mask meta_block::match_available() const noexcept
	{
		index_mask::value_type result = 0;
#ifdef TPP_HAS_CONSTEVAL
		if (TPP_IS_CONSTEVAL)
			for (std::size_t i = 0; i < sizeof(block_value); ++i)
			{
				const auto a = static_cast<std::int8_t>(meta_byte::SENTINEL);
				const auto b = static_cast<std::int8_t>((*this)[i]);
				result |= static_cast<index_mask::value_type>(a > b) << i;
			}
		else
#endif
		{
			const auto mask = _mm_set1_epi8(static_cast<char>(meta_byte::SENTINEL));
			result = static_cast<index_mask::value_type>(_mm_movemask_epi8(x86_cmpgt_epi8(mask, value())));
		}
		return index_mask{result};
	}
	TPP_CXX20_CONSTEXPR index_mask meta_block::match_eq(meta_byte b) const noexcept
	{
		index_mask::value_type result = 0;
#ifdef TPP_HAS_CONSTEVAL
		if (TPP_IS_CONSTEVAL)
			for (std::size_t i = 0; i < sizeof(block_value); ++i)
				result |= static_cast<index_mask::value_type>((*this)[i] == b) << i;
		else
#endif
		{
			const auto mask_vec = _mm_set1_epi8(static_cast<char>(b));
			result = static_cast<index_mask::value_type>(_mm_movemask_epi8(x86_cmpeq_epi8(mask_vec, value())));
		}
		return index_mask{result};
	}
#else
	constexpr index_mask meta_block::match_empty() const noexcept
	{
		constexpr index_mask::value_type msb_mask = 0x8080808080808080
		return index_mask{(value() & (~value() << 6)) & msb_mask};
	}
	constexpr index_mask meta_block::match_sentinel() const noexcept { return match_eq(meta_byte::SENTINEL); }
	constexpr index_mask meta_block::match_occupied() const noexcept
	{
		constexpr index_mask::value_type msb_mask = 0x8080808080808080;
		return index_mask{~(value() & (~value() << 7)) & msb_mask};
	}
	constexpr index_mask meta_block::match_available() const noexcept
	{
		constexpr index_mask::value_type msb_mask = 0x8080808080808080;
		return index_mask{(value() & (~value() << 7)) & msb_mask};
	}
	constexpr index_mask meta_block::match_eq(meta_byte b) const noexcept
	{
		constexpr index_mask::value_type msb_mask = 0x8080808080808080;
		constexpr index_mask::value_type lsb_mask = 0x0101010101010101;
		const auto x = value() ^ (lsb_mask * static_cast<std::uint8_t>(b));
		return index_mask{(x - lsb_mask) & ~x & msb_mask};
	}
#endif

	template<typename V, typename K, typename Kh, typename Kc, typename Alloc, typename ValueTraits>
	struct swiss_table_traits : table_traits<V, V, K, Kh, Kc, Alloc>
	{
		using size_type = typename table_traits<V, V, K, Kh, Kc, Alloc>::size_type;

		using is_transparent = std::conjunction<detail::is_transparent<Kh>, detail::is_transparent<Kc>>;
		using is_ordered = detail::is_ordered<typename ValueTraits::link_type>;

		using bucket_link = typename ValueTraits::link_type;
		using bucket_node = table_node<V, Alloc, ValueTraits>;

		using meta_allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<meta_block>;
		using node_allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<bucket_node>;
	};

	template<typename Node, typename Traits, bool = true>
	class swiss_node_iterator : public ordered_iterator<Node, Traits> {};

	template<typename Node, typename Traits>
	class swiss_node_iterator<Node, Traits, false>
	{
		using meta_ptr = const meta_byte *;
		using node_ptr = Node *;

	public:
		using value_type = Node;
		using pointer = Node *;
		using reference = Node &;

		using size_type = typename Traits::type;
		using difference_type = typename Traits::type;
		using iterator_category = std::forward_iterator_tag;

	public:
		constexpr swiss_node_iterator() noexcept = default;
		template<typename U, typename = std::enable_if_t<!std::is_same_v<Node, U> && std::is_constructible_v<node_ptr, U *>>>
		constexpr swiss_node_iterator(const swiss_node_iterator<U, Traits, false> &other) noexcept : m_meta(other.m_meta), m_node(other.m_node) {}

		constexpr swiss_node_iterator(meta_ptr meta, node_ptr node) noexcept : m_meta(meta), m_node(node) { next_occupied(); }

		constexpr swiss_node_iterator operator++(int) noexcept
		{
			auto tmp = *this;
			operator++();
			return tmp;
		}
		constexpr swiss_node_iterator &operator++() noexcept
		{
			++m_meta;
			++m_node;
			next_occupied();
			return *this;
		}

	private:
		constexpr void next_occupied() noexcept
		{
			while (static_cast<std::int8_t>(*m_meta) < static_cast<std::int8_t>(meta_byte::SENTINEL))
			{
				const auto off_mask = meta_block{m_meta}.match_occupied();
				const auto off = off_mask.empty() ? sizeof(meta_block) : off_mask.msb_index();
				m_meta += off;
				m_node += off;
			}
		}

		meta_ptr m_meta = nullptr;
		node_ptr m_node = nullptr;
	};

	template<typename V, typename K, typename Kh, typename Kc, typename Alloc, typename ValueTraits>
	class swiss_table
			: ValueTraits::link_type,
			  ebo_container<typename swiss_table_traits<V, V, Kh, Kc, Alloc, ValueTraits>::meta_allocator>,
			  ebo_container<typename swiss_table_traits<V, V, Kh, Kc, Alloc, ValueTraits>::node_allocator>,
			  ebo_container<Kh>,
			  ebo_container<Kc>
	{
		using traits_t = swiss_table_traits<V, V, Kh, Kc, Alloc, ValueTraits>;

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

		constexpr static float initial_load_factor = .875f;
		constexpr static size_type initial_capacity = sizeof(meta_block);

	private:
		using bucket_link = typename traits_t::bucket_link;
		using bucket_node = typename traits_t::bucket_node;
		using meta_allocator = typename traits_t::meta_allocator;
		using node_allocator = typename traits_t::node_allocator;

		using node_iterator = swiss_node_iterator<bucket_node, traits_t, is_ordered::value>;
		using const_node_iterator = swiss_node_iterator<const bucket_node, traits_t, is_ordered::value>;

		struct probe
		{
			constexpr probe(size_type k, size_type cap) noexcept : pos(k), cap(cap) {}

			constexpr probe &operator++() noexcept
			{
				/* Always advance by block size. */
				idx += sizeof(meta_block);
				pos = (pos + idx) & cap;
				return *this;
			}

			size_type idx = 0;
			size_type pos = 0;
			size_type cap = 0;
		};

		template<typename N>
		class bucket_iterator
		{
			// @formatter:off
			template<typename>
			friend class bucket_iterator;
			// @formatter:on

		public:
			using value_type = V;
			using reference = std::conditional_t<std::is_const_v<N>, typename ValueTraits::const_reference, typename ValueTraits::reference>;
			using pointer = std::conditional_t<std::is_const_v<N>, typename ValueTraits::const_pointer, typename ValueTraits::pointer>;

			using size_type = typename bucket_node::size_type;
			using difference_type = typename bucket_node::difference_type;
			using iterator_category = std::forward_iterator_tag;

		public:
			constexpr bucket_iterator() noexcept = default;
			template<typename U, typename = std::enable_if_t<!std::is_same_v<N, U> && std::is_constructible_v<N *, U *>>>
			constexpr bucket_iterator(const bucket_iterator<U> &other) noexcept
					: m_meta_base(other.m_meta_base), m_node_base(other.m_node_base),
					  m_idx(other.m_idx), m_pos(other.m_pos), m_cap(other.m_cap) {}
			constexpr bucket_iterator(const meta_byte *meta, N *node, size_type pos, size_type cap) noexcept
					: m_meta_base(meta), m_node_base(node), m_pos(pos), m_cap(cap) { next_occupied(); }

			constexpr bucket_iterator operator++(int) noexcept
			{
				auto tmp = *this;
				operator++();
				return tmp;
			}
			constexpr bucket_iterator &operator++() noexcept
			{
				inc_probe();
				next_occupied();
				return *this;
			}

			[[nodiscard]] constexpr pointer operator->() const noexcept { return pointer{&node()->value()}; }
			[[nodiscard]] constexpr reference operator*() const noexcept { return *operator->(); }

			[[nodiscard]] constexpr bool operator==(const bucket_iterator &other) const noexcept
			{
				return m_node_base == other.m_node_base && m_pos == other.m_pos;
			}
#if (__cplusplus < 202002L || _MSVC_LANG < 202002L)
			[[nodiscard]] constexpr bool operator!=(const bucket_iterator &other) const noexcept
			{
				return m_node_base != other.m_node_base || m_pos != other.m_pos;
			}
#endif

		private:
			[[nodiscard]] constexpr auto *meta() const noexcept { return m_meta_base + m_pos; }
			[[nodiscard]] constexpr auto *node() const noexcept { return m_node_base + m_pos; }

			constexpr void next_occupied() noexcept
			{
				while (*meta() == meta_byte::DELETED)
					inc_probe();
				if (*meta() == meta_byte::EMPTY)
				{
					m_node_base = nullptr;
					m_pos = 0;
				}
			}
			constexpr void inc_probe() noexcept
			{
				if ((++m_idx % sizeof(meta_block)) != 0)
					m_pos = (m_pos + m_idx) & m_cap;
			}

			const meta_byte *m_meta_base = nullptr;
			N *m_node_base = nullptr;

			size_type m_idx = 0;
			size_type m_pos = 0;
			size_type m_cap = 0;
		};

	public:
		using iterator = table_iterator<value_type, ValueTraits, node_iterator>;
		using const_iterator = table_iterator<const value_type, ValueTraits, const_node_iterator>;
		using local_iterator = bucket_iterator<bucket_node>;
		using const_local_iterator = bucket_iterator<const bucket_node>;

		using reference = typename iterator::reference;
		using const_reference = typename const_iterator::reference;
		using pointer = typename iterator::pointer;
		using const_pointer = typename const_iterator::pointer;

	private:
		using header_base = bucket_link;
		using hash_base = ebo_container<hasher>;
		using cmp_base = ebo_container<key_equal>;
		using meta_alloc_base = ebo_container<meta_allocator>;
		using node_alloc_base = ebo_container<node_allocator>;

		[[nodiscard]] constexpr static std::pair<std::size_t, std::uint8_t> decompose_hash(std::size_t h) noexcept { return {h >> 7, h & 0x7f}; }

	public:
		[[nodiscard]] constexpr iterator begin() noexcept { return to_iter(begin_node()); }
		[[nodiscard]] constexpr const_iterator begin() const noexcept { return to_iter(begin_node()); }
		[[nodiscard]] constexpr iterator end() noexcept { return to_iter(end_node()); }
		[[nodiscard]] constexpr const_iterator end() const noexcept { return to_iter(end_node()); }

		[[nodiscard]] constexpr reference front() noexcept { return *to_iter(front_node()); }
		[[nodiscard]] constexpr const_reference front() const noexcept { return *to_iter(front_node()); }
		[[nodiscard]] constexpr reference back() noexcept { return *to_iter(back_node()); }
		[[nodiscard]] constexpr const_reference back() const noexcept { return *to_iter(back_node()); }

		[[nodiscard]] constexpr size_type size() const noexcept { return m_size; }
		[[nodiscard]] constexpr size_type max_size() const noexcept { return to_capacity(max_bucket_count()); }
		[[nodiscard]] constexpr size_type capacity() const noexcept { return to_capacity(bucket_count()); }
		[[nodiscard]] constexpr float load_factor() const noexcept { return static_cast<float>(size()) / static_cast<float>(bucket_count()); }

		[[nodiscard]] constexpr local_iterator begin(size_type n) noexcept
		{
			return m_capacity ? local_iterator{meta_bytes(), m_buckets, n, m_capacity} : end(n);
		}
		[[nodiscard]] constexpr const_local_iterator begin(size_type n) const noexcept
		{
			return m_capacity ? const_local_iterator{meta_bytes(), m_buckets, n, m_capacity} : end(n);
		}
		[[nodiscard]] constexpr local_iterator end(size_type) noexcept { return local_iterator{}; }
		[[nodiscard]] constexpr const_local_iterator end(size_type) const noexcept { return const_local_iterator{}; }

		[[nodiscard]] constexpr size_type bucket_count() const noexcept { return m_capacity; }
		[[nodiscard]] constexpr size_type max_bucket_count() const noexcept { return std::numeric_limits<size_type>::max() - sizeof(meta_block); }
		[[nodiscard]] constexpr size_type bucket_size(size_type n) const noexcept { return static_cast<size_type>(std::distance(begin(n), end(n))); }
		template<typename T>
		[[nodiscard]] TPP_CXX20_CONSTEXPR size_type bucket(const T &key) const { return hash(key) % bucket_count(); }

		TPP_CXX20_CONSTEXPR void clear();
		TPP_CXX20_CONSTEXPR void reserve(size_type n)
		{
			reserve_data(n);
			rehash(static_cast<size_type>(static_cast<float>(n) / m_max_load_factor));
		}

		template<typename T>
		[[nodiscard]] TPP_CXX20_CONSTEXPR bool contains(const T &key) const { return find_node(key, hash(key)).second != end_node(); }

		template<typename T>
		[[nodiscard]] TPP_CXX20_CONSTEXPR iterator find(const T &key) { return to_iter(find_node(key, hash(key))); }
		template<typename T>
		[[nodiscard]] TPP_CXX20_CONSTEXPR const_iterator find(const T &key) const { return to_iter(find_node(key, hash(key))); }

		[[nodiscard]] constexpr float max_load_factor() const noexcept { return m_max_load_factor; }
		constexpr void max_load_factor(float f) noexcept
		{
			/* Load factor cannot exceed 1 for probing to work. */
			TPP_ASSERT(f < 1.0f, "Load factor must be less than 1.0");
			m_max_load_factor = f;
		}

		[[nodiscard]] constexpr auto &get_allocator() const noexcept { return get_node_alloc(); }
		[[nodiscard]] constexpr auto &get_hash() const noexcept { return hash_base::value(); }
		[[nodiscard]] constexpr auto &get_cmp() const noexcept { return cmp_base::value(); }

		constexpr void swap(swiss_table &other)
		noexcept(std::is_nothrow_swappable_v<hasher> &&
		         std::is_nothrow_swappable_v<key_equal> &&
		         std::is_nothrow_swappable_v<meta_alloc_base> &&
		         std::is_nothrow_swappable_v<node_alloc_base>)
		{
			using std::swap;

			header_base::swap(other);
			hash_base::swap(other);
			cmp_base::swap(other);

			if constexpr (std::allocator_traits<meta_allocator>::propagate_on_container_swap::value)
				swap(get_meta_alloc(), other.get_meta_alloc());
			if constexpr (std::allocator_traits<node_allocator>::propagate_on_container_swap::value)
				swap(get_node_alloc(), other.get_node_alloc());

			TPP_ASSERT(allocator_eq(get_meta_alloc(), other.get_sparse_alloc()), "Swapped allocators must be equal");
			TPP_ASSERT(allocator_eq(get_node_alloc(), other.get_dense_alloc()), "Swapped allocators must be equal");

			swap_data(other);
			swap(m_max_load_factor, other.m_max_load_factor);
		}

	private:
		[[nodiscard]] constexpr auto to_capacity(size_type n) const noexcept { return static_cast<size_type>(static_cast<float>(n) * m_max_load_factor); }

		[[nodiscard]] constexpr auto &get_meta_alloc() noexcept { return meta_alloc_base::value(); }
		[[nodiscard]] constexpr auto &get_meta_alloc() const noexcept { return meta_alloc_base::value(); }
		[[nodiscard]] constexpr auto &get_node_alloc() noexcept { return node_alloc_base::value(); }
		[[nodiscard]] constexpr auto &get_node_alloc() const noexcept { return node_alloc_base::value(); }

		template<typename T>
		[[nodiscard]] constexpr std::size_t hash(const T &k) const { return get_hash()(k); }
		template<typename T, typename U>
		[[nodiscard]] constexpr bool cmp(const T &a, const U &b) const { return get_cmp()(a, b); }

		[[nodiscard]] constexpr auto *header_link() const noexcept
		{
			/* Using `const_cast` here to avoid non-const function duplicates. Pointers will be converted to appropriate const-ness either way. */
			return const_cast<bucket_link *>(static_cast<const bucket_link *>(this));
		}
		[[nodiscard]] constexpr auto *begin_node() const noexcept
		{
			if constexpr (is_ordered::value)
				return static_cast<bucket_node *>(header_link()->off(header_base::next));
			else
				return m_buckets;
		}
		[[nodiscard]] constexpr auto *end_node() const noexcept
		{
			if constexpr (is_ordered::value)
				return static_cast<bucket_node *>(header_link());
			else
				return m_buckets + m_capacity;
		}

		[[nodiscard]] constexpr auto *front_node() const noexcept
		{
			static_assert(is_ordered::value, "front & back is only available for ordered tables");
			return begin_node();
		}
		[[nodiscard]] constexpr auto *back_node() const noexcept
		{
			static_assert(is_ordered::value, "front & back is only available for ordered tables");
			return static_cast<bucket_node *>(header_link()->off(header_base::prev));
		}

		[[nodiscard]] constexpr meta_byte *meta_bytes() const noexcept { return m_metadata->data(); }

		[[nodiscard]] constexpr auto to_iter(bucket_node *node) noexcept
		{
			if constexpr (!is_ordered::value)
				return node ? iterator{node_iterator{meta_bytes() + (node - m_buckets), node}} : iterator{};
			else
				return iterator{node_iterator{node}};
		}
		[[nodiscard]] constexpr auto to_iter(const bucket_node *node) const noexcept
		{
			if constexpr (!is_ordered::value)
				return node ? const_iterator{const_node_iterator{meta_bytes() + (node - m_buckets), node}} : const_iterator{};
			else
				return const_iterator{const_node_iterator{node}};
		}
		[[nodiscard]] constexpr auto to_iter(std::pair<const meta_block *, bucket_node *> node) noexcept
		{
			if constexpr (!is_ordered::value)
				return node.second ? iterator{node_iterator{node.first, node.second}} : iterator{};
			else
				return iterator{node_iterator{node.second}};
		}
		[[nodiscard]] constexpr auto to_iter(std::pair<const meta_block *, const bucket_node *> node) const noexcept
		{
			if constexpr (!is_ordered::value)
				return node.second ? const_iterator{const_node_iterator{node.first, node.second}} : const_iterator{};
			else
				return const_iterator{const_node_iterator{node.second}};
		}

		TPP_CXX20_CONSTEXPR std::pair<meta_block *, bucket_node *> find_node(std::size_t h, const key_type &key) const
		{
			TPP_IF_LIKELY(size() != 0)
			{
				const auto [h1, h2] = decompose_hash(h);
				for (auto p = probe{h1 & m_capacity, m_capacity};; ++p)
				{
					/* Make sure probe never iterates more than we have buckets. */
					TPP_ASSERT(p.idx < m_capacity, "Probe must not exceed bucket count");

					/* Go through each matched element in the block and test for equality. */
					const auto &block = m_metadata[p.pos];
					for (auto match = block.match_eq(static_cast<meta_byte>(h2)); !match.empty(); match = match.next())
					{
						const auto offset = p.pos + match.lsb_index();
						const auto &node = m_buckets[offset];
						TPP_IF_LIKELY(node.hash() == h && cmp(node.key(), key)) return {m_metadata + offset, m_buckets + offset};
					}
					/* Fail if the block is empty. */
					TPP_IF_UNLIKELY(block.match_empty()) break;
				}
			}
			const auto end = end_node();
			return {m_metadata + (end - m_buckets), end};
		}

		float m_max_load_factor = initial_load_factor;

		size_type m_size = 0;       /* Total amount of elements in the table. */
		size_type m_capacity = 0;   /* Total capacity in number of buckets (actual size of the table's buffers). */

		meta_block *m_metadata = nullptr;
		bucket_node *m_buckets = nullptr;
	};
}