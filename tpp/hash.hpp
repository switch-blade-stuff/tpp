/*
 * Created by switchblade on 11/4/22.
 */

#pragma once

#include "detail/common.hpp"

#ifndef TPP_NO_HASH

/* If there is no modules support, include what we need. */
#ifndef TPP_USE_IMPORT

#include <cstring>
#include <cstdint>
#include <climits>

#endif

namespace tpp
{
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

	namespace detail
	{
		constexpr inline std::uint32_t crc32_table[] = {
				0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
				0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
				0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
				0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
				0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
				0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
				0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
				0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
				0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
				0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
				0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
				0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
				0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
				0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
				0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
				0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
				0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
				0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
				0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
				0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
				0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
				0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
				0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
				0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
				0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
				0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
				0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
				0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
				0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
				0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
				0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
				0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
				0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
				0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
				0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
				0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
				0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
				0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
				0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
				0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
				0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
				0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
				0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
		};
	}

	/** @brief CRC32 byte checksum function.
	 * @param[in] data Pointer to bytes of input data.
	 * @param[in] n Size of the \p data buffer. */
	[[nodiscard]] constexpr std::uint32_t crc32(const void *data, std::size_t n) noexcept
	{
		auto *bytes = static_cast<const std::uint8_t *>(data);
		std::uint32_t result = UINT32_MAX;
		for (std::size_t i = 0; i < n; i++)
		{
			const auto word = static_cast<std::uint32_t>(bytes[i]);
			const auto idx = (result ^ word) & 0xff;
			result = (result >> 8) ^ detail::crc32_table[idx];
		}
		return ~result;
	}

	namespace detail
	{
		constexpr inline std::uint32_t md5_a = 0x67452301;
		constexpr inline std::uint32_t md5_b = 0xefcdab89;
		constexpr inline std::uint32_t md5_c = 0x98badcfe;
		constexpr inline std::uint32_t md5_d = 0x10325476;

		constexpr inline int md5_s[] = {
				7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 5, 9, 14, 20, 5, 9,
				14, 20, 5, 9, 14, 20, 5, 9, 14, 20, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
				4, 11, 16, 23, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21,
		};
		constexpr inline std::uint32_t md5_k[] = {
				0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
				0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
				0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa, 0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
				0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
				0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
				0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
				0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
				0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1, 0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391,
		};
		constexpr inline std::uint8_t md5_pad[] = {
				0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		};

		template<typename T>
		[[nodiscard]] constexpr T md5_f(T x, T y, T z) noexcept { return ((x & y) | (~x & z)); }
		template<typename T>
		[[nodiscard]] constexpr T md5_g(T x, T y, T z) noexcept { return ((x & z) | (y & ~z)); }
		template<typename T>
		[[nodiscard]] constexpr T md5_h(T x, T y, T z) noexcept { return (x ^ y ^ z); }
		template<typename T>
		[[nodiscard]] constexpr T md5_i(T x, T y, T z) noexcept { return (y ^ (x | ~z)); }
	}     // namespace detail

