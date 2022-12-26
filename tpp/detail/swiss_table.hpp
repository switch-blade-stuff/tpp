/*
 * Created by switchblade on 11/27/22.
 */

#pragma once

#include "table_common.hpp"

#ifndef TPP_USE_IMPORT

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

#ifdef TPP_HAS_NEON

#include <arm_neon.h>

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
	template<typename I, typename V, typename K, typename Kh, typename Kc, typename Alloc, typename ValueTraits>
	class swiss_table;

	struct meta_byte
	{
		static const meta_byte empty;
		static const meta_byte deleted;
		static const meta_byte sentinel;

		constexpr meta_byte() noexcept = default;
		constexpr meta_byte(std::int8_t value) noexcept : value(value) {}

		[[nodiscard]] constexpr operator std::int8_t() const noexcept { return value; }

		[[nodiscard]] constexpr bool is_occupied() const noexcept { return value > sentinel.value; }
		[[nodiscard]] constexpr bool is_available() const noexcept { return value < sentinel.value; }

		[[nodiscard]] constexpr bool operator==(const meta_byte &other) const noexcept { return value == other.value; }
#if (__cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L))
		[[nodiscard]] constexpr auto operator<=>(const meta_byte &other) const noexcept { return value <=> other.value; }
#else
		[[nodiscard]] constexpr bool operator!=(const meta_byte &other) const noexcept { return value != other.value; }
		[[nodiscard]] constexpr bool operator<=(const meta_byte &other) const noexcept { return value <= other.value; }
		[[nodiscard]] constexpr bool operator>=(const meta_byte &other) const noexcept { return value >= other.value; }
		[[nodiscard]] constexpr bool operator<(const meta_byte &other) const noexcept { return value < other.value; }
		[[nodiscard]] constexpr bool operator>(const meta_byte &other) const noexcept { return value > other.value; }
#endif

		std::int8_t value = 0;
	};

	constexpr meta_byte meta_byte::empty = meta_byte{static_cast<std::int8_t>(0b1000'0000)};
	constexpr meta_byte meta_byte::deleted = meta_byte{static_cast<std::int8_t>(0b1111'1110)};
	constexpr meta_byte meta_byte::sentinel = meta_byte{static_cast<std::int8_t>(0b1111'1111)};

	template<typename T, std::size_t P>
	class basic_index_mask
	{
	public:
		/* Use the smallest possible unsigned word type. */
		using value_type = std::conditional_t<sizeof(T) <= sizeof(std::size_t), std::size_t, std::uint64_t>;

	public:
		constexpr basic_index_mask() noexcept = default;
		constexpr explicit basic_index_mask(value_type value) noexcept : m_value(value) {}

		constexpr basic_index_mask operator++(int) noexcept
		{
			auto tmp = *this;
			operator++();
			return tmp;
		}
		constexpr basic_index_mask &operator++() noexcept
		{
			m_value &= (m_value - 1);
			return *this;
		}

		[[nodiscard]] constexpr bool empty() const noexcept { return m_value == 0; }
		[[nodiscard]] std::size_t lsb_index() const noexcept;
		[[nodiscard]] std::size_t msb_index() const noexcept;

		[[nodiscard]] constexpr bool operator==(const basic_index_mask &other) const noexcept { return m_value == other.m_value; }
#if (__cplusplus < 202002L || (defined(_MSVC_LANG) && _MSVC_LANG < 202002L))
		[[nodiscard]] constexpr bool operator!=(const basic_index_mask &other) const noexcept { return m_value != other.m_value; }
#endif

	private:
		value_type m_value = 0;
	};

	template<typename T>
	[[maybe_unused]] [[nodiscard]] constexpr std::size_t generic_ctz(T value) noexcept
	{
		constexpr T mask = T{1};
		std::size_t result = 0;
		while ((value & (mask << result++)) == T{});
		return result;
	}
	template<typename T>
	[[maybe_unused]] [[nodiscard]] constexpr std::size_t generic_clz(T value) noexcept
	{
		constexpr T mask = T{1} << (std::numeric_limits<T>::digits - 1);
		std::size_t result = 0;
		while ((value & (mask >> result++)) == T{});
		return result;
	}

#if defined(__GNUC__) || defined(__clang__)
	template<typename T>
	[[nodiscard]] inline std::size_t ctz(T value) noexcept
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
	[[nodiscard]] inline std::size_t clz(T value) noexcept
	{
		if constexpr (sizeof(T) <= sizeof(unsigned int))
		{
			constexpr auto diff = (sizeof(unsigned int) - sizeof(T)) * 8;
			return static_cast<std::size_t>(__builtin_clz(static_cast<unsigned int>(value))) - diff;
		}
		else if constexpr (sizeof(T) <= sizeof(unsigned long))
		{
			constexpr auto diff = (sizeof(unsigned long) - sizeof(T)) * 8;
			return static_cast<std::size_t>(__builtin_clzl(static_cast<unsigned long>(value))) - diff;
		}
		else if constexpr (sizeof(T) <= sizeof(unsigned long long))
		{
			constexpr auto diff = (sizeof(unsigned long long) - sizeof(T)) * 8;
			return static_cast<std::size_t>(__builtin_clzll(static_cast<unsigned long long>(value))) - diff;
		}
		else
			return generic_clz(value);
	}
#elif defined(_MSC_VER)
	template<typename T>
	[[nodiscard]] inline std::size_t ctz(T value) noexcept
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
	[[nodiscard]] inline std::size_t clz(T value) noexcept
	{
		if constexpr (sizeof(T) <= sizeof(unsigned long))
		{
			unsigned long result = 0;
			_BitScanReverse(&result, static_cast<unsigned long>(value));
			return static_cast<std::size_t>(result) - (sizeof(unsigned long) - sizeof(T));
		}
#ifdef _WIN64
			else if constexpr (sizeof(T) <= sizeof(unsigned __int64))
			{
				unsigned long result = 0;
				_BitScanReverse64(&result, static_cast<unsigned __int64>(value));
				return static_cast<std::size_t>(result) - (sizeof(unsigned __int64) - sizeof(T));
			}
#endif
		else
			return generic_clz(value);
	}
#else
	template<typename T>
	[[nodiscard]] inline std::size_t ctz(T value) noexcept { return generic_ctz(value); }
	template<typename T>
	[[nodiscard]] inline std::size_t clz(T value) noexcept { return generic_clz(value); }
#endif

	template<typename T, std::size_t P>
	std::size_t basic_index_mask<T, P>::lsb_index() const noexcept { return ctz(m_value) >> P; }
	template<typename T, std::size_t P>
	std::size_t basic_index_mask<T, P>::msb_index() const noexcept { return clz(m_value) >> P; }

#if defined(TPP_HAS_SSE2)
	using index_mask = basic_index_mask<std::uint16_t, 0>;
	using block_value = __m128i;
#elif defined(TPP_HAS_NEON)
	using index_mask = basic_index_mask<std::uint64_t, 3>;
	using block_value = uint8x8_t;
#else
	using index_mask = basic_index_mask<std::uint64_t, 3>;
	using block_value = std::uint64_t;
#endif

	struct meta_block
	{
		constexpr meta_block() noexcept = default;
		constexpr meta_block(block_value value) noexcept : value(value) {}

		inline meta_block(const meta_byte *bytes) noexcept;

		[[nodiscard]] inline index_mask match_empty() const noexcept;
		[[nodiscard]] inline index_mask match_available() const noexcept;
		[[nodiscard]] inline index_mask match_eq(meta_byte b) const noexcept;

		/* Count leading empty of deleted entries (index of the left-most occupied entry). */
		[[nodiscard]] inline std::size_t count_available() const noexcept;

		/* Set available to empty & occupied to deleted. */
		[[nodiscard]] inline meta_block reset_occupied() const noexcept;

		block_value value = {};
	};

#if defined(TPP_HAS_SSE2)
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

	meta_block::meta_block(const meta_byte *bytes) noexcept { value = _mm_loadu_si128(reinterpret_cast<const block_value *>(bytes)); }

	index_mask meta_block::match_empty() const noexcept
	{
#ifdef TPP_HAS_SSSE3
		return index_mask{static_cast<std::uint16_t>(_mm_movemask_epi8(_mm_sign_epi8(value, value)))};
#else
		return match_eq(meta_byte::empty);
#endif
	}
	index_mask meta_block::match_available() const noexcept
	{
		return index_mask{static_cast<std::uint16_t>(_mm_movemask_epi8(x86_cmpgt_epi8(_mm_set1_epi8(meta_byte::sentinel), value)))};
	}
	index_mask meta_block::match_eq(meta_byte b) const noexcept
	{
		return index_mask{static_cast<std::uint16_t>(_mm_movemask_epi8(x86_cmpeq_epi8(_mm_set1_epi8(b), value)))};
	}

	std::size_t meta_block::count_available() const noexcept
	{
		return ctz(_mm_movemask_epi8(x86_cmpgt_epi8(_mm_set1_epi8(meta_byte::sentinel), value)) + 1);
	}
	meta_block meta_block::reset_occupied() const noexcept
	{
		/* Mask all occupied. */
		const auto mask = x86_cmpgt_epi8(value, _mm_set1_epi8(meta_byte::sentinel));
		const auto deleted = _mm_set1_epi8(meta_byte::deleted);
		const auto empty = _mm_set1_epi8(meta_byte::empty);

		/* (deleted & mask) | (empty & ~mask) */
		return _mm_or_si128(_mm_and_si128(mask, deleted), _mm_andnot_si128(mask, empty));
	}
#elif defined(TPP_HAS_NEON)
	meta_block::meta_block(const meta_byte *bytes) noexcept { value = vld1_u8(reinterpret_cast<const block_value *>(bytes)); }

	index_mask meta_block::match_empty() const noexcept
	{
		return index_mask{vget_lane_u64(vreinterpret_u64_u8(vceq_s8(vdup_n_s8(meta_byte::empty), vreinterpret_s8_u8(value))), 0)};
	}
	index_mask meta_block::match_available() const noexcept
	{
		return index_mask{vget_lane_u64(vreinterpret_u64_u8(vcgt_s8(vdup_n_s8(meta_byte::sentinel), vreinterpret_s8_u8(value))), 0)};
	}
	index_mask meta_block::match_eq(meta_byte b) const noexcept
	{
		constexpr std::uint64_t msb_mask = 0x8080808080808080;
		const auto v = vdup_n_u8(static_cast<std::uint8_t>(b));
		return index_mask{vget_lane_u64(vreinterpret_u64_u8(vceq_u8(value, v)), 0) & msb_mask};
	}

	std::size_t meta_block::count_available() const noexcept
	{
		return ctz(vget_lane_u64(vreinterpret_u64_u8(vcle_s8(vdup_n_s8(meta_byte::sentinel), vreinterpret_s8_u8(value))), 0)) >> 3;
	}
	meta_block meta_block::reset_occupied() const noexcept
	{
		/* Mask all occupied. */
		const auto mask = vreinterpret_u64_u8(vcgt_s8(vreinterpret_s8_u8(value), vdup_n_s8(meta_byte::sentinel)));
		const auto deleted = vreinterpret_u8_s8(vdup_n_s8(meta_byte::deleted));
		const auto empty = vreinterpret_u8_s8(vdup_n_s8(meta_byte::empty));

		/* mask ? deleted : empty */
		return vbsl_u8(mask, deleted, empty);
	}
#else
	meta_block::meta_block(const meta_byte *bytes) noexcept { value = read_unaligned<block_value>(bytes); }

	index_mask meta_block::match_empty() const noexcept
	{
		constexprstd::uint64_t msb_mask = 0x8080808080808080;
		return index_mask{(value & (~value << 6)) & msb_mask};
	}
	index_mask meta_block::match_available() const noexcept
	{
		constexpr std::uint64_t msb_mask = 0x8080808080808080;
		return index_mask{(value & (~value << 7)) & msb_mask};
	}
	index_mask meta_block::match_eq(meta_byte b) const noexcept
	{
		constexpr std::uint64_t msb_mask = 0x8080808080808080;
		constexpr std::uint64_t lsb_mask = 0x0101010101010101;
		const auto x = value ^ (lsb_mask * static_cast<std::uint8_t>(b));
		return index_mask{(x - lsb_mask) & ~x & msb_mask};
	}

	std::size_t meta_block::count_available() const noexcept
	{
		constexpr std::uint64_t lsb_mask = 0x0101010101010101;
		return ctz((value | ~(value >> 7)) & lsb_mask) >> 3;
	}
	std::size_t meta_block::count_available() const noexcept
	{
		constexpr std::uint64_t lsb_mask = 0x0101010101010101;
		return ctz((value | ~(value >> 7)) & lsb_mask) >> 3;
	}
	meta_block meta_block::reset_occupied() const noexcept
	{
		constexpr std::uint64_t msb_mask = 0x8080808080808080;
		constexpr std::uint64_t lsb_mask = 0x0101010101010101;
		const auto x = value & msb_mask;
		return (~x + (x >> 7)) & ~lsb_mask;
	}
#endif

	template<typename I, typename V, typename K, typename Kh, typename Kc, typename Alloc, typename ValueTraits>
	struct swiss_table_traits : table_traits<I, V, K, Kh, Kc, Alloc>
	{
		using size_type = typename table_traits<I, V, K, Kh, Kc, Alloc>::size_type;

		using is_transparent = std::conjunction<detail::is_transparent<Kh>, detail::is_transparent<Kc>>;
		using is_ordered = detail::is_ordered<typename ValueTraits::link_type>;

		using bucket_link = typename ValueTraits::link_type;
		using bucket_node = table_node<I, Alloc, ValueTraits>;

		using meta_allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<meta_byte>;
		using node_allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<bucket_node>;
	};

	template<typename Node, typename Traits, bool = true>
	class swiss_node_iterator : public ordered_iterator<Node, Traits> {};
	template<typename Node, typename Traits>
	class swiss_node_iterator<Node, Traits, false>
	{
		using meta_ptr = const meta_byte *;
		using node_ptr = Node *;

		// @formatter:off
		template<typename, typename, typename, typename, typename, typename, typename>
		friend struct swiss_table_traits;
		template<typename, typename, bool>
		friend class swiss_node_iterator;
		// @formatter:on

	public:
		using value_type = Node;
		using pointer = Node *;
		using reference = Node &;

		using size_type = typename Traits::type;
		using difference_type = typename Traits::type;
		using iterator_category = std::forward_iterator_tag;

	private:
		swiss_node_iterator(meta_ptr meta, node_ptr node) noexcept : m_meta(meta), m_node(node) { next_occupied(); }

	public:
		constexpr swiss_node_iterator() noexcept = default;
		template<typename U, typename = std::enable_if_t<!std::is_same_v<Node, U> && std::is_constructible_v<node_ptr, U *>>>
		constexpr swiss_node_iterator(const swiss_node_iterator<U, Traits, false> &other) noexcept : m_meta(other.m_meta), m_node(other.m_node) {}

		swiss_node_iterator operator++(int) noexcept
		{
			auto tmp = *this;
			operator++();
			return tmp;
		}
		swiss_node_iterator &operator++() noexcept
		{
			++m_meta;
			++m_node;
			next_occupied();
			return *this;
		}

	private:
		void next_occupied() noexcept
		{
			while (m_meta->is_available())
			{
				const auto off = meta_block{m_meta}.count_available();
				m_meta += off;
				m_node += off;
			}
		}

		meta_ptr m_meta = nullptr;
		node_ptr m_node = nullptr;
	};

	template<typename>
	struct swiss_node_traits {};
	template<typename I, typename A, typename T>
	struct swiss_node_traits<stable_node<I, A, T>>
	{
		using node_type = typename stable_node<I, A, T>::extracted_type;
		template<typename Iter>
		using insert_return_type = typename stable_node<I, A, T>::template insert_return<Iter, node_type>;
	};

	template<typename I, typename V, typename K, typename Kh, typename Kc, typename Alloc, typename ValueTraits>
	class swiss_table : ValueTraits::link_type, ebo_container<Kh>, ebo_container<Kc>, public swiss_node_traits<table_node<I, Alloc, ValueTraits>>
	{
		using traits_t = swiss_table_traits<I, V, K, Kh, Kc, Alloc, ValueTraits>;

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

	private:
		using bucket_link = typename traits_t::bucket_link;
		using bucket_node = typename traits_t::bucket_node;
		using meta_allocator = typename traits_t::meta_allocator;
		using node_allocator = typename traits_t::node_allocator;

#ifdef TPP_DEBUG /* Use dual buffers in debug mode for ease of debugging. */
		class buffer_type : ebo_container<meta_allocator>, ebo_container<node_allocator>
		{
			using meta_alloc_base = ebo_container<meta_allocator>;
			using node_alloc_base = ebo_container<node_allocator>;

		public:
			constexpr buffer_type() noexcept = default;

			[[nodiscard]] meta_byte *metadata() const noexcept { return m_metadata; }
			[[nodiscard]] meta_byte *tail() const noexcept { return m_metadata + capacity + 1; }
			[[nodiscard]] bucket_node *nodes() const noexcept { return m_nodes; }

			template<typename F>
			void resize(size_type n, F &&relocate)
			{
				auto *old_metadata = std::exchange(m_metadata, std::allocator_traits<meta_allocator>::allocate(meta_alloc(), n + sizeof(meta_block)));
				auto *old_nodes = std::exchange(m_nodes, std::allocator_traits<node_allocator>::allocate(node_alloc(), n));
				if (const auto old_capacity = std::exchange(capacity, n); old_capacity != 0)
				{
					relocate(old_metadata, old_nodes, old_capacity);
					std::allocator_traits<meta_allocator>::deallocate(meta_alloc(), old_metadata, old_capacity + sizeof(meta_block));
					std::allocator_traits<node_allocator>::deallocate(node_alloc(), old_nodes, old_capacity);
				}
			}

			void allocate(size_type n)
			{
				m_metadata = std::allocator_traits<meta_allocator>::allocate(meta_alloc(), n + sizeof(meta_block));
				m_nodes = std::allocator_traits<node_allocator>::allocate(node_alloc(), n);
				capacity = n;
			}
			void deallocate()
			{
				std::allocator_traits<meta_allocator>::deallocate(meta_alloc(), m_metadata, capacity + sizeof(meta_block));
				std::allocator_traits<node_allocator>::deallocate(node_alloc(), m_nodes, capacity);
				m_metadata = nullptr;
				m_nodes = nullptr;
				capacity = 0;
			}

			[[nodiscard]] auto &get_allocator() const noexcept { return node_alloc_base::value(); }

			void swap(buffer_type &other) noexcept(std::is_nothrow_swappable_v<meta_allocator> && std::is_nothrow_swappable_v<node_allocator>)
			{
				using std::swap;
				if constexpr (std::allocator_traits<meta_allocator>::propagate_on_container_swap::value)
					swap(meta_alloc(), other.meta_alloc());
				if constexpr (std::allocator_traits<node_allocator>::propagate_on_container_swap::value)
					swap(node_alloc(), other.node_alloc());

				TPP_ASSERT(allocator_eq(meta_alloc(), other.meta_alloc()) && allocator_eq(node_alloc(), other.node_alloc()),
				           "Swapped allocators must be equal");

				swap(capacity, other.capacity);
				swap(m_metadata, other.m_metadata);
				swap(m_nodes, other.m_nodes);
			}

			size_type capacity = 0;

		private:
			[[nodiscard]] constexpr auto &meta_alloc() noexcept { return meta_alloc_base::value(); }
			[[nodiscard]] constexpr auto &node_alloc() noexcept { return node_alloc_base::value(); }

			meta_byte *m_metadata = nullptr;
			bucket_node *m_nodes = nullptr;
		};
#else
		class buffer_type : ebo_container<node_allocator>
		{
			using alloc_base = ebo_container<node_allocator>;

			[[nodiscard]] static constexpr size_type nodes_offset(size_type n) noexcept
			{
				const auto meta_bytes = n + sizeof(meta_block);
				const auto rem = meta_bytes % sizeof(bucket_node);
				return meta_bytes + (rem ? sizeof(bucket_node) - rem : 0);
			}
			[[nodiscard]] static constexpr size_type buffer_size(size_type n) noexcept
			{
				/* Total size = aligned metadata + nodes */
				return n + align_bytes(nodes_offset(n)) / sizeof(bucket_node);
			}

		public:
			constexpr buffer_type() noexcept = default;

			[[nodiscard]] meta_byte *metadata() const noexcept { return reinterpret_cast<meta_byte *>(bytes()); }
			[[nodiscard]] meta_byte *tail() const noexcept { return metadata() + capacity + 1; }
			[[nodiscard]] bucket_node *nodes() const noexcept { return reinterpret_cast<bucket_node *>(bytes() + nodes_offset(capacity)); }

			template<typename F>
			void resize(size_type n, F &&relocate)
			{
				auto *old_data = std::exchange(m_data, static_cast<void *>(std::allocator_traits<node_allocator>::allocate(node_alloc(), buffer_size(n))));
				if (old_data != nullptr)
				{
					auto *old_nodes = reinterpret_cast<bucket_node *>(static_cast<std::uint8_t *>(old_data) + nodes_offset(n));
					auto *old_metadata = static_cast<meta_byte *>(old_data);
					const auto old_capacity = std::exchange(capacity, n);

					relocate(old_metadata, old_nodes, old_capacity);
					std::allocator_traits<node_allocator>::deallocate(node_alloc(), old_data, buffer_size(n));
				}
			}

			void allocate(size_type n)
			{
				m_data = static_cast<void *>(std::allocator_traits<node_allocator>::allocate(node_alloc(), buffer_size(n)));
				capacity = n;
			}
			void deallocate()
			{
				std::allocator_traits<node_allocator>::deallocate(node_alloc(), m_data, buffer_size(capacity));
				m_data = nullptr;
				capacity = 0;
			}

			[[nodiscard]] auto &get_allocator() const noexcept { return alloc_base::value(); }

			void swap(buffer_type &other) noexcept(std::is_nothrow_swappable_v<node_allocator>)
			{
				using std::swap;
				if constexpr (std::allocator_traits<node_allocator>::propagate_on_container_swap::value)
					swap(node_alloc(), other.node_alloc());

				TPP_ASSERT(allocator_eq(node_alloc(), other.node_alloc()), "Swapped allocators must be equal");

				swap(capacity, other.capacity);
				swap(m_data, other.m_data);
			}

			size_type capacity = 0;

		private:
			[[nodiscard]] constexpr auto &node_alloc() noexcept { return alloc_base::value(); }
			[[nodiscard]] constexpr auto *bytes() const noexcept { return static_cast<std::uint8_t *>(m_data); }

			/* Combined node & metadata buffers. */
			void *m_data = nullptr;
		};
#endif

		using node_iterator = swiss_node_iterator<bucket_node, traits_t, is_ordered::value>;
		using const_node_iterator = swiss_node_iterator<const bucket_node, traits_t, is_ordered::value>;
		using hint_t = const_node_iterator;

		struct bucket_probe
		{
			constexpr bucket_probe(size_type pos, size_type cap) noexcept : pos(pos), cap(cap) {}

			constexpr bucket_probe &operator++() noexcept
			{
				/* Always advance by block size. */
				idx += sizeof(meta_block);
				pos = (pos + idx) & cap;
				return *this;
			}

			/* Capacity-aware offset from the probe position. */
			[[nodiscard]] constexpr size_type off(size_type n) const noexcept { return (pos + n) & cap; }

			size_type idx = 0; /* Iteration/index of the probe. */
			size_type pos = 0; /* Position of the target block. */
			size_type cap = 0; /* Capacity of the table. */
		};

	public:
		using iterator = table_iterator<value_type, ValueTraits, node_iterator>;
		using const_iterator = table_iterator<const value_type, ValueTraits, const_node_iterator>;

		using reference = typename iterator::reference;
		using const_reference = typename const_iterator::reference;
		using pointer = typename iterator::pointer;
		using const_pointer = typename const_iterator::pointer;

	private:
		using header_base = bucket_link;
		using hash_base = ebo_container<hasher>;
		using cmp_base = ebo_container<key_equal>;

		[[nodiscard]] static constexpr std::pair<std::size_t, meta_byte> decompose_hash(std::size_t h) noexcept
		{
			return {h >> 7, {static_cast<std::int8_t>(h & 0x7f)}};
		}

		/* Convert node capacity to max table size. */
		[[nodiscard]] static constexpr size_type capacity_to_max_size(size_type n) noexcept
		{
			if constexpr (sizeof(meta_block) == 8)
				return n == 7 ? 6 : n - n / 8;
			else
				return n - n / 8;
		}
		/* Convert table size to required node capacity. */
		[[nodiscard]] static constexpr size_type size_to_min_capacity(size_type n) noexcept
		{
			if constexpr (sizeof(meta_block) == 8)
				return n == 7 ? 8 : n + static_cast<size_type>((static_cast<std::int64_t>(n) - 1) / 7);
			else
				return n + static_cast<size_type>((static_cast<std::int64_t>(n) - 1) / 7);
		}
		/* Aligns capacity to next power of 2 - 1. */
		[[nodiscard]] static constexpr size_type align_capacity(size_type n) noexcept
		{
			return n ? std::numeric_limits<size_type>::max() >> clz(n) : 1;
		}

	public:
		~swiss_table()
		{
			if (m_size != 0)
			{
				clear_nodes();
				m_buffer.deallocate();
			}
		}

		[[nodiscard]] constexpr iterator begin() noexcept { return to_iter(begin_node()); }
		[[nodiscard]] constexpr const_iterator begin() const noexcept { return to_iter(begin_node()); }
		[[nodiscard]] constexpr iterator end() noexcept { return to_iter(end_node()); }
		[[nodiscard]] constexpr const_iterator end() const noexcept { return to_iter(end_node()); }

		[[nodiscard]] constexpr reference front() noexcept { return *to_iter(front_node()); }
		[[nodiscard]] constexpr const_reference front() const noexcept { return *to_iter(front_node()); }
		[[nodiscard]] constexpr reference back() noexcept { return *to_iter(back_node()); }
		[[nodiscard]] constexpr const_reference back() const noexcept { return *to_iter(back_node()); }

		[[nodiscard]] constexpr size_type size() const noexcept { return m_size; }
		[[nodiscard]] constexpr size_type max_size() const noexcept { return capacity_to_max_size(max_bucket_count()); }
		[[nodiscard]] constexpr size_type capacity() const noexcept { return capacity_to_max_size(bucket_count()); }
		[[nodiscard]] constexpr float load_factor() const noexcept { return static_cast<float>(size()) / static_cast<float>(bucket_count()); }

		[[nodiscard]] constexpr size_type bucket_count() const noexcept { return m_buffer.capacity; }
		[[nodiscard]] constexpr size_type max_bucket_count() const noexcept { return std::numeric_limits<size_type>::max() - sizeof(meta_block); }

		void clear()
		{
			if (m_size != 0)
			{
				clear_nodes();
				clear_metadata();
				m_size = 0;
				m_num_empty = capacity_to_max_size(m_buffer.capacity);
			}
		}
		void reserve(size_type n) { if (n > m_size + m_num_empty) rehash_impl(align_capacity(size_to_min_capacity(n))); }

		template<typename T>
		[[nodiscard]] bool contains(const T &key) const { return find_node(key, hash(key)) != m_buffer.capacity; }

		template<typename T>
		[[nodiscard]] iterator find(const T &key) { return to_iter(find_node(key, hash(key))); }
		template<typename T>
		[[nodiscard]] const_iterator find(const T &key) const { return to_iter(find_node(key, hash(key))); }

		std::pair<iterator, bool> insert(const insert_type &value) { return insert_impl({}, ValueTraits::get_key(value), value); }
		std::pair<iterator, bool> insert(insert_type &&value) { return insert_impl({}, ValueTraits::get_key(value), std::move(value)); }
		iterator insert(const_iterator hint, const insert_type &value)
		{
			return insert_impl(to_underlying(hint), ValueTraits::get_key(value), value).first;
		}
		iterator insert(const_iterator hint, insert_type &&value)
		{
			return insert_impl(to_underlying(hint), ValueTraits::get_key(value), std::move(value)).first;
		}

		template<typename T, typename = std::enable_if_t<!(std::is_convertible_v<T &&, insert_type &&> || std::is_convertible_v<T &&, value_type &&>)>>
		std::pair<iterator, bool> insert(T &&value) TPP_REQUIRES((std::is_constructible_v<V, T>))
		{
			return emplace_impl({}, std::forward<T>(value));
		}
		template<typename T, typename = std::enable_if_t<!(std::is_convertible_v<T &&, insert_type &&> || std::is_convertible_v<T &&, value_type &&>)>>
		iterator insert(const_iterator hint, T &&value) TPP_REQUIRES((std::is_constructible_v<V, T>))
		{
			return emplace_impl(to_underlying(hint), std::forward<T>(value)).first;
		}

		template<typename N, typename S = stable_node<I, Alloc, ValueTraits>>
		std::enable_if_t<std::is_same_v<N, typename S::extracted_type>, typename S::template insert_return<iterator, N>> insert(N &&node)
		{
			if (auto target_pos = find_node(node.key(), node.hash()); target_pos == m_buffer.capacity)
				return {emplace_node({}, node.hash(), std::forward<N>(node)), true};
			else
				return {to_iter(target_pos), false, std::forward<N>(node)};
		}
		template<typename N, typename S = stable_node<I, Alloc, ValueTraits>>
		std::enable_if_t<std::is_same_v<N, typename S::extracted_type>, iterator> insert(const_iterator hint, N &&node)
		{
			if (auto target_pos = find_node(node.key(), node.hash()); target_pos == m_buffer.capacity)
				return emplace_node(hint, node.hash(), std::forward<N>(node));
			else
				return to_iter(target_pos);
		}

		template<typename Iter>
		void insert(Iter first, Iter last)
		{
			if constexpr (std::is_base_of_v<std::random_access_iterator_tag, typename std::iterator_traits<Iter>::iterator_category>)
				reserve(static_cast<size_type>(std::distance(first, last)));
			for (; first != last; ++first) insert(*first);
		}

		template<typename T, typename U>
		std::pair<iterator, bool> insert_or_assign(const T &key, U &&value) TPP_REQUIRES((std::is_constructible_v<V, T, U>))
		{
			return insert_or_assign_impl({}, key, std::forward<U>(value));
		}
		template<typename T, typename U>
		iterator insert_or_assign(const_iterator hint, const T &key, U &&value) TPP_REQUIRES((std::is_constructible_v<V, T, U>))
		{
			return insert_or_assign_impl(to_underlying(hint), key, std::forward<U>(value)).first;
		}

		template<typename N, typename S = stable_node<I, Alloc, ValueTraits>>
		std::enable_if_t<std::is_same_v<N, typename S::extracted_type>, typename S::template insert_return<iterator, N>> insert_or_assign(N &&node)
		{
			if (auto target_pos = find_node(node.key(), node.hash()); target_pos == m_buffer.capacity)
				return {emplace_node({}, node.hash(), std::forward<N>(node)), true};
			else
			{
				auto alloc = allocator_type{get_allocator()};
				m_buffer.nodes()[target_pos].construct(alloc, std::forward<N>(node));
				return {to_iter(target_pos), false};
			}
		}
		template<typename N, typename S = stable_node<I, Alloc, ValueTraits>>
		std::enable_if_t<std::is_same_v<N, typename S::extracted_type>, iterator> insert_or_assign(hint_t hint, N &&node)
		{
			if (auto target_pos = find_node(node.key(), node.hash()); target_pos == m_buffer.capacity)
				return emplace_node(hint, node.hash(), std::forward<N>(node));
			else
			{
				auto alloc = allocator_type{get_allocator()};
				m_buffer.nodes()[target_pos].construct(alloc, std::forward<N>(node));
				return to_iter(target_pos);
			}
		}

		template<typename... Args>
		std::pair<iterator, bool> emplace(Args &&...args) TPP_REQUIRES((std::is_constructible_v<V, Args...>))
		{
			return emplace_impl({}, std::forward<Args>(args)...);
		}
		template<typename... Args>
		iterator emplace_hint(const_iterator hint, Args &&...args) TPP_REQUIRES((std::is_constructible_v<V, Args...>))
		{
			return emplace_impl(to_underlying(hint), std::forward<Args>(args)...);
		}

		template<typename U, typename... Args>
		std::pair<iterator, bool> emplace_or_replace(U &&key, Args &&...args) TPP_REQUIRES(
				(std::is_constructible_v<V, std::piecewise_construct_t, std::tuple<U &&>, std::tuple<Args && ...>>))
		{
			return insert_or_assign_impl({}, std::forward<U>(key), std::forward<Args>(args)...);
		}
		template<typename U, typename... Args>
		iterator emplace_or_replace(const_iterator hint, U &&key, Args &&...args) TPP_REQUIRES(
				(std::is_constructible_v<V, std::piecewise_construct_t, std::tuple<U &&>, std::tuple<Args && ...>>))
		{
			return insert_or_assign_impl(hint, std::forward<U>(key), std::forward<Args>(args)...);
		}

		template<typename... Ks, typename... Args>
		std::pair<iterator, bool> try_emplace(std::tuple<Ks...> keys, Args &&...args) TPP_REQUIRES(
				(std::is_constructible_v<V, std::piecewise_construct_t, std::tuple<Ks ...>, std::tuple<Args && ...>>))
		{
			return try_emplace_impl({}, std::move(keys), std::forward<Args>(args)...);
		}
		template<typename... Ks, typename... Args>
		iterator try_emplace(const_iterator hint, std::tuple<Ks...> keys, Args &&...args) TPP_REQUIRES(
				(std::is_constructible_v<V, std::piecewise_construct_t, std::tuple<Ks ...>, std::tuple<Args && ...>>))
		{
			return try_emplace_impl(to_underlying(hint), std::move(keys), std::forward<Args>(args)...).first;
		}

		template<typename T, typename = std::enable_if_t<!std::is_convertible_v<T, const_iterator>>>
		iterator erase(const T &key)
		{
			if (auto pos = find(key); pos != end())
				return erase_impl(pos);
			else
				return pos;
		}
		iterator erase(const_iterator where)
		{
			if (where != end())
			{
				const auto pos = &(*to_underlying(where)) - m_buffer.nodes();
				return erase_impl(pos);
			}
			else
				return end();
		}
		iterator erase(const_iterator first, const_iterator last)
		{
			iterator result = end();
			while (last != first) result = erase(--last);
			return result;
		}

		template<typename T, typename = std::enable_if_t<!std::is_convertible_v<T, const_iterator>>>
		typename stable_node<I, Alloc, ValueTraits>::extracted_type extract(const T &key)
		{
			if (auto pos = find(key); pos != end())
			{
				auto alloc = allocator_type{get_allocator()};
				return {alloc, std::move(extract_impl(pos))};
			}
			else
				return {};
		}
		typename stable_node<I, Alloc, ValueTraits>::extracted_type extract(const_iterator where)
		{
			if (where != end())
			{
				auto alloc = allocator_type{get_allocator()};
				const auto pos = &(*to_underlying(where)) - m_buffer.nodes();
				return {alloc, std::move(extract_impl(pos))};
			}
			else
				return {};
		}

		template<typename Kh2, typename Kc2>
		void merge(swiss_table<I, V, K, Kh2, Kc2, Alloc, ValueTraits> &other)
		{
			reserve(m_size + other.size());

			/* Iterate backwards for ordered links, since the ordered iterators are invalidated on erase. */
			std::conditional_t<is_ordered::value, std::reverse_iterator<iterator>, iterator> pos, last;
			if constexpr (is_ordered::value)
			{
				pos = std::reverse_iterator{other.end()};
				last = std::reverse_iterator{other.begin()};
			}
			else
			{
				pos = other.begin();
				last = other.end();
			}

			/* Extract nodes from other and insert into this. */
			for (; pos != last; ++pos) insert(other.extract(pos));
		}
		template<typename Kh2, typename Kc2>
		void merge(swiss_table<I, V, K, Kh2, Kc2, Alloc, ValueTraits> &&other)
		{
			if (m_size == 0)
				operator=(std::move(other));
			else
				merge(other);
		}

		void rehash(size_type n)
		{
			/* Skip rehash if table is empty and requested size is 0. */
			TPP_IF_UNLIKELY(!n && !m_size) return;

			const auto new_cap = align_capacity(n | size_to_min_capacity(n));
			if (!n || new_cap > m_buffer.capacity) rehash_impl(new_cap);
		}

		/* SwissHash uses a fixed maximum load factor. See https://github.com/abseil/abseil-cpp/blob/189d55a57f57731d335fd84999d5dccf771b8e6b/absl/container/internal/raw_hash_set.h#L479 */
		[[nodiscard]] constexpr float max_load_factor() const noexcept { return 7.0f / 8.0f; }

		[[nodiscard]] constexpr auto &get_allocator() const noexcept { return m_buffer.get_allocator(); }
		[[nodiscard]] constexpr auto &get_hash() const noexcept { return hash_base::value(); }
		[[nodiscard]] constexpr auto &get_cmp() const noexcept { return cmp_base::value(); }

		void swap(swiss_table &other)
		noexcept(std::is_nothrow_swappable_v<buffer_type> &&
		         std::is_nothrow_swappable_v<key_equal> &&
		         std::is_nothrow_swappable_v<hasher>)
		{
			header_base::swap(other);
			hash_base::swap(other);
			cmp_base::swap(other);
			m_buffer.swap(other.m_buffer);
		}

	private:
		void assert_probe(bucket_probe probe) const noexcept
		{
			TPP_ASSERT(probe.idx < m_buffer.capacity, "Probe must not exceed table capacity");
		}

		template<typename T>
		[[nodiscard]] constexpr std::size_t hash(const T &k) const { return get_hash()(k); }
		template<typename T, typename U>
		[[nodiscard]] constexpr bool cmp(const T &a, const U &b) const { return get_cmp()(a, b); }

		[[nodiscard]] constexpr auto *begin_node() const noexcept
		{
			if constexpr (is_ordered::value)
				return static_cast<bucket_node *>(header_link()->off(header_base::next));
			else
				return m_buffer.nodes();
		}
		[[nodiscard]] constexpr auto *end_node() const noexcept
		{
			if constexpr (is_ordered::value)
				return static_cast<bucket_node *>(header_link());
			else
				return m_buffer.nodes() + m_buffer.capacity;
		}

		[[nodiscard]] constexpr auto *header_link() const noexcept
		{
			/* Using `const_cast` here to avoid non-const function duplicates. Pointers will be converted to appropriate const-ness either way. */
			return const_cast<bucket_link *>(static_cast<const bucket_link *>(this));
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

		[[nodiscard]] constexpr auto to_iter(size_type pos) noexcept
		{
			if constexpr (is_ordered::value)
				return iterator{node_iterator{pos < m_buffer.capacity ? m_buffer.nodes() + pos : end_node()}};
			else
				return iterator{node_iterator{m_buffer.metadata() + pos, m_buffer.nodes() + pos}};
		}
		[[nodiscard]] constexpr auto to_iter(size_type pos) const noexcept
		{
			if constexpr (is_ordered::value)
				return const_iterator{const_node_iterator{pos < m_buffer.capacity ? m_buffer.nodes() + pos : end_node()}};
			else
				return const_iterator{const_node_iterator{m_buffer.metadata() + pos, m_buffer.nodes() + pos}};
		}
		[[nodiscard]] constexpr auto to_iter(bucket_node *node) noexcept
		{
			if constexpr (!is_ordered::value)
				return iterator{node_iterator{m_buffer.metadata() + (node - begin_node()), node}};
			else
				return iterator{node_iterator{node}};
		}
		[[nodiscard]] constexpr auto to_iter(const bucket_node *node) const noexcept
		{
			if constexpr (!is_ordered::value)
				return const_iterator{const_node_iterator{m_buffer.metadata() + (node - begin_node()), node}};
			else
				return const_iterator{const_node_iterator{node}};
		}

		template<typename T>
		size_type find_node(const T &key, std::size_t h) const
		{
			TPP_IF_LIKELY(m_size != 0)
			{
				const auto [h1, h2] = decompose_hash(h);
				for (auto probe = bucket_probe{h1 & m_buffer.capacity, m_buffer.capacity};; ++probe)
				{
					/* Go through each matched element in the block and test for equality. */
					const auto block = meta_block{m_buffer.metadata() + probe.pos};
					for (auto match = block.match_eq(h2); !match.empty(); ++match)
					{
						const auto offset = probe.off(match.lsb_index());
						const auto &node = m_buffer.nodes()[offset];
						TPP_IF_LIKELY(node.hash() == h && cmp(node.key(), key)) return offset;
					}
					/* Fail if the block is empty and there are no matching elements. */
					TPP_IF_UNLIKELY(!block.match_empty().empty()) break;
					assert_probe(probe);
				}
			}
			return m_buffer.capacity;
		}
		size_type find_available(std::size_t h) const noexcept
		{
			const auto [h1, h2] = decompose_hash(h);
			for (auto probe = bucket_probe{h1 & m_buffer.capacity, m_buffer.capacity};; ++probe)
			{
				const auto block = meta_block{m_buffer.metadata() + probe.pos};
				if (const auto match = block.match_available(); !match.empty())
				{
					const auto offset = probe.off(match.lsb_index());
					return {offset, probe.idx};
				}
				assert_probe(probe);
			}
		}

		void clear_metadata() noexcept
		{
			std::fill_n(m_buffer.metadata(), m_buffer.capacity + sizeof(meta_block), meta_byte::empty);
			m_buffer.metadata()[m_buffer.capacity] = meta_byte::sentinel;
		}
		void clear_nodes()
		{
			auto alloc = allocator_type{get_allocator()};
			auto *metadata = m_buffer.metadata();
			auto *nodes = m_buffer.nodes();
			for (size_type i = 0; i != m_buffer.capacity; ++i) if (metadata[i].is_occupied()) nodes[i].destroy(alloc);
		}

		void set_metadata(size_type pos, meta_byte value) noexcept
		{
			constexpr auto tail_size = sizeof(meta_block) - 1;
			auto *metadata = m_buffer.metadata();
			metadata[((pos - tail_size) & m_buffer.capacity) + (tail_size & m_buffer.capacity)] = value;
			metadata[pos] = value;
		}
		void init_buffers(size_type capacity)
		{
			TPP_ASSERT(((capacity + 1) & capacity) == 0, "Capacity must be a power of 2 - 1");

			m_buffer.allocate(capacity);
			m_num_empty = capacity_to_max_size(capacity);
			clear_metadata();
		}

		void insert_link(hint_t hint, bucket_node *node) noexcept
		{
			if constexpr (is_ordered::value) node->link(hint.link ? const_cast<bucket_link *>(hint.link) : back_node());
		}
		template<typename... Args>
		iterator emplace_node(hint_t hint, std::size_t h, Args &&...args)
		{
			size_type target_pos = 0;
			TPP_IF_UNLIKELY(m_buffer.capacity == 0)
			{
				rehash_impl(1);
				target_pos = find_available(h);
			}
			else
			{
				target_pos = find_available(h);
				TPP_IF_UNLIKELY(!m_num_empty && m_buffer.metadata()[target_pos] != meta_byte::deleted)
				{
					/* Do an in-place rehash by reclaiming deleted entries. Choice of coefficients is outlined by the reference implementation at
					 * https://github.com/abseil/abseil-cpp/blob/f7affaf32a6a396465507dd10520a3fe183d4e40/absl/container/internal/raw_hash_set.cc#L97
					 *
					 * NOTE: Reclaiming deleted entries may leave blocks partially empty. */
					if (m_buffer.capacity > sizeof(meta_block) && static_cast<std::uint64_t>(m_size) * 32 <= static_cast<std::uint64_t>(m_buffer.capacity) * 25)
						rehash_deleted();
					else
						rehash_impl(m_buffer.capacity * 2 - 1);
					target_pos = find_available(h);
				}
			}

			auto *target = m_buffer.nodes() + target_pos;
			auto alloc = allocator_type{get_allocator()};
			target->construct(alloc, std::forward<Args>(args)...);
			insert_link(hint, target);
			target->hash() = h;

			m_num_empty -= m_buffer.metadata()[target_pos] == meta_byte::empty;
			set_metadata(target_pos, decompose_hash(h).second);
			++m_size;

			return {to_iter(target_pos), true};
		}
		iterator erase_node(size_type pos, bucket_node *node)
		{
			TPP_ASSERT(pos < m_buffer.capacity, "Erased position is out of range");
			TPP_ASSERT(m_buffer.metadata()[pos].is_occupied(), "Erased node must be occupied");

			auto *next = node;
			if constexpr (is_ordered::value)
			{
				auto *link = static_cast<bucket_link *>(node);
				next = static_cast<bucket_node *>(link->off(link->next));
				link->unlink();
			}
			set_metadata(pos, meta_byte::deleted);
			--m_size;
			return to_iter(next);
		}

		template<typename... Args>
		std::pair<iterator, bool> emplace_impl(hint_t hint, Args &&...args)
		{
			auto alloc = allocator_type{get_allocator()};
			auto tmp = bucket_node{};

			tmp.construct(alloc, std::forward<Args>(args)...);
			auto result = insert_impl(hint, std::move(tmp));
			tmp.destroy(alloc);

			return result;
		}
		template<typename T, typename... Args>
		std::pair<iterator, bool> try_emplace_impl(hint_t hint, T &&key, Args &&...args)
		{
			const auto h = hash(key);
			if (auto target_pos = find_node(key, h); target_pos == m_buffer.capacity)
				return {emplace_node(hint, h, std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(std::forward<Args>(args))...), true};
			else
				return {to_iter(target_pos), false};
		}
		template<typename T, typename... Args>
		std::pair<iterator, bool> insert_impl(hint_t hint, const T &key, Args &&...args)
		{
			const auto h = hash(key);
			if (auto target_pos = find_node(h, key); target_pos == m_buffer.capacity)
				return {emplace_node(hint, h, std::forward<Args>(args)...), true};
			else
				return {to_iter(target_pos), false};
		}
		template<typename T, typename... Args>
		std::pair<iterator, bool> insert_or_assign_impl(hint_t hint, T &&key, Args &&...args)
		{
			const auto h = hash(key);
			if (auto target_pos = find_node(h, key); target_pos == m_buffer.capacity)
				return {emplace_node(hint, h, std::piecewise_construct, std::forward_as_tuple(key), std::forward_as_tuple(std::forward<Args>(args))...), true};
			else
			{
				m_buffer.nodes()[target_pos].replace(std::forward<Args>(args)...);
				return {to_iter(target_pos), false};
			}
		}

		bucket_node &extract_impl(size_type pos)
		{
			auto &node = m_buffer.nodes()[pos];
			erase_node(pos, &node);
			return node;
		}
		iterator erase_impl(size_type pos)
		{
			auto alloc = allocator_type{get_allocator()};
			auto *node = m_buffer.nodes() + pos;
			auto result = erase_node(pos, node);
			node->destroy(alloc);
			return result;
		}

		void rehash_impl(size_type capacity)
		{
			m_buffer.resize(capacity, [&](meta_byte *src_meta, bucket_node *src_nodes, size_type src_capacity)
			{
				/* Relocate occupied entries from the old buffers to the new buffers. */
				auto alloc = node_allocator{get_allocator()};
				for (size_type i = 0; i < src_capacity; ++i)
					if (src_meta[i].is_occupied())
					{
						auto *node = src_nodes + i;
						const auto h = node->hash();
						const auto target_pos = find_available(h);
						relocate_node{}(alloc, node, alloc, m_buffer.nodes() + target_pos);
						set_metadata(target_pos, decompose_hash(h).second);
					}
			});
			m_num_empty = capacity_to_max_size(capacity) - m_size;
		}
		void rehash_deleted()
		{
			/* Set all occupied as deleted. */
			for (auto i = 0; i < m_buffer.capacity; i += sizeof(meta_block))
			{
				const auto old_block = m_buffer.metadata() + i;
				const auto new_block = meta_block{old_block}.reset_occupied();
				std::memcpy(old_block, &new_block, sizeof(meta_block));
			}
			std::memcpy(m_buffer.tail(), m_buffer.metadata(), sizeof(meta_block) - 1);
			m_buffer.metadata()[m_buffer.capacity] = meta_byte::sentinel;

			/* Relocation algorithm as described by the reference implementation at https://github.com/abseil/abseil-cpp/blob/f7affaf32a6a396465507dd10520a3fe183d4e40/absl/container/internal/raw_hash_set.cc#L97 */
			for (auto i = 0; i < m_buffer.capacity; ++i)
				if (m_buffer.metadata()[i] == meta_byte::deleted)
				{
					auto *node = m_buffer.nodes() + i;

				process_node:
					const auto target_pos = find_available(node->hash());
					const auto [h1, h2] = decompose_hash(node->hash());

					/* If relocation position is within the same block, resurrect the original entry. */
					const auto block_idx = [cap = m_buffer.capacity, h1 = h1](size_t pos) { return ((pos - (h1 & cap)) & cap) / sizeof(meta_block); };
					TPP_IF_LIKELY(block_idx(target_pos) == block_idx(i))
					{
						set_metadata(i, h2);
						continue;
					}

					/* If the target is in a different block and is empty, relocate the node. Otherwise, swap with the other element. */
					if (m_buffer.metadata()[target_pos] == meta_byte::empty)
					{
						auto alloc = allocator_type{get_allocator()};
						relocate_node{}(alloc, node, alloc, m_buffer.nodes() + target_pos);
						set_metadata(i, meta_byte::empty);
						set_metadata(target_pos, h2);
					}
					else
					{
						swap(*node, m_buffer.nodes()[target_pos]);
						set_metadata(target_pos, h2);
						goto process_node; /* Process the swapped-with node. */
					}
				}

			/* Reset the amount of empty slots left. */
			m_num_empty = capacity_to_max_size(m_buffer.capacity) - m_size;
		}

		size_type m_size = 0;       /* Amount of occupied nodes. */
		size_type m_num_empty = 0;  /* Amount of empty entries we can still use. */

		buffer_type m_buffer = 0;
	};
}