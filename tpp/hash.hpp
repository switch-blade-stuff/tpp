/*
 * Created by switchblade on 2022-11-4.
 */

#pragma once

#include "detail/utility.hpp"

#ifndef TPP_USE_IMPORT
#include <functional>
#include <limits>
#endif

#ifndef TPP_NO_HASH
namespace tpp
{
	namespace _detail
	{
		inline constexpr std::size_t fnv1a_prime = sizeof(std::size_t) >= sizeof(std::uint64_t) ? 0x00000100000001b3 : 0x01000193;
		inline constexpr std::size_t fnv1a_offset = sizeof(std::size_t) >= sizeof(std::uint64_t) ? 0xcbf29ce484222325 : 0x811c9dc5;
	}

	/** @brief SDBM byte hash function.
	 * @param[in] data Pointer to bytes of input data.
	 * @param[in] n Size of the \p data buffer. */
	[[nodiscard]] inline TPP_FORCEINLINE std::size_t sdbm(const void *data, std::size_t n) noexcept;
	/** @copydoc sdbm
	 * @param[in] seed Seed used for the hash algorithm. */
	[[nodiscard]] inline std::size_t sdbm(const void *data, std::size_t n, std::size_t seed) noexcept
	{
		auto *bytes = static_cast<const std::uint8_t *>(data);
		std::size_t result = seed;
		for (std::size_t i = 0; i < n; ++i)
		{
			const auto word = static_cast<std::size_t>(bytes[i]);
			result = word + (result << 6) + (result << 16) - result;
		}
		return result;
	}

	inline TPP_FORCEINLINE std::size_t sdbm(const void *data, std::size_t n) noexcept { return sdbm(data, n, 0); }

	/** @brief FNV1a byte hash function.
	 * @param[in] data Pointer to bytes of input data.
	 * @param[in] n Size of the \p data buffer. */
	[[nodiscard]] inline TPP_FORCEINLINE std::size_t fnv1a(const void *data, std::size_t n) noexcept;
	/** @copydoc fnv1a
	 * @param[in] seed Seed used for the hash algorithm. */
	[[nodiscard]] inline std::size_t fnv1a(const void *data, std::size_t n, std::size_t seed) noexcept
	{
		auto *bytes = static_cast<const std::uint8_t *>(data);
		std::size_t result = seed;
		for (std::size_t i = 0; i < n; ++i)
		{
			result ^= bytes[i];
			result *= _detail::fnv1a_prime;
		}
		return result;
	}

	inline TPP_FORCEINLINE std::size_t fnv1a(const void *data, std::size_t n) noexcept { return fnv1a(data, n, _detail::fnv1a_offset); }

	namespace _detail
	{
		template<typename T>
		struct is_hash_builder
		{

		};
		template<typename T>
		struct is_hash_builder_for
		{

		};
	};

	/** @brief Combines hash of the value type with the seed.
	 * @param[in] seed Initial seed to combine the hash with.
	 * @param[in] value Object to hash.
	 * @param[in] hash Hash functor.
	 * @param[in] offset Offset constant used when combining hash values.
	 * @return Copy of \p seed. */
	template<typename T, typename H>
	[[nodiscard]] constexpr std::size_t hash_combine(std::size_t seed, const T &value, const H &hash, std::size_t offset = 0x9e3779b9) noexcept
	{
		return seed ^ (hash(value) + offset + (seed << 6) + (seed >> 2));
	}

}
#endif