	/** @brief MD5 byte hash function.
	 * @param[in] data Pointer to bytes of input data.
	 * @param[in] n Size of the \p data buffer.
	 * @param[out] result Buffer of 16 bytes to write the hash result to. */
	constexpr void md5(const void *data, std::size_t n, std::uint8_t (&result)[16]) noexcept
	{
		constexpr auto rotl = [](std::uint32_t x, std::uint32_t s) noexcept
		{
			constexpr auto N = UINT32_MAX;
			const auto r = s % N;
			return (x << r) | (x >> (N - r));
		};

		std::uint32_t buffer[4] = {detail::md5_a, detail::md5_b, detail::md5_c, detail::md5_d};
		std::uint8_t input[64] = {};
		std::uint64_t size = 0;

		const auto step = [&](const std::uint32_t data[]) noexcept
		{
			std::uint32_t a = buffer[0], b = buffer[1], c = buffer[2], d = buffer[3];
			std::uint32_t e;
			for (std::size_t j, i = 0; i < 64; ++i)
			{
				switch (i / 16)
				{
					case 0:
					{
						e = detail::md5_f(b, c, d);
						j = i;
						break;
					}
					case 1:
					{
						e = detail::md5_g(b, c, d);
						j = ((i * 5) + 1) % 16;
						break;
					}
					case 2:
					{
						e = detail::md5_h(b, c, d);
						j = ((i * 3) + 5) % 16;
						break;
					}
					default:
					{
						e = detail::md5_i(b, c, d);
						j = (i * 7) % 16;
						break;
					}
				}

				const auto temp = d;
				d = c;
				c = b;
				b = b + rotl(a + e + detail::md5_k[i] + data[j], detail::md5_s[i]);
				a = temp;
			}

			buffer[0] += a;
			buffer[1] += b;
			buffer[2] += c;
			buffer[3] += d;
		};
		const auto update = [&](const std::uint8_t data[], std::uint64_t n) noexcept
		{
			std::uint32_t work_buff[16];
			auto offset = size % 64;
			size += static_cast<std::uint64_t>(n);
			for (std::size_t i = 0; i < n; ++i)
			{
				input[offset++] = data[i];
				if (offset % 64 == 0)
				{
					for (std::size_t j = 0; j < 16; ++j)
					{
						work_buff[j] = static_cast<std::uint32_t>(input[(j * 4) + 3]) << 24 |
						               static_cast<std::uint32_t>(input[(j * 4) + 2]) << 16 |
						               static_cast<std::uint32_t>(input[(j * 4) + 1]) << 8 |
						               static_cast<std::uint32_t>(input[(j * 4)]);
					}
					step(work_buff);
					offset = 0;
				}
			}
		};

		/* Update for data. */
		update(static_cast<const std::uint8_t *>(data), n);

		{ /* Finalize md5 */
			std::uint32_t work_buff[16] = {};
			const auto offset = size % 64;
			const auto padding_size = offset < 56 ? 56 - offset : (56 + 64) - offset;

			update(detail::md5_pad, padding_size);
			size -= padding_size;

			for (std::size_t j = 0; j < 14; ++j)
			{
				work_buff[j] = static_cast<std::uint32_t>(input[(j * 4) + 3]) << 24 |
				               static_cast<std::uint32_t>(input[(j * 4) + 2]) << 16 |
				               static_cast<std::uint32_t>(input[(j * 4) + 1]) << 8 |
				               static_cast<std::uint32_t>(input[(j * 4)]);
			}

			work_buff[14] = static_cast<std::uint32_t>(size * 8);
			work_buff[15] = static_cast<std::uint32_t>((size * 8) >> 32);
			step(work_buff);

			for (unsigned int i = 0; i < 4; ++i)
			{
				result[(i * 4) + 0] = static_cast<std::uint8_t>((buffer[i] & 0x000000ff));
				result[(i * 4) + 1] = static_cast<std::uint8_t>((buffer[i] & 0x0000ff00) >> 8);
				result[(i * 4) + 2] = static_cast<std::uint8_t>((buffer[i] & 0x00ff0000) >> 16);
				result[(i * 4) + 3] = static_cast<std::uint8_t>((buffer[i] & 0xff000000) >> 24);
			}
		}
	}

	namespace detail
	{
#if SIZE_MAX < INT64_MAX // Select fnv1a constants for 32-bit hashes
		constexpr std::size_t fnv1a_prime = 0x01000193;
		constexpr std::size_t fnv1a_offset = 0x811c9dc5;
#else
		constexpr std::size_t fnv1a_prime = 0x00000100000001b3;
		constexpr std::size_t fnv1a_offset = 0xcbf29ce484222325;
#endif
	}

	/** @brief SDBM byte hash function.
	 * @param[in] data Pointer to bytes of input data.
	 * @param[in] n Size of the \p data buffer. */
	[[nodiscard]] constexpr std::size_t sdbm(const void *data, std::size_t n) noexcept;
	/** @copydoc sdbm
	 * @param[in] seed Seed used for the hash algorithm. */
	[[nodiscard]] constexpr std::size_t sdbm(const void *data, std::size_t n, std::size_t seed) noexcept
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

	constexpr std::size_t sdbm(const void *data, std::size_t n) noexcept
	{
		return sdbm(data, n, 0);
	}

	/** @brief FNV1a byte hash function.
	 * @param[in] data Pointer to bytes of input data.
	 * @param[in] n Size of the \p data buffer. */
	[[nodiscard]] constexpr std::size_t fnv1a(const void *data, std::size_t n) noexcept;
	/** @copydoc fnv1a
	 * @param[in] seed Seed used for the hash algorithm. */
	[[nodiscard]] constexpr std::size_t fnv1a(const void *data, std::size_t n, std::size_t seed) noexcept
	{
		auto *bytes = static_cast<const std::uint8_t *>(data);
		std::size_t result = seed;
		for (std::size_t i = 0; i < n; ++i)
		{
			result ^= bytes[i];
			result *= detail::fnv1a_prime;
		}
		return result;
	}

