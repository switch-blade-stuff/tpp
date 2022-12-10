/*
 * Created by switchblade on 11/27/22.
 */

#pragma once

#include "table_util.hpp"

#ifndef TPP_USE_IMPORT

#include <vector>
#include <limits>
#include <tuple>

#endif

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

namespace tpp::detail
{
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

		[[nodiscard]] constexpr bool empty() const noexcept { return m_value == 0; }
		[[nodiscard]] constexpr std::size_t lsb_index() const noexcept;
		[[nodiscard]] constexpr std::size_t msb_index() const noexcept;

		[[nodiscard]] constexpr bool operator==(const basic_index_mask &other) const noexcept { return m_value == other.m_value; }
#if __cplusplus < 202002L
		[[nodiscard]] constexpr bool operator!=(const basic_index_mask &other) const noexcept { return m_value != other.m_value; }
#endif

	private:
		value_type m_value = 0;
	};

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
	[[nodiscard]] constexpr std::size_t ctz(T value) noexcept
	{
		TPP_IF_CONSTEVAL
		{
			return generic_ctz(value);
		}
		else
		{
			return builtin_ctz(value);
		}
	}
	template<typename T>
	[[nodiscard]] constexpr std::size_t clz(T value) noexcept
	{
		TPP_IF_CONSTEVAL
		{
			return generic_clz(value);
		}
		else
		{
			return builtin_clz(value);
		}
	}

	template<typename T, std::size_t P>
	constexpr std::size_t basic_index_mask<T, P>::lsb_index() const noexcept { return ctz(m_value) >> P; }
	template<typename T, std::size_t P>
	constexpr std::size_t basic_index_mask<T, P>::msb_index() const noexcept { return clz(m_value) >> P; }

#ifdef TPP_HAS_SSE2
	using index_mask = basic_index_mask<std::uint16_t, 0>;
	using block_value = __m128i;
#else
	using index_mask = basic_index_mask<std::uint64_t, 3>;
	using block_value = std::uint64_t;
#endif

	class alignas(block_value) meta_block : public std::array<meta_byte, sizeof(block_value)>
	{
		using array_base = std::array<meta_byte, sizeof(block_value)>;

	public:
		constexpr meta_block() noexcept : array_base{} { fill(meta_byte::EMPTY); }

		constexpr void fill(meta_byte b) noexcept;

		[[nodiscard]] constexpr index_mask match_empty() const noexcept;
		[[nodiscard]] constexpr index_mask match_available() const noexcept;
		[[nodiscard]] constexpr index_mask match_eq(meta_byte b) const noexcept;

	private:
		[[nodiscard]] constexpr void *raw_data() noexcept { return array_base::data(); }
		[[nodiscard]] constexpr const void *raw_data() const noexcept { return array_base::data(); }

		[[nodiscard]] constexpr block_value &value() noexcept { return *static_cast<block_value *>(raw_data()); }
		[[nodiscard]] constexpr const block_value &value() const noexcept { return *static_cast<const block_value *>(raw_data()); }
	};

#ifdef TPP_HAS_SSE2
	constexpr void meta_block::fill(meta_byte b) noexcept
	{
		TPP_IF_CONSTEVAL for (std::size_t i = 0; i < sizeof(block_value); ++i)
				(*this)[i] = b;
		else
			value() = _mm_set1_epi8(static_cast<char>(b));
	}

	constexpr index_mask meta_block::match_empty() const noexcept
	{
#ifdef TPP_HAS_SSSE3
		return index_mask{static_cast<index_mask::value_type>(_mm_movemask_epi8(_mm_sign_epi8(value(), value())))};
#else
		return match_eq(meta_byte::EMPTY);
#endif
	}
	constexpr index_mask meta_block::match_available() const noexcept
	{
		index_mask::value_type result = 0;
		TPP_IF_CONSTEVAL for (std::size_t i = 0; i < sizeof(block_value); ++i)
			{
				const auto a = static_cast<std::int8_t>(meta_byte::SENTINEL);
				const auto b = static_cast<std::int8_t>((*this)[i]);
				result |= static_cast<index_mask::value_type>(a > b) << i;
			}
		else
		{
			/* https://gcc.gnu.org/bugzilla/show_bug.cgi?id=87853 */
			constexpr auto x86_cmpgt_epi8 = [](__m128i a, __m128i b) noexcept -> __m128i
			{
#if defined(__GNUC__) && !defined(__clang__)
				if constexpr (std::is_unsigned_v<char>)
				{
					const auto mask = _mm_set1_epi8(static_cast<char>(0x80));
					const auto diff = _mm_subs_epi8(b, a);
					return _mm_cmpeq_epi8(_mm_and_si128(diff, mask), mask);
				}
				else
#endif
					return _mm_cmpgt_epi8(a, b);
			};

			const auto mask = _mm_set1_epi8(static_cast<char>(meta_byte::SENTINEL));
			result = static_cast<index_mask::value_type>(_mm_movemask_epi8(x86_cmpgt_epi8(mask, value())));
		}
		return index_mask{result};
	}
	constexpr index_mask meta_block::match_eq(meta_byte b) const noexcept
	{
		index_mask::value_type result = 0;
		TPP_IF_CONSTEVAL for (std::size_t i = 0; i < sizeof(block_value); ++i)
				result |= static_cast<index_mask::value_type>((*this)[i] == b) << i;
		else
		{
			const auto mask_vec = _mm_set1_epi8(static_cast<char>(b));
			result = static_cast<index_mask::value_type>(_mm_movemask_epi8(_mm_cmpeq_epi8(mask_vec, value())));
		}
		return index_mask{result};
	}
#else
	constexpr void meta_block::fill(meta_byte b) noexcept
	{
		TPP_IF_CONSTEVAL for (std::size_t i = 0; i < sizeof(block_value); ++i)
				(*this)[i] = b;
		else
			std::memset(raw_data(), static_cast<int>(b), sizeof(block_value));
	}

	constexpr index_mask meta_block::match_empty() const noexcept { return match_eq(meta_byte::EMPTY); }
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

	template<typename V, typename K, typename KHash, typename KCmp, typename Alloc, typename ValueTraits>
	class swiss_table : ValueTraits::link_type, ebo_container<KHash>, ebo_container<KCmp>
	{
		using traits_t = table_traits<V, V, K, KHash, KCmp, Alloc>;

	public:
		typedef typename traits_t::insert_type insert_type;
		typedef typename traits_t::value_type value_type;
		typedef typename traits_t::key_type key_type;

		typedef typename traits_t::hasher hasher;
		typedef typename traits_t::key_equal key_equal;
		typedef typename traits_t::allocator_type allocator_type;

		typedef typename traits_t::size_type size_type;
		typedef typename traits_t::difference_type difference_type;

		typedef std::conjunction<detail::is_transparent<hasher>, detail::is_transparent<key_equal>> is_transparent;
		typedef detail::is_ordered<typename ValueTraits::link_type> is_ordered;

		constexpr static float initial_load_factor = traits_t::initial_load_factor;
		constexpr static typename traits_t::size_type initial_capacity = traits_t::initial_capacity;

	private:
		using bucket_link = typename ValueTraits::link_type;
		using bucket_node = typename ValueTraits::template node_type<V, ValueTraits>;
	};
}