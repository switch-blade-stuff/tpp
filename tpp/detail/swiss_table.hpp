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

	struct meta_word
	{
		static const meta_word empty;
		static const meta_word deleted;
		static const meta_word sentinel;

		[[nodiscard]] constexpr operator std::int8_t() const noexcept { return value; }

		[[nodiscard]] constexpr bool is_occupied() const noexcept;
		[[nodiscard]] constexpr bool is_available() const noexcept;

		[[nodiscard]] constexpr bool operator==(const meta_word &other) const noexcept { return value == other.value; }
#if (__cplusplus >= 202002L || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L))
		[[nodiscard]] constexpr auto operator<=>(const meta_word &other) const noexcept { return value <=> other.value; }
#else
		[[nodiscard]] constexpr bool operator!=(const meta_word &other) const noexcept { return value != other.value; }
		[[nodiscard]] constexpr bool operator<=(const meta_word &other) const noexcept { return value <= other.value; }
		[[nodiscard]] constexpr bool operator>=(const meta_word &other) const noexcept { return value >= other.value; }
		[[nodiscard]] constexpr bool operator<(const meta_word &other) const noexcept { return value < other.value; }
		[[nodiscard]] constexpr bool operator>(const meta_word &other) const noexcept { return value > other.value; }
#endif

		std::int8_t value = 0;
	};

	constexpr meta_word meta_word::empty = meta_word{static_cast<std::int8_t>(0b1000'0000)};
	constexpr meta_word meta_word::deleted = meta_word{static_cast<std::int8_t>(0b1111'1110)};
	constexpr meta_word meta_word::sentinel = meta_word{static_cast<std::int8_t>(0b1111'1111)};

	constexpr bool meta_word::is_occupied() const noexcept { return value > sentinel.value; }
	constexpr bool meta_word::is_available() const noexcept { return value < sentinel.value; }

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
#if (__cplusplus < 202002L && (!defined(_MSVC_LANG) || _MSVC_LANG < 202002L))
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

		inline meta_block(const meta_word *bytes) noexcept;

		[[nodiscard]] inline index_mask match_empty() const noexcept;
		[[nodiscard]] inline index_mask match_available() const noexcept;
		[[nodiscard]] inline index_mask match_eq(meta_word b) const noexcept;

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

	meta_block::meta_block(const meta_word *bytes) noexcept { value = _mm_loadu_si128(reinterpret_cast<const block_value *>(bytes)); }

	index_mask meta_block::match_empty() const noexcept
	{
#ifdef TPP_HAS_SSSE3
		return index_mask{static_cast<std::uint16_t>(_mm_movemask_epi8(_mm_sign_epi8(value, value)))};
#else
		return match_eq(meta_word::empty);
#endif
	}
	index_mask meta_block::match_available() const noexcept
	{
		return index_mask{static_cast<std::uint16_t>(_mm_movemask_epi8(x86_cmpgt_epi8(_mm_set1_epi8(meta_word::sentinel), value)))};
	}
	index_mask meta_block::match_eq(meta_word b) const noexcept
	{
		return index_mask{static_cast<std::uint16_t>(_mm_movemask_epi8(x86_cmpeq_epi8(_mm_set1_epi8(b), value)))};
	}

	std::size_t meta_block::count_available() const noexcept
	{
		return ctz(_mm_movemask_epi8(x86_cmpgt_epi8(_mm_set1_epi8(meta_word::sentinel), value)) + 1);
	}
	meta_block meta_block::reset_occupied() const noexcept
	{
		/* Mask all occupied. */
		const auto mask = x86_cmpgt_epi8(value, _mm_set1_epi8(meta_word::sentinel));
		const auto deleted = _mm_set1_epi8(meta_word::deleted);
		const auto empty = _mm_set1_epi8(meta_word::empty);

		/* (deleted & mask) | (empty & ~mask) */
		return _mm_or_si128(_mm_and_si128(mask, deleted), _mm_andnot_si128(mask, empty));
	}
#elif defined(TPP_HAS_NEON)
	meta_block::meta_block(const meta_word *bytes) noexcept { value = vld1_u8(reinterpret_cast<const block_value *>(bytes)); }

	index_mask meta_block::match_empty() const noexcept
	{
		return index_mask{vget_lane_u64(vreinterpret_u64_u8(vceq_s8(vdup_n_s8(meta_word::empty), vreinterpret_s8_u8(value))), 0)};
	}
	index_mask meta_block::match_available() const noexcept
	{
		return index_mask{vget_lane_u64(vreinterpret_u64_u8(vcgt_s8(vdup_n_s8(meta_word::sentinel), vreinterpret_s8_u8(value))), 0)};
	}
	index_mask meta_block::match_eq(meta_word b) const noexcept
	{
		constexpr std::uint64_t msb_mask = 0x8080808080808080;
		const auto v = vdup_n_u8(static_cast<std::uint8_t>(b));
		return index_mask{vget_lane_u64(vreinterpret_u64_u8(vceq_u8(value, v)), 0) & msb_mask};
	}

	std::size_t meta_block::count_available() const noexcept
	{
		return ctz(vget_lane_u64(vreinterpret_u64_u8(vcle_s8(vdup_n_s8(meta_word::sentinel), vreinterpret_s8_u8(value))), 0)) >> 3;
	}
	meta_block meta_block::reset_occupied() const noexcept
	{
		/* Mask all occupied. */
		const auto mask = vreinterpret_u64_u8(vcgt_s8(vreinterpret_s8_u8(value), vdup_n_s8(meta_word::sentinel)));
		const auto deleted = vreinterpret_u8_s8(vdup_n_s8(meta_word::deleted));
		const auto empty = vreinterpret_u8_s8(vdup_n_s8(meta_word::empty));

		/* mask ? deleted : empty */
		return vbsl_u8(mask, deleted, empty);
	}
#else
	meta_block::meta_block(const meta_word *bytes) noexcept { value = read_unaligned<block_value>(bytes); }

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
	index_mask meta_block::match_eq(meta_word b) const noexcept
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

	/* Helper used to select node table type. */
	template<typename T, typename = void>
	struct is_stable : std::false_type {};
	template<typename T>
	struct is_stable<T, std::void_t<typename T::is_stable>> : T::is_stable {};

	template<typename I, typename V, typename K, typename Kh, typename Kc, typename Alloc, typename ValueTraits>
	struct swiss_table_traits : table_traits<I, V, K, Kh, Kc, Alloc>
	{
		using size_type = typename table_traits<I, V, K, Kh, Kc, Alloc>::size_type;

		using is_transparent = std::conjunction<detail::is_transparent<Kh>, detail::is_transparent<Kc>>;
		using is_ordered = detail::is_ordered<typename ValueTraits::link_type>;

		using bucket_node = std::conditional_t<is_stable<ValueTraits>::value, stable_node<V, Alloc, ValueTraits>, packed_node<I, Alloc, ValueTraits>>;
		using bucket_link = typename ValueTraits::link_type;

		using meta_allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<meta_word>;
		using node_allocator = typename std::allocator_traits<Alloc>::template rebind_alloc<bucket_node>;
	};

	template<typename V, typename A, typename T, bool = is_stable<T>::value>
	struct swiss_node_traits {};
	template<typename V, typename A, typename T>
	struct swiss_node_traits<V, A, T, true>
	{
		using node_type = typename stable_node<V, A, T>::extracted_type;
		template<typename Iter>
		using insert_return_type = typename stable_node<V, A, T>::template insert_return<Iter, node_type>;
	};

	template<typename Node, typename Traits, bool = true>
	class swiss_node_iterator : public ordered_iterator<Node, Traits>
	{
		using ordered_iterator<Node, Traits>::ordered_iterator;
		using ordered_iterator<Node, Traits>::operator=;
	};
	template<typename Node, typename Traits>
	class swiss_node_iterator<Node, Traits, false>
	{
		using meta_ptr = const meta_word *;
		using node_ptr = Node *;

		// @formatter:off
		template<typename, typename, typename, typename, typename, typename, typename>
		friend class swiss_table;
		template<typename, typename, bool>
		friend class swiss_node_iterator;
		// @formatter:on

	public:
		using value_type = Node;
		using pointer = Node *;
		using reference = Node &;

		using size_type = typename Traits::size_type;
		using difference_type = typename Traits::difference_type;
		using iterator_category = std::forward_iterator_tag;

	private:
		swiss_node_iterator(meta_ptr meta, node_ptr node) noexcept : m_meta(meta), m_node(node) { if (meta) next_occupied(); }

	public:
		constexpr swiss_node_iterator() noexcept = default;
		template<typename U, typename = std::enable_if_t<!std::is_same_v<Node, U> && std::is_convertible_v<U *, node_ptr>>>
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

		[[nodiscard]] constexpr pointer operator->() const noexcept { return m_node; }
		[[nodiscard]] constexpr reference operator*() const noexcept { return *operator->(); }

		[[nodiscard]] constexpr bool operator==(const swiss_node_iterator &other) const noexcept { return m_meta == other.m_meta; }
#if (__cplusplus < 202002L && (!defined(_MSVC_LANG) || _MSVC_LANG < 202002L))
		[[nodiscard]] constexpr bool operator!=(const swiss_node_iterator &other) const noexcept { return m_meta != other.m_meta; }
#endif

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

	template<typename I, typename V, typename K, typename Kh, typename Kc, typename Alloc, typename ValueTraits>
	class swiss_table : ValueTraits::link_type, ebo_container<Kh>, ebo_container<Kc>, public swiss_node_traits<V, Alloc, ValueTraits>
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
		using value_allocator = typename bucket_node::allocator_type;

#ifdef TPP_DEBUG /* Use dual buffers in debug mode for ease of debugging. */
		class buffer_type : ebo_container<meta_allocator>, ebo_container<node_allocator>
		{
			using meta_alloc_base = ebo_container<meta_allocator>;
			using node_alloc_base = ebo_container<node_allocator>;

		public:
			constexpr buffer_type() noexcept = default;

			buffer_type(const buffer_type &other) : meta_alloc_base(allocator_copy(other.meta_alloc())), node_alloc_base(allocator_copy(other.node_alloc())) {}
			buffer_type(buffer_type &&other) : meta_alloc_base(std::move(other)), node_alloc_base(std::move(other)) {}

			buffer_type &operator=(const buffer_type &other)
			{
				if constexpr (std::allocator_traits<meta_allocator>::propagate_on_container_copy_assignment::value)
				{
					std::allocator_traits<meta_allocator>::deallocate(meta_alloc(), std::exchange(m_metadata, nullptr), capacity + sizeof(meta_block));
					meta_alloc_base::operator=(other);
				}
				if constexpr (std::allocator_traits<node_allocator>::propagate_on_container_copy_assignment::value)
				{
					std::allocator_traits<node_allocator>::deallocate(node_alloc(), std::exchange(m_nodes, nullptr), capacity);
					node_alloc_base::operator=(other);
				}
				if constexpr (std::allocator_traits<meta_allocator>::propagate_on_container_copy_assignment::value ||
							  std::allocator_traits<node_allocator>::propagate_on_container_copy_assignment::value)
					capacity = 0;
			}
			buffer_type &operator=(buffer_type &&other)
			noexcept(std::is_nothrow_move_assignable_v<meta_allocator> &&
					 std::is_nothrow_move_assignable_v<node_allocator>)
			{
				if constexpr (std::allocator_traits<meta_allocator>::propagate_on_container_move_assignment::value)
				{
					std::allocator_traits<meta_allocator>::deallocate(meta_alloc(), std::exchange(m_metadata, nullptr), capacity + sizeof(meta_block));
					meta_alloc_base::operator=(other);
				}
				if constexpr (std::allocator_traits<node_allocator>::propagate_on_container_move_assignment::value)
				{
					std::allocator_traits<node_allocator>::deallocate(node_alloc(), std::exchange(m_nodes, nullptr), capacity);
					node_alloc_base::operator=(other);
				}
				if constexpr (std::allocator_traits<meta_allocator>::propagate_on_container_move_assignment::value ||
							  std::allocator_traits<node_allocator>::propagate_on_container_move_assignment::value)
					capacity = 0;
			}

			buffer_type(const allocator_type &alloc) : meta_alloc_base(alloc), node_alloc_base(alloc) {}
			buffer_type(size_type capacity, const allocator_type &alloc) : meta_alloc_base(alloc), node_alloc_base(alloc)
			{
				m_buffer.allocate(capacity);
				m_buffer.fill_empty();
			}

			[[nodiscard]] meta_word *metadata() const noexcept { return m_metadata; }
			[[nodiscard]] meta_word *tail() const noexcept { return m_metadata + capacity + 1; }
			[[nodiscard]] bucket_node *nodes() const noexcept { return m_nodes; }

			void fill_empty() noexcept
			{
				std::fill_n(metadata(), capacity + sizeof(meta_block), meta_word::empty);
				metadata()[capacity] = meta_word::sentinel;
			}
			template<typename F>
			void resize(size_type n, F &&relocate)
			{
				auto *old_metadata = std::exchange(m_metadata, std::allocator_traits<meta_allocator>::allocate(meta_alloc(), n + sizeof(meta_block)));
				auto *old_nodes = std::exchange(m_nodes, std::allocator_traits<node_allocator>::allocate(node_alloc(), n));
				const auto old_capacity = std::exchange(capacity, n);
				fill_empty();

				if (old_capacity != 0)
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
			void reallocate(size_type n)
			{
				if (capacity < n)
				{
					if (capacity != 0)
					{
						std::allocator_traits<meta_allocator>::deallocate(meta_alloc(), m_metadata, capacity + sizeof(meta_block));
						std::allocator_traits<node_allocator>::deallocate(node_alloc(), m_nodes, capacity);
					}
					m_metadata = std::allocator_traits<meta_allocator>::allocate(meta_alloc(), n + sizeof(meta_block));
					m_nodes = std::allocator_traits<node_allocator>::allocate(node_alloc(), n);
					capacity = n;
				}
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

			bool can_swap(const buffer_type &other) const
			{
				return allocator_eq(meta_alloc(), other.meta_alloc()) && allocator_eq(node_alloc(), other.node_alloc());
			}
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
			[[nodiscard]] constexpr auto &meta_alloc() const noexcept { return meta_alloc_base::value(); }
			[[nodiscard]] constexpr auto &node_alloc() noexcept { return node_alloc_base::value(); }
			[[nodiscard]] constexpr auto &node_alloc() const noexcept { return node_alloc_base::value(); }

			meta_word *m_metadata = nullptr;
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
				return n + nodes_offset(n) / sizeof(bucket_node);
			}

		public:
			constexpr buffer_type() noexcept = default;

			buffer_type(const buffer_type &other) : alloc_base(allocator_copy(other.node_alloc())) {}
			buffer_type(buffer_type &&other) : alloc_base(std::move(other)) {}

			buffer_type &operator=(const buffer_type &other)
			{
				if constexpr (std::allocator_traits<node_allocator>::propagate_on_container_copy_assignment::value)
				{
					std::allocator_traits<node_allocator>::deallocate(node_alloc(), static_cast<bucket_node *>(m_data), capacity);
					alloc_base::operator=(other);
					m_data = nullptr;
					capacity = 0;
				}
			}
			buffer_type &operator=(buffer_type &&other) noexcept(std::is_nothrow_move_assignable_v<node_allocator>)
			{
				if constexpr (std::allocator_traits<node_allocator>::propagate_on_container_move_assignment::value)
				{
					std::allocator_traits<node_allocator>::deallocate(node_alloc(), static_cast<bucket_node *>(m_data), capacity);
					alloc_base::operator=(other);
					m_data = nullptr;
					capacity = 0;
				}
			}

			buffer_type(const allocator_type &alloc) : alloc_base(alloc) {}
			buffer_type(size_type capacity, const allocator_type &alloc) : alloc_base(alloc)
			{
				m_buffer.allocate(capacity);
				m_buffer.fill_empty();
			}

			[[nodiscard]] meta_word *metadata() const noexcept { return reinterpret_cast<meta_word *>(bytes()); }
			[[nodiscard]] meta_word *tail() const noexcept { return metadata() + capacity + 1; }
			[[nodiscard]] bucket_node *nodes() const noexcept { return m_data ? reinterpret_cast<bucket_node *>(bytes() + nodes_offset(capacity)) : nullptr; }

			void fill_empty() noexcept
			{
				std::fill_n(metadata(), capacity + sizeof(meta_block), meta_word::empty);
				metadata()[capacity] = meta_word::sentinel;
			}
			template<typename F>
			void resize(size_type n, F &&relocate)
			{
				auto *old_data = std::exchange(m_data, static_cast<void *>(std::allocator_traits<node_allocator>::allocate(node_alloc(), buffer_size(n))));
				const auto old_capacity = std::exchange(capacity, n);
				fill_empty();

				if (old_data != nullptr)
				{
					auto *old_nodes = reinterpret_cast<bucket_node *>(static_cast<std::uint8_t *>(old_data) + nodes_offset(old_capacity));
					auto *old_metadata = static_cast<meta_word *>(old_data);

					relocate(old_metadata, old_nodes, old_capacity);
					std::allocator_traits<node_allocator>::deallocate(node_alloc(), static_cast<bucket_node *>(old_data), buffer_size(n));
				}
			}

			void allocate(size_type n)
			{
				m_data = static_cast<void *>(std::allocator_traits<node_allocator>::allocate(node_alloc(), buffer_size(n)));
				capacity = n;
			}
			void reallocate(size_type n)
			{
				if (capacity < n)
				{
					if (capacity) std::allocator_traits<node_allocator>::deallocate(node_alloc(), static_cast<bucket_node *>(m_data), buffer_size(capacity));
					m_data = static_cast<void *>(std::allocator_traits<node_allocator>::allocate(node_alloc(), buffer_size(n)));
					capacity = n;
				}
			}
			void deallocate()
			{
				std::allocator_traits<node_allocator>::deallocate(node_alloc(), static_cast<bucket_node *>(m_data), buffer_size(capacity));
				m_data = nullptr;
				capacity = 0;
			}

			[[nodiscard]] auto &get_allocator() const noexcept { return alloc_base::value(); }

			bool can_swap(const buffer_type &other) const { return allocator_eq(node_alloc(), other.node_alloc()); }
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
			[[nodiscard]] constexpr auto &node_alloc() const noexcept { return alloc_base::value(); }

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

		[[nodiscard]] static constexpr std::pair<std::size_t, meta_word> decompose_hash(std::size_t h) noexcept
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
		swiss_table() = default;

		swiss_table(const allocator_type &alloc) : m_buffer(alloc) {}
		swiss_table(size_type bucket_count, const hasher &hash, const key_equal &cmp, const allocator_type &alloc)
				: hash_base(hash), cmp_base(cmp), m_buffer(bucket_count, alloc)
		{
			TPP_ASSERT(((bucket_count + 1) & bucket_count) == 0, "Capacity must be a power of 2 - 1");
			m_num_empty = capacity_to_max_size(bucket_count);
		}

		template<typename Iter>
		swiss_table(Iter first, Iter last, size_type bucket_count, const hasher &hash, const key_equal &cmp, const allocator_type &alloc)
				: swiss_table(max_distance_or_n(first, last, bucket_count), hash, cmp, alloc) { insert(first, last); }

		swiss_table(const swiss_table &other) : header_base(), hash_base(other), cmp_base(other), m_buffer(other.m_buffer)
		{
			copy_data(other);
		}
		swiss_table(const swiss_table &other, const allocator_type &alloc) : header_base(), hash_base(other), cmp_base(other), m_buffer(alloc)
		{
			copy_data(other);
		}

		swiss_table(swiss_table &&other)
		noexcept(std::is_nothrow_move_constructible_v<buffer_type> &&
		         std::is_nothrow_move_constructible_v<hasher> &&
		         std::is_nothrow_move_constructible_v<key_equal>)
				: header_base(std::move(other)),
				  hash_base(std::move(other)),
				  cmp_base(std::move(other)),
				  m_buffer(std::move(other.m_buffer))
		{
			move_from(other);
		}
		swiss_table(swiss_table &&other, const allocator_type &alloc)
		noexcept(std::is_nothrow_constructible_v<buffer_type, const allocator_type &> &&
		         std::is_nothrow_move_constructible_v<hasher> &&
		         std::is_nothrow_move_constructible_v<key_equal>)
				: header_base(std::move(other)),
				  hash_base(std::move(other)),
				  cmp_base(std::move(other)),
				  m_buffer(alloc)
		{
			move_from(other);
		}

		swiss_table &operator=(const swiss_table &other)
		{
			if (this != &other)
			{
				header_base::operator=(other);
				hash_base::operator=(other);
				cmp_base::operator=(other);

				erase_nodes();
				m_buffer = other.m_buffer;
				copy_data(other);
			}
			return *this;
		}
		swiss_table &operator=(swiss_table &&other)
		noexcept(std::is_nothrow_move_assignable_v<buffer_type> &&
		         std::is_nothrow_move_assignable_v<hasher> &&
		         std::is_nothrow_move_assignable_v<key_equal>)
		{
			if (this != &other)
			{
				header_base::operator=(std::move(other));
				hash_base::operator=(std::move(other));
				cmp_base::operator=(std::move(other));

				erase_nodes();
				m_buffer = std::move(other.m_buffer);
				move_from(other);
			}
			return *this;
		}

		~swiss_table()
		{
			if (m_size != 0) erase_nodes();
			m_buffer.deallocate();
		}

		template<typename Iter>
		void assign(Iter first, Iter last)
		{
			clear();
			insert(first, last);
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
				/* Reset header link. */
				if constexpr (is_ordered::value) *header_link() = bucket_link{};

				erase_nodes();
				m_buffer.fill_empty();
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

		template<typename N>
		auto insert_node(N &&node) -> typename stable_node<V, Alloc, ValueTraits>::template insert_return<iterator, N>
		{
			const auto h = hash(node.key());
			if (auto target_pos = find_node(node.key(), h); target_pos == m_buffer.capacity)
				return {emplace_node({}, h, std::forward<N>(node)), true};
			else
				return {to_iter(target_pos), false, std::forward<N>(node)};
		}
		template<typename N>
		auto insert_node(const_iterator hint, N &&node) -> iterator
		{
			const auto h = hash(node.key());
			if (auto target_pos = find_node(node.key(), h); target_pos == m_buffer.capacity)
				return emplace_node(hint, h, std::forward<N>(node));
			else
				return to_iter(target_pos);
		}

		template<typename Iter>
		void insert(Iter first, Iter last)
		{
			if constexpr (std::is_base_of_v<std::random_access_iterator_tag, typename std::iterator_traits<Iter>::iterator_category>)
				reserve(m_size + static_cast<size_type>(std::distance(first, last)));
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

		template<typename N>
		std::pair<iterator, bool> insert_or_assign_node(N &&node)
		{
			const auto h = hash(node.key());
			if (auto target_pos = find_node(node.key(), h); target_pos == m_buffer.capacity)
				return {emplace_node({}, h, std::forward<N>(node)), true};
			else
			{
				auto alloc = value_allocator{get_allocator()};
				m_buffer.nodes()[target_pos].construct(alloc, std::forward<N>(node));
				return {to_iter(target_pos), false};
			}
		}
		template<typename N>
		iterator insert_or_assign_node(hint_t hint, N &&node)
		{
			const auto h = hash(node.key());
			if (auto target_pos = find_node(node.key(), h); target_pos == m_buffer.capacity)
				return emplace_node(hint, h, std::forward<N>(node));
			else
			{
				auto alloc = value_allocator{get_allocator()};
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

		template<typename U, typename... Args>
		std::pair<iterator, bool> try_emplace(U &&key, Args &&...args) TPP_REQUIRES(
				(std::is_constructible_v<V, std::piecewise_construct_t, std::tuple<K &&>, std::tuple<Args && ...>>))
		{
			return try_emplace_impl({}, std::forward<U>(key), std::forward<Args>(args)...);
		}
		template<typename U, typename... Args>
		iterator try_emplace(const_iterator hint, U &&key, Args &&...args) TPP_REQUIRES(
				(std::is_constructible_v<V, std::piecewise_construct_t, std::tuple<K &&>, std::tuple<Args && ...>>))
		{
			return try_emplace_impl(to_underlying(hint), std::forward<U>(key), std::forward<Args>(args)...).first;
		}

		template<typename T, typename = std::enable_if_t<!std::is_convertible_v<T, const_iterator>>>
		iterator erase(const T &key)
		{
			if (const auto pos = find_node(key, hash(key)); pos != m_buffer.capacity)
				return erase_impl(pos);
			else
				return end();
		}
		iterator erase(const_iterator where)
		{
			if (where != const_iterator{end()})
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
		typename stable_node<V, Alloc, ValueTraits>::extracted_type extract(const T &key)
		{
			if (const auto pos = find_node(key, hash(key)); pos != m_buffer.capacity)
				return {value_allocator{get_allocator()}, std::move(extract_impl(pos))};
			else
				return {};
		}
		typename stable_node<V, Alloc, ValueTraits>::extracted_type extract(const_iterator where)
		{
			if (where != const_iterator{end()})
			{
				const auto pos = &(*to_underlying(where)) - m_buffer.nodes();
				return {value_allocator{get_allocator()}, std::move(extract_impl(pos))};
			}
			else
				return {};
		}

		template<typename Kh2, typename Kc2>
		void merge(swiss_table<I, V, K, Kh2, Kc2, Alloc, ValueTraits> &other)
		{
			reserve(m_size + other.size());

			/* Extract nodes from other and insert into this. */
			/* Iterate backwards for ordered links, since the ordered iterators are invalidated on erase. */
			const auto transfer = [&](const const_iterator &pos) noexcept
			{
				const auto &key = ValueTraits::get_key(*pos);
				const auto h = hash(key);
				if (find_node(key, h) == m_buffer.capacity) emplace_node({}, h, other.extract(pos));
			};
			if constexpr (!is_ordered::value)
				for (auto pos = other.begin(), last = other.end(); pos != last; ++pos) transfer(pos);
			else
				for (auto pos = other.end(), last = other.begin(); --pos != last;) transfer(pos);
		}
		template<typename Kh2, typename Kc2>
		void merge(swiss_table<I, V, K, Kh2, Kc2, Alloc, ValueTraits> &&other) { merge(other); }
		void merge(swiss_table &&other)
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
			swap_buffers(other);
		}

	private:
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

		void assert_probe(bucket_probe probe) const noexcept
		{
			TPP_ASSERT(probe.idx < m_buffer.capacity, "Probe must not exceed table capacity");
		}

		template<typename T>
		size_type find_node(const T &key, std::size_t h) const
		{
			TPP_IF_LIKELY(m_size != 0)
			{
				const auto [h1, h2] = decompose_hash(h);
				auto *metadata = m_buffer.metadata();
				auto *nodes = m_buffer.nodes();
				for (auto probe = bucket_probe{h1 & m_buffer.capacity, m_buffer.capacity};; ++probe)
				{
					/* Go through each matched element in the block and test for equality. */
					const auto block = meta_block{metadata + probe.pos};
					for (auto match = block.match_eq(h2); !match.empty(); ++match)
					{
						const auto offset = probe.off(match.lsb_index());
						TPP_IF_LIKELY(cmp(nodes[offset].key(), key)) return offset;
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
			auto *metadata = m_buffer.metadata();
			for (auto probe = bucket_probe{h1 & m_buffer.capacity, m_buffer.capacity};; ++probe)
			{
				const auto block = meta_block{metadata + probe.pos};
				if (const auto match = block.match_available(); !match.empty())
					return probe.off(match.lsb_index());
				assert_probe(probe);
			}
		}

		void set_metadata(size_type pos, meta_word value) noexcept
		{
			constexpr auto tail_size = sizeof(meta_block) - 1;
			auto *metadata = m_buffer.metadata();
			metadata[((pos - tail_size) & m_buffer.capacity) + (tail_size & m_buffer.capacity)] = value;
			metadata[pos] = value;
		}

		void insert_link(hint_t hint, bucket_node *node) noexcept
		{
			if constexpr (is_ordered::value) node->link(hint.link ? const_cast<bucket_link *>(hint.link) : back_node());
		}
		template<typename... Args>
		iterator emplace_node(hint_t hint, std::size_t h, Args &&...args)
		{
			size_type target_pos;
			TPP_IF_UNLIKELY(m_buffer.capacity == 0)
			{
				rehash_impl(1);
				target_pos = find_available(h);
			} else
			{
				target_pos = find_available(h);
				TPP_IF_UNLIKELY(!m_num_empty && m_buffer.metadata()[target_pos] != meta_word::deleted)
				{
					/* Do an in-place rehash by reclaiming deleted entries. Choice of coefficients is outlined by the reference implementation at
					 * https://github.com/abseil/abseil-cpp/blob/f7affaf32a6a396465507dd10520a3fe183d4e40/absl/container/internal/raw_hash_set.cc#L97
					 *
					 * NOTE: Reclaiming deleted entries may leave blocks partially empty. */
					if (m_buffer.capacity > sizeof(meta_block) && static_cast<std::uint64_t>(m_size) * 32 <= static_cast<std::uint64_t>(m_buffer.capacity) * 25)
						rehash_deleted();
					else
						rehash_impl((m_buffer.capacity + 1) * 2 - 1);
					target_pos = find_available(h);
				}
			}

			auto *target = m_buffer.nodes() + target_pos;
			auto alloc = value_allocator{get_allocator()};
			target->construct(alloc, std::forward<Args>(args)...);
			insert_link(hint, target);
			target->hash() = h;

			m_num_empty -= m_buffer.metadata()[target_pos] == meta_word::empty;
			set_metadata(target_pos, decompose_hash(h).second);
			++m_size;

			return to_iter(target_pos);
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
			set_metadata(pos, meta_word::deleted);
			--m_size;
			return to_iter(next);
		}

		template<typename... Args>
		std::pair<iterator, bool> emplace_impl(hint_t hint, Args &&...args)
		{
			auto alloc = value_allocator{get_allocator()};
			auto tmp = bucket_node{};

			tmp.construct(alloc, std::forward<Args>(args)...);
			auto result = insert_impl(hint, tmp.key(), std::move(tmp));
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
			if (auto target_pos = find_node(key, h); target_pos == m_buffer.capacity)
				return {emplace_node(hint, h, std::forward<Args>(args)...), true};
			else
				return {to_iter(target_pos), false};
		}
		template<typename T, typename... Args>
		std::pair<iterator, bool> insert_or_assign_impl(hint_t hint, T &&key, Args &&...args)
		{
			const auto h = hash(key);
			if (auto target_pos = find_node(key, h); target_pos == m_buffer.capacity)
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
			auto alloc = value_allocator{get_allocator()};
			auto *node = m_buffer.nodes() + pos;
			auto result = erase_node(pos, node);
			node->destroy(alloc);
			return result;
		}

		void rehash_impl(size_type capacity)
		{
			m_buffer.resize(capacity, [&](meta_word *src_meta, bucket_node *src_nodes, size_type src_capacity)
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
			auto *metadata = m_buffer.metadata(), *tail = m_buffer.tail();
			auto *nodes = m_buffer.nodes();

			/* Set all occupied as deleted. */
			for (size_type i = 0; i < m_buffer.capacity; i += sizeof(meta_block))
			{
				const auto old_block = metadata + i;
				const auto new_block = meta_block{old_block}.reset_occupied();
				std::memcpy(old_block, reinterpret_cast<const meta_word *>(&new_block), sizeof(meta_block));
			}
			std::memcpy(tail, metadata, sizeof(meta_block) - 1);
			metadata[m_buffer.capacity] = meta_word::sentinel;

			/* Relocation algorithm as described by the reference implementation at https://github.com/abseil/abseil-cpp/blob/f7affaf32a6a396465507dd10520a3fe183d4e40/absl/container/internal/raw_hash_set.cc#L97 */
			for (size_type i = 0; i < m_buffer.capacity; ++i)
				if (metadata[i] == meta_word::deleted)
				{
					auto *node = nodes + i;

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
					if (metadata[target_pos] == meta_word::empty)
					{
						auto alloc = value_allocator{get_allocator()};
						relocate_node{}(alloc, node, alloc, nodes + target_pos);
						set_metadata(i, meta_word::empty);
						set_metadata(target_pos, h2);
					}
					else
					{
						using std::swap;
						swap(*node, nodes[target_pos]);
						set_metadata(target_pos, h2);
						goto process_node; /* Process the swapped-with node. */
					}
				}

			/* Reset the amount of empty slots left. */
			m_num_empty = capacity_to_max_size(m_buffer.capacity) - m_size;
		}

		void copy_data(const swiss_table &other)
		{
			/* Expect that there is no data in the buffers, but the buffers might still exist. */
			TPP_ASSERT(m_size == 0, "Table must be empty prior to copying elements");

			/* Ignore empty tables. */
			TPP_IF_UNLIKELY(other.m_size == 0)
			{
				m_size = 0;
				m_num_empty = m_buffer.capacity ? capacity_to_max_size(m_buffer.capacity) : 0;
				return;
			}

			/* (re)allocate the metadata & node buffers if needed. */
			m_buffer.reallocate(other.m_buffer.capacity);
			m_buffer.fill_empty();

			/* Copy elements from the other table. */
			auto alloc = value_allocator{get_allocator()};
			auto *nodes = m_buffer.nodes(), *other_nodes = other.m_buffer.nodes();
			auto *other_metadata = other.m_buffer.metadata();
			for (size_type i = 0; i != other.m_buffer.capacity; ++i)
				if (other_metadata[i].is_occupied())
				{
					nodes[i].construct(alloc, other_nodes[i]);
					set_metadata(i, other_metadata[i]);
				}
			m_size = other.m_size;
			m_num_empty = capacity_to_max_size(m_buffer.capacity) - m_size;

			/* If the node link is ordered, update header offsets to point to the copied data. */
			if constexpr (is_ordered::value)
				if (m_size != 0)
				{
					const auto front_off = other.front_node() - other_nodes;
					const auto back_off = other.back_node() - other_nodes;
					header_base::link(nodes + front_off, nodes + back_off);
				}
		}
		void move_data(swiss_table &other)
		{
			/* Expect that there is no data in the buffers, but the buffers might still exist. */
			TPP_ASSERT(size() == 0, "Table must be empty prior to moving elements");

			/* Ignore empty tables. */
			TPP_IF_UNLIKELY(other.m_size == 0)
			{
				m_size = 0;
				m_num_empty = m_buffer.capacity ? capacity_to_max_size(m_buffer.capacity) : 0;
				return;
			}

			/* (re)allocate the metadata & node buffers if needed. */
			m_buffer.reallocate(other.m_buffer.capacity);
			m_buffer.fill_empty();

			/* Move elements from the other table. */
			auto alloc = value_allocator{get_allocator()}, other_alloc = value_allocator{other.get_allocator()};
			auto *nodes = m_buffer.nodes(), *other_nodes = other.m_buffer.nodes();
			auto *other_metadata = other.m_buffer.metadata();
			for (size_type i = 0; i != other.m_buffer.capacity; ++i)
				if (other_metadata[i].is_occupied())
				{
					auto *src_node = other_nodes + i, *dst_node = nodes + i;
					dst_node->construct(alloc, std::move(*src_node));
					src_node->destroy(other_alloc);
					set_metadata(i, other_metadata[i]);
				}
			m_size = other.m_size;
			m_num_empty = capacity_to_max_size(m_buffer.capacity) - m_size;

			/* Reset the other table. */
			other.m_size = 0;
			other.m_num_empty = capacity_to_max_size(other.m_buffer.capacity);
			other.m_buffer.fill_empty();
		}
		void erase_nodes()
		{
			auto alloc = value_allocator{get_allocator()};
			auto *metadata = m_buffer.metadata();
			auto *nodes = m_buffer.nodes();
			for (size_type i = 0; i != m_buffer.capacity; ++i) if (metadata[i].is_occupied()) nodes[i].destroy(alloc);
		}
		void swap_buffers(swiss_table &other)
		{
			std::swap(m_size, other.m_size);
			std::swap(m_num_empty, other.m_num_empty);
			m_buffer.swap(other.m_buffer);
		}
		void move_from(swiss_table &other)
		{
			if (m_buffer.can_swap(other.m_buffer))
				swap_buffers(other);
			else
				move_data(other);
		}

		size_type m_size = 0;       /* Amount of occupied nodes. */
		size_type m_num_empty = 0;  /* Amount of empty entries we can still use. */
		buffer_type m_buffer = {};
	};
}