	constexpr std::size_t fnv1a(const void *data, std::size_t n) noexcept
	{
		return fnv1a(data, n, detail::fnv1a_offset);
	}

	/** @brief Helper structure used to assemble a SeaHash hash.
	 * @note Reference implementation at <a href="https://docs.rs/seahash/latest/src/seahash/stream.rs.html">https://docs.rs/seahash/latest/src/seahash/stream.rs.html</a>. */
	class seahash_builder
	{
		[[nodiscard]] constexpr TPP_FORCEINLINE static std::uint64_t read_u64_buff(const void *data, std::size_t n) noexcept
		{
			switch (n)
			{
				case 1: return static_cast<std::uint64_t>(*static_cast<const std::uint8_t *>(data));
				case 2: return static_cast<std::uint64_t>(*static_cast<const std::uint16_t *>(data));
				case 3:
				{
					const auto h = static_cast<std::uint64_t>(*(static_cast<const std::uint8_t *>(data) + 2));
					const auto l = static_cast<std::uint64_t>(*static_cast<const std::uint16_t *>(data));
					return l | (h << 16);
				}
				case 4: return static_cast<std::uint64_t>(*static_cast<const std::uint32_t *>(data));
				case 5:
				{
					const auto h = static_cast<std::uint64_t>(*(static_cast<const std::uint8_t *>(data) + 4));
					const auto l = static_cast<std::uint64_t>(*static_cast<const std::uint32_t *>(data));
					return l | (h << 32);
				}
				case 6:
				{
					const auto h = static_cast<std::uint64_t>(*(static_cast<const std::uint16_t *>(data) + 2));
					const auto l = static_cast<std::uint64_t>(*static_cast<const std::uint32_t *>(data));
					return l | (h << 32);
				}
				case 7:
				{
					const auto a = static_cast<std::uint64_t>(*(static_cast<const std::uint16_t *>(data) + 2));
					const auto b = static_cast<std::uint64_t>(*(static_cast<const std::uint8_t *>(data) + 6));
					const auto c = static_cast<std::uint64_t>(*static_cast<const std::uint32_t *>(data));
					return c | (a << 32) | (b << 48);
				}
				default: return 0;
			}
		}
		[[nodiscard]] constexpr TPP_FORCEINLINE static std::uint64_t read_u64_aligned(const void *data) noexcept
		{
			return *static_cast<const std::uint64_t *>(data);
		}
		[[nodiscard]] constexpr TPP_FORCEINLINE static std::uint64_t diffuse(std::uint64_t x) noexcept
		{
			/* Diffuse constants from the reference implementation at `https://docs.rs/seahash/latest/src/seahash/helper.rs.html#85`. */
			const std::uint64_t c = 0x6eed0e9da4d94a4f;
			x *= c;
			const auto a = x >> 32;
			const auto b = x >> 60;
			x ^= a >> b;
			return x * c;
		}

	public:
		/** Initializes SeaHash builder with a default seed. */
		constexpr seahash_builder() noexcept = default;
		/** Initializes SeaHash builder with the specified seed. */
		constexpr seahash_builder(const std::uint64_t (&seed)[4]) noexcept : seahash_builder(seed[0], seed[1], seed[2], seed[3]) {}
		/** @copydoc seahash_builder */
		constexpr seahash_builder(std::uint64_t s0, std::uint64_t s1, std::uint64_t s2, std::uint64_t s3) noexcept : state{s0, s1, s2, s3} {}

		/** Writes a scalar value to the resulting hash.
		 * @param value Scalar value to be added to the resulting hash.
		 * @return Reference to this hash builder.
		 * @note This overload is available only if `T` is a scalar type. */
		template<typename T>
		constexpr std::enable_if_t<std::is_scalar_v<std::decay_t<T>>, seahash_builder &> write(const T &value) noexcept
		{
			push(&value, sizeof(std::decay_t<T>));
			return *this;
		}
		/** Writes a buffer of bytes to the resulting hash.
		 * @param data Pointer to the source buffer to be added to the resulting hash.
		 * @param n Size of the source buffer in bytes.
		 * @return Reference to this hash builder. */
		constexpr seahash_builder &write(const void *data, std::size_t n) noexcept
		{
			push(data, n);
			return *this;
		}

