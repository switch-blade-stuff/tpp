/*
 * Created by switchblade on 11/7/22.
 */

#include "tests.hpp"

#include <random>
#include <array>

#ifndef TPP_STL_HASH_ALL
#define TPP_STL_HASH_ALL
#endif

#include <tables-pp/hash.hpp>
#include <tables-pp/stl_hash.hpp>

using namespace std::literals;

template<std::size_t (*A)(const void *, std::size_t)>
static auto test_hash()
{
	{
		using value_t = std::variant<bool, char, int, long, long long, float, double, long double, void *,
		                             std::string, std::string_view, std::wstring, std::wstring_view,
		                             std::filesystem::path, std::thread::id>;

		const std::array<value_t, 15> a = {
				value_t{false},
				value_t{'\0'},
				value_t{0},
				value_t{0l},
				value_t{0ll},
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
				value_t{0xaabb'ccddl},
				value_t{0xaabb'ccddll},
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
			assert(hash(a[i]) != hash(b[i]));
			assert(hash(a[i]) == hash(c[i]));
			assert(hash(b[i]) != hash(c[i]));
			assert(hash(b[i]) == hash(d[i]));
		}
	}
	{
		const tpp::hash<std::monostate, A> hash = {};
		assert(hash(std::monostate{}) == hash(std::monostate{}));
	}
	{
		const tpp::hash<std::nullptr_t, A> hash = {};
		assert(hash(nullptr) == hash(nullptr));
	}
	{
		const tpp::hash<std::string_view, A> hash = {};
		for (std::size_t i = 0; i < 1024; ++i)
		{
			const auto a = std::to_string(rand());
			const auto b = std::to_string(rand());
			const auto c = std::string_view{a};
			const auto d = std::string_view{b};

			assert(hash(a) != hash(b));
			assert(hash(a) == hash(c));
			assert(hash(b) != hash(c));
			assert(hash(b) == hash(d));
		}
	}
}

void test_seahash() noexcept { test_hash<tpp::seahash>(); }
void test_fnv1a() noexcept { test_hash<tpp::fnv1a>(); }
void test_sdbm() noexcept { test_hash<tpp::sdbm>(); }

void test_crc32() noexcept
{
	const auto a = "abcd"sv, b = "abcd"sv, c = "efgh"sv;
	assert(tpp::crc32(a.data(), a.size()) == tpp::crc32(b.data(), b.size()));
	assert(tpp::crc32(a.data(), a.size()) != tpp::crc32(c.data(), c.size()));
	assert(tpp::crc32(b.data(), b.size()) != tpp::crc32(c.data(), c.size()));
}
void test_md5() noexcept
{
	const auto a = "abcd"sv, b = "abcd"sv, c = "efgh"sv;

	std::uint8_t md5_a[16], md5_b[16], md5_c[16];
	tpp::md5(a.data(), a.size(), md5_a);
	tpp::md5(b.data(), b.size(), md5_b);
	tpp::md5(c.data(), c.size(), md5_c);

	assert(std::equal(md5_a, md5_a + 16, md5_b));
	assert(!std::equal(md5_a, md5_a + 16, md5_c));
	assert(!std::equal(md5_b, md5_b + 16, md5_c));
}