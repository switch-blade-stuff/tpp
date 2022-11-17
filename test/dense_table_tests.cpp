/*
 * Created by switchblade on 11/15/22.
 */

#include "tests.hpp"

#ifdef TPP_NO_HASH
#undef TPP_NO_HASH
#endif

#ifndef TPP_STL_HASH_ALL
#define TPP_STL_HASH_ALL
#endif

#include <tpp/dense_set.hpp>
#include <tpp/dense_map.hpp>

using namespace tpp;

template<typename T>
static void test_map(T &&map) noexcept
{
	TEST_ASSERT(map.size() == 0);

	TEST_ASSERT(map.emplace("0", 0).second);
	TEST_ASSERT(map.size() == 1);
	TEST_ASSERT(map.contains("0"));
	TEST_ASSERT(map.find("0")->second == 0);

	TEST_ASSERT(map.emplace("1", 1).second);
	TEST_ASSERT(map.size() == 2);
	TEST_ASSERT(map.contains("1"));
	TEST_ASSERT(map.find("1")->second == 1);
	TEST_ASSERT(map.find("1") != map.find("0"));

	TEST_ASSERT(!map.emplace("0", 1).second);
	TEST_ASSERT(map.size() == 2);
	TEST_ASSERT(map.contains("0"));
	TEST_ASSERT(map.find("0")->second == 0);

	TEST_ASSERT(!map.emplace_or_replace("0", 1).second);
	TEST_ASSERT(map.size() == 2);
	TEST_ASSERT(map.contains("0"));
	TEST_ASSERT(map.find("0")->second == 1);

	TEST_ASSERT(!map.insert_or_assign("0", 2).second);
	TEST_ASSERT(map.size() == 2);
	TEST_ASSERT(map.contains("0"));
	TEST_ASSERT(map.find("0")->second == 2);

	map.erase(map.find("0"));
	TEST_ASSERT(map.size() == 1);
	TEST_ASSERT(!map.contains("0"));

	TEST_ASSERT(map.insert_or_assign("0", 0).second);
	TEST_ASSERT(map.size() == 2);
	TEST_ASSERT(map.contains("0"));
	TEST_ASSERT(map.find("0")->second == 0);

	auto map2 = std::move(map);
	TEST_ASSERT(map.size() == 0);
	TEST_ASSERT(map2.size() == 2);
	TEST_ASSERT(map.begin() == map.end());
	TEST_ASSERT(map2.begin() != map2.end());

	map2.clear();
	TEST_ASSERT(map2.size() == 0);
	TEST_ASSERT(!map2.contains("0"));
	TEST_ASSERT(!map2.contains("1"));
	TEST_ASSERT(map2.find("0") == map2.end());
	TEST_ASSERT(map2.find("1") == map2.end());
	TEST_ASSERT(map2.begin() == map2.end());

	const int n = 0x10000;
	for (int i = 0; i < n; ++i)
	{
		const auto key = std::to_string(i);
		const auto result = map2.emplace(key, i);

		TEST_ASSERT(result.second);
		TEST_ASSERT(map2.contains(key));
		TEST_ASSERT(map2.find(key) != map2.end());
		TEST_ASSERT(map2.find(key) == result.first);
		TEST_ASSERT(map2.find(key)->first == key);
		TEST_ASSERT(map2.find(key)->second == i);
	}
	TEST_ASSERT(map2.size() == static_cast<std::size_t>(n));

	for (int i = 0; i < n; ++i)
	{
		const auto key = std::to_string(i);
		TEST_ASSERT(map2.contains(key));
		TEST_ASSERT(map2.find(key) != map2.end());
		TEST_ASSERT(map2.find(key)->first == key);
		TEST_ASSERT(map2.find(key)->second == i);

		map2.erase(key);
		TEST_ASSERT(!map2.contains(key));
		TEST_ASSERT(map2.find(key) == map2.end());
	}
	TEST_ASSERT(map2.size() == 0);
	TEST_ASSERT(map2.begin() == map2.end());
}