		/** Finalizes the hash and returns it's value.
		 * @note Returned hash is always a 64-bit unsigned integer, even if `std::size_t` is 32-bit. */
		[[nodiscard]] constexpr std::uint64_t finish() noexcept
		{
			const auto a = tail_n > 0 ? diffuse(state[0] ^ read_u64_buff(&tail, tail_n)) : 0;
			return diffuse(a ^ state[1] ^ state[2] ^ state[3] ^
			               static_cast<std::uint64_t>(written) +
			               static_cast<std::uint64_t>(tail_n));
		}

	private:
		constexpr TPP_FORCEINLINE void push(const void *data, std::size_t n) noexcept
		{
			const auto overflow = 8 - tail_n;
			const auto copy_n = overflow < n ? overflow : n;
			auto *bytes = static_cast<const std::uint8_t *>(data);

			std::uint64_t tail_tmp = read_u64_buff(bytes + tail_n, copy_n);
			if (copy_n + tail_n != 8)
			{
				tail = tail_tmp;
				tail_n += copy_n;
			}
			else
			{
				push_u64(tail_tmp);
				tail_n = 0;
				tail = 0;

				const auto *ptr = bytes + copy_n;
				const auto *end = ptr + ((n - copy_n) & ~static_cast<std::size_t>(0x1F));
				while (ptr < end)
				{
					state[0] = diffuse(state[0] ^ read_u64_aligned(ptr));
					state[1] = diffuse(state[1] ^ read_u64_aligned(ptr + 8));
					state[2] = diffuse(state[2] ^ read_u64_aligned(ptr + 16));
					state[3] = diffuse(state[3] ^ read_u64_aligned(ptr + 24));
					written += 32;
					ptr += 32;
				}

				auto excess = bytes + n - ptr;
				switch (excess)
				{
					case 0: break;
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
					case 6:
					case 7:
					{
						tail = read_u64_buff(ptr, excess);
						tail_n = excess;
						break;
					}
					case 8:
					{
						push_u64(read_u64_aligned(ptr));
						break;
					}
					case 9:
					case 10:
					case 11:
					case 12:
					case 13:
					case 14:
					case 15:
					{
						push_u64(read_u64_aligned(ptr));
						tail = read_u64_buff(ptr + 8, excess -= 8);
						tail_n = excess;
						break;
					}
					case 16:
					{
						const auto a = diffuse(state[0] ^ read_u64_aligned(ptr));
						const auto b = diffuse(state[1] ^ read_u64_aligned(ptr + 8));

						state[0] = state[2];
						state[1] = state[3];
						state[2] = a;
						state[3] = b;
						written += 16;
						break;
					}
					case 17:
					case 18:
					case 19:
					case 20:
					case 21:
					case 22:
					case 23:
					{
						const auto a = diffuse(state[0] ^ read_u64_aligned(ptr));
						const auto b = diffuse(state[1] ^ read_u64_aligned(ptr + 8));

						state[0] = state[2];
						state[1] = state[3];
						state[2] = a;
						state[3] = b;
						tail = read_u64_buff(ptr + 16, excess -= 16);
						tail_n = excess;
						written += 16;
						break;
					}
					case 24:
					{
						const auto a = diffuse(state[0] ^ read_u64_aligned(ptr));
						const auto b = diffuse(state[1] ^ read_u64_aligned(ptr + 8));
						const auto c = diffuse(state[2] ^ read_u64_aligned(ptr + 16));

						state[0] = state[3];
						state[1] = a;
						state[2] = b;
						state[3] = c;
						written += 24;
						break;
					}
					default:
					{
						const auto a = diffuse(state[0] ^ read_u64_aligned(ptr));
						const auto b = diffuse(state[1] ^ read_u64_aligned(ptr + 8));
						const auto c = diffuse(state[2] ^ read_u64_aligned(ptr + 16));
						state[0] = state[3];
						state[1] = a;
						state[2] = b;
						state[3] = c;
						tail = read_u64_buff(ptr + 24, excess -= 24);
						tail_n = excess;
						written += 24;
					}
				}
			}
		}
		constexpr TPP_FORCEINLINE void push_u64(std::uint64_t x) noexcept
		{
			x = diffuse(state[0] ^ x);
			state[0] = state[1];
			state[1] = state[2];
			state[2] = state[3];
			state[3] = x;
			written += 8;
		}

		/* Initial state seeds from the reference implementation at `https://docs.rs/seahash/latest/src/seahash/stream.rs.html#19`. */
		std::uint64_t state[4] = {0x16f11fe89b0d677c, 0xb480a793d8e6c86c, 0x6fe2e5aaf078ebc9, 0x14f994a4c5259381};
		std::uint64_t tail = 0;
		std::size_t tail_n = 0;
		std::size_t written = 0;
	};

	/** @brief SeaHash byte hash function.
	 * @param[in] data Pointer to bytes of input data.
	 * @param[in] n Size of the \p data buffer. */
	[[nodiscard]] constexpr std::size_t seahash(const void *data, std::size_t n) noexcept
	{
		return static_cast<std::size_t>(seahash_builder{}.write(data, n).finish());
	}
	/** @copydoc seahash
	 * @param[in] seed Seed used for the hash algorithm. */
	[[nodiscard]] constexpr std::size_t seahash(const void *data, std::size_t n, const std::uint64_t (&seed)[4]) noexcept
	{
		return static_cast<std::size_t>(seahash_builder{seed}.write(data, n).finish());
	}

	/** @brief Generic hash functor used to hash a value of type `T` using the specified byte hash algorithm. */
	template<typename T, std::size_t (*Algo)(const void *, std::size_t)>
	struct hash;
	/** @brief Hasher that uses the SeaHash algorithm. */
	template<typename T>
	using seahash_hash = hash<T, seahash>;
	/** @brief Hasher that uses the FNV1a algorithm. */
	template<typename T>
	using fnv1a_hash = hash<T, fnv1a>;
	/** @brief Hasher that uses the SDBM algorithm. */
	template<typename T>
	using sdbm_hash = hash<T, sdbm>;
}

#define TPP_TRIVIAL_HASH_IMPL(T)                                                \
    template<std::size_t (*Algo)(const void *, std::size_t)>                    \
    struct tpp::hash<T, Algo>                                                   \
    {                                                                           \
        [[nodiscard]] constexpr std::size_t operator()(T value) const noexcept  \
        {                                                                       \
            return static_cast<std::size_t>(value);                             \
        }                                                                       \
    };

/* Trivial types are value-casted to std::size_t (already unique). */

TPP_TRIVIAL_HASH_IMPL(bool)

TPP_TRIVIAL_HASH_IMPL(char)

TPP_TRIVIAL_HASH_IMPL(signed char)

TPP_TRIVIAL_HASH_IMPL(unsigned char)

#if defined(__cpp_char8_t) && __cpp_char8_t >= 201811L

TPP_TRIVIAL_HASH_IMPL(char8_t)

#endif

TPP_TRIVIAL_HASH_IMPL(char16_t)

TPP_TRIVIAL_HASH_IMPL(char32_t)

TPP_TRIVIAL_HASH_IMPL(wchar_t)

TPP_TRIVIAL_HASH_IMPL(short)

TPP_TRIVIAL_HASH_IMPL(unsigned short)

TPP_TRIVIAL_HASH_IMPL(int)

TPP_TRIVIAL_HASH_IMPL(unsigned int)

TPP_TRIVIAL_HASH_IMPL(long)

TPP_TRIVIAL_HASH_IMPL(long long)

TPP_TRIVIAL_HASH_IMPL(unsigned long)

TPP_TRIVIAL_HASH_IMPL(unsigned long long)

#undef TPP_TRIVIAL_HASH_IMPL

/* Floating-point numbers are byte-hashed. */

#define TPP_FLOAT_HASH_IMPL(T)                                                  \
    template<std::size_t (*Algo)(const void *, std::size_t)>                    \
    struct tpp::hash<T, Algo>                                                   \
    {                                                                           \
        [[nodiscard]] constexpr std::size_t operator()(T value) const noexcept  \
        {                                                                       \
            if (value != static_cast<T>(0.0)) /* -0.0 must be equal to 0.0 */   \
                return Algo(&value, sizeof(T));                                 \
            else                                                                \
                return 0;                                                       \
        }                                                                       \
    };

TPP_FLOAT_HASH_IMPL(float);

TPP_FLOAT_HASH_IMPL(double);

TPP_FLOAT_HASH_IMPL(long double);

#undef TPP_FLOAT_HASH_IMPL

/* Pointers are value-casted to std::size_t (already unique). */
template<typename T, std::size_t (*Algo)(const void *, std::size_t)>
struct tpp::hash<T *, Algo>
{
	[[nodiscard]] std::size_t operator()(T *value) const noexcept
	{
		return reinterpret_cast<std::size_t>(value);
	}
};

template<std::size_t (*Algo)(const void *, std::size_t)>
struct tpp::hash<std::nullptr_t, Algo>
{
	[[nodiscard]] constexpr std::size_t operator()(std::nullptr_t) const noexcept { return 0; }
};

#endif