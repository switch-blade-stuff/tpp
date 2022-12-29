/*
 * Created by switchblade on 2022-11-7.
 */

#include "tests.hpp"

#ifndef TPP_STL_HASH_ALL
#define TPP_STL_HASH_ALL
#endif

#include <tpp/stl_hash.hpp>

#ifndef TPP_USE_MODULES

#include <random>
#include <array>

#endif

using namespace std::literals;

template<std::size_t (*A)(const void *, std::size_t)>
static auto test_hash()
{
	{
		using value_t = std::variant<bool, char, int, unsigned long, unsigned long long, float, double, long double, void *,
		                             std::string, std::string_view, std::wstring, std::wstring_view,
		                             std::filesystem::path, std::thread::id>;

		const std::array<value_t, 15> a = {
				value_t{false},
				value_t{'\0'},
				value_t{0},
				value_t{0ul},
				value_t{0ull},
				value_t{0.0f},
				value_t{0.0},
				value_t{0.0L},
				value_t{(void *) nullptr},
				value_t{std::string{}},
				value_t{std::string_view{}},
				value_t{std::wstring{}},
				value_t{std::wstring_view{}},
				value_t{std::filesystem::path{}},
				value_t{std::thread::id{}}
		};
		const std::array<value_t, 15> b = {
				value_t{true},
				value_t{'A'},
				value_t{0xaabb},
				value_t{0xaabb'ccddul},
				value_t{0xaabb'ccddull},
				value_t{13.34f},
				value_t{13.34},
				value_t{13.34L},
				value_t{(void *) 0xffff'ffff},
				value_t{std::string{"abcd"}},
				value_t{std::string_view{"abcd"}},
				value_t{std::wstring{L"abcd"}},
				value_t{std::wstring_view{L"abcd"}},
				value_t{std::filesystem::current_path()},
				value_t{std::this_thread::get_id()},
		};
		const auto c = a;
		const auto d = b;

		const tpp::hash<value_t, A> hash = {};
		for (std::size_t i = 0; i < 15; ++i)
		{
			const std::size_t h[] = {hash(a[i]), hash(b[i]), hash(c[i]), hash(d[i])};

			TEST_ASSERT(h[0] == h[2]);
			TEST_ASSERT(h[1] == h[3]);
			TEST_ASSERT((h[0] == h[1]) == (h[2] == h[3]));
		}
	}
	{
		const tpp::hash<std::monostate, A> hash = {};
		TEST_ASSERT(hash(std::monostate{}) == hash(std::monostate{}));
	}
	{
		const tpp::hash<std::nullptr_t, A> hash = {};
		TEST_ASSERT(hash(nullptr) == hash(nullptr));
	}
	{
		const tpp::hash<std::string_view, A> hash = {};
		for (std::size_t i = 0; i < 0x10000; ++i)
		{
			const auto a = std::to_string(i);
			const auto b = std::to_string(i * 2 + 1);
			const auto c = std::string_view{a};
			const auto d = std::string_view{b};

			const std::size_t h[] = {hash(a), hash(b), hash(c), hash(d)};
			TEST_ASSERT(h[0] == h[2]);
			TEST_ASSERT(h[1] == h[3]);
			TEST_ASSERT(h[0] == h[1] ? h[2] == h[3] : h[2] != h[3]);
		}
	}
}

void test_seahash() noexcept { test_hash<tpp::seahash>(); }
void test_fnv1a() noexcept { test_hash<tpp::fnv1a>(); }
void test_sdbm() noexcept { test_hash<tpp::sdbm>(); }

void test_crc32() noexcept
{
	const auto a = "abcd"sv, b = "abcd"sv, c = "efgh"sv;
	TEST_ASSERT(tpp::crc32(a.data(), a.size()) == tpp::crc32(b.data(), b.size()));
	TEST_ASSERT(tpp::crc32(a.data(), a.size()) != tpp::crc32(c.data(), c.size()));
	TEST_ASSERT(tpp::crc32(b.data(), b.size()) != tpp::crc32(c.data(), c.size()));
}
void test_md5() noexcept
{
	const auto a = "abcd"sv, b = "abcd"sv, c = "efgh"sv;

	std::uint8_t md5_a[16], md5_b[16], md5_c[16];
	tpp::md5(a.data(), a.size(), md5_a);
	tpp::md5(b.data(), b.size(), md5_b);
	tpp::md5(c.data(), c.size(), md5_c);

	TEST_ASSERT(std::equal(md5_a, md5_a + 16, md5_b));
	TEST_ASSERT(!std::equal(md5_a, md5_a + 16, md5_c));
	TEST_ASSERT(!std::equal(md5_b, md5_b + 16, md5_c));
}