void test_dense_map() noexcept
{
	test_map(dense_map<std::string, int>{});
}
void test_ordered_dense_map() noexcept
{
	ordered_dense_map<std::string, int> map;
	TEST_ASSERT(map.size() == 0);

	TEST_ASSERT(map.emplace("0", 0).second);
	TEST_ASSERT(map.size() == 1);
	TEST_ASSERT(map.contains("0"));
	TEST_ASSERT(map.find("0")->second == 0);
	TEST_ASSERT(map.find("0") == map.begin());
	TEST_ASSERT(*map.find("0") == map.front());

	TEST_ASSERT(map.emplace("1", 1).second);
	TEST_ASSERT(map.size() == 2);
	TEST_ASSERT(map.contains("1"));
	TEST_ASSERT(map.find("1")->second == 1);
	TEST_ASSERT(map.find("1") == std::prev(map.end()));
	TEST_ASSERT(*map.find("1") == map.back());
	TEST_ASSERT(map.find("1") != map.find("0"));

	TEST_ASSERT(!map.emplace("0", 1).second);
	TEST_ASSERT(map.size() == 2);
	TEST_ASSERT(map.contains("0"));
	TEST_ASSERT(map.find("0")->second == 0);
	TEST_ASSERT(map.find("0") == map.begin());
	TEST_ASSERT(*map.find("0") == map.front());
	TEST_ASSERT(map.find("1") != map.find("0"));

	auto map2 = std::move(map);
	TEST_ASSERT(map.size() == 0);
	TEST_ASSERT(map.begin() == map.end());
	TEST_ASSERT(map2.size() == 2);
	TEST_ASSERT(map2.begin() != map2.end());
	TEST_ASSERT(map2.contains("0"));
	TEST_ASSERT(map2.contains("1"));
	TEST_ASSERT(map2.find("0") == map2.begin());
	TEST_ASSERT(map2.find("1") == std::prev(map2.end()));
	TEST_ASSERT(*map2.find("0") == map2.front());
	TEST_ASSERT(*map2.find("1") == map2.back());

	/* Other tests are handled by `test_map`. */
	map2.clear();
	test_map(std::move(map2));
}

void test_dense_set() noexcept
{
	dense_set<std::string> set0 = {"0", "1", "2"};
	dense_set<std::string> set1 = {"2", "1", "0"};
	dense_set<std::string> set2 = {"1", "2", "0"};
	dense_set<std::string> set3 = {"2", "0", "1"};

	TEST_ASSERT(set0 == set1);
	TEST_ASSERT(set0 == set2);
	TEST_ASSERT(set0 == set3);
}
void test_ordered_dense_set() noexcept
{
	ordered_dense_set<std::string> set0 = {"0", "1", "2"};
	ordered_dense_set<std::string> set1 = {"2", "1", "0"};
	ordered_dense_set<std::string> set2 = {"1", "2", "0"};
	ordered_dense_set<std::string> set3 = {"2", "0", "1"};

	TEST_ASSERT(set0 == set1);
	TEST_ASSERT(set0 == set2);
	TEST_ASSERT(set0 == set3);

	TEST_ASSERT(set0.front() == "0");
	TEST_ASSERT(set1.front() == "2");
	TEST_ASSERT(set2.front() == "1");
	TEST_ASSERT(set3.front() == "2");
	TEST_ASSERT(set0.back() == "2");
	TEST_ASSERT(set1.back() == "0");
	TEST_ASSERT(set2.back() == "0");
	TEST_ASSERT(set3.back() == "1");
	TEST_ASSERT(set0.find("0") == set0.begin());
	TEST_ASSERT(set1.find("2") == set1.begin());
	TEST_ASSERT(set2.find("1") == set2.begin());
	TEST_ASSERT(set3.find("2") == set3.begin());
	TEST_ASSERT(set0.find("1") == std::next(set0.begin()));
	TEST_ASSERT(set1.find("1") == std::next(set1.begin()));
	TEST_ASSERT(set2.find("2") == std::next(set2.begin()));
	TEST_ASSERT(set3.find("0") == std::next(set3.begin()));
	TEST_ASSERT(set0.find("2") == std::prev(set0.end()));
	TEST_ASSERT(set1.find("0") == std::prev(set1.end()));
	TEST_ASSERT(set2.find("0") == std::prev(set2.end()));
	TEST_ASSERT(set3.find("1") == std::prev(set3.end()));
}