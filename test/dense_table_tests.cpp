/*
 * Created by switchblade on 11/15/22.
 */

#include "tests.hpp"

#ifndef TPP_STL_HASH_ALL
#define TPP_STL_HASH_ALL
#endif

#include <tpp/detail/dense_table.hpp>
#include <tpp/dense_set.hpp>
#include <tpp/stl_hash.hpp>

template<typename T>
static void test_table(T &table) noexcept
{
	TEST_ASSERT(table.size() == 0);

	TEST_ASSERT(table.emplace("0", 0).second);
	TEST_ASSERT(table.size() == 1);
	TEST_ASSERT(table.contains("0"));
	TEST_ASSERT(table.find("0")->second == 0);

	TEST_ASSERT(table.emplace("1", 1).second);
	TEST_ASSERT(table.size() == 2);
	TEST_ASSERT(table.contains("1"));
	TEST_ASSERT(table.find("1")->second == 1);
	TEST_ASSERT(table.find("1") != table.find("0"));

	TEST_ASSERT(!table.emplace("0", 1).second);
	TEST_ASSERT(table.size() == 2);
	TEST_ASSERT(table.contains("0"));
	TEST_ASSERT(table.find("0")->second == 0);

	TEST_ASSERT(!table.emplace_or_replace("0", 1).second);
	TEST_ASSERT(table.size() == 2);
	TEST_ASSERT(table.contains("0"));
	TEST_ASSERT(table.find("0")->second == 1);

	TEST_ASSERT(!table.insert_or_assign("0", 2).second);
	TEST_ASSERT(table.size() == 2);
	TEST_ASSERT(table.contains("0"));
	TEST_ASSERT(table.find("0")->second == 2);

	table.erase(table.find("0"));
	TEST_ASSERT(table.size() == 1);
	TEST_ASSERT(!table.contains("0"));

	TEST_ASSERT(table.insert_or_assign("0", 0).second);
	TEST_ASSERT(table.size() == 2);
	TEST_ASSERT(table.contains("0"));
	TEST_ASSERT(table.find("0")->second == 0);

	auto table2 = std::move(table);
	TEST_ASSERT(table.size() == 0);
	TEST_ASSERT(table2.size() == 2);
	TEST_ASSERT(table.begin() == table.end());
	TEST_ASSERT(table2.begin() != table2.end());

	table2.clear();
	TEST_ASSERT(table2.size() == 0);
	TEST_ASSERT(!table2.contains("0"));
	TEST_ASSERT(!table2.contains("1"));
	TEST_ASSERT(table2.find("0") == table2.end());
	TEST_ASSERT(table2.find("1") == table2.end());
	TEST_ASSERT(table2.begin() == table2.end());

	const int n = 0x10000;
	for (int i = 0; i < n; ++i)
	{
		const auto key = std::to_string(i);
		const auto result = table2.emplace(key, i);

		TEST_ASSERT(result.second);
		TEST_ASSERT(table2.contains(key));
		TEST_ASSERT(table2.find(key) != table2.end());
		TEST_ASSERT(table2.find(key) == result.first);
		TEST_ASSERT(table2.find(key)->first == key);
		TEST_ASSERT(table2.find(key)->second == i);
	}
	TEST_ASSERT(table2.size() == static_cast<std::size_t>(n));

	for (int i = 0; i < n; ++i)
	{
		const auto key = std::to_string(i);
		TEST_ASSERT(table2.contains(key));
		TEST_ASSERT(table2.find(key) != table2.end());
		TEST_ASSERT(table2.find(key)->first == key);
		TEST_ASSERT(table2.find(key)->second == i);

		table2.erase(key);
		TEST_ASSERT(!table2.contains(key));
		TEST_ASSERT(table2.find(key) == table2.end());
	}
	TEST_ASSERT(table2.size() == 0);
	TEST_ASSERT(table2.begin() == table2.end());
}

void test_dense_table() noexcept
{
	using namespace tpp;
	using value_t = std::pair<std::string, int>;
	using equal_t = std::equal_to<std::string>;
	using hash_t = seahash_hash<std::string>;
	using alloc_t = std::allocator<value_t>;

	using table_t = detail::dense_table<value_t, const std::string, hash_t, equal_t, detail::key_first, alloc_t>;

	table_t table;
	test_table(table);

	dense_set<std::string> set0 = {"0", "1", "2"};
	dense_set<std::string> set1 = {"2", "1", "0"};
	dense_set<std::string> set2 = {"1", "2", "0"};
	dense_set<std::string> set3 = {"2", "0", "1"};

	TEST_ASSERT(set0 == set1);
	TEST_ASSERT(set0 == set2);
	TEST_ASSERT(set0 == set3);
}
void test_ordered_dense_table() noexcept
{
	using namespace tpp;
	using value_t = std::pair<std::string, int>;
	using equal_t = std::equal_to<std::string>;
	using hash_t = seahash_hash<std::string>;
	using alloc_t = std::allocator<value_t>;

	using table_t = detail::dense_table<value_t, const std::string, hash_t, equal_t, detail::key_first, alloc_t, detail::ordered_link>;

	table_t table;
	TEST_ASSERT(table.size() == 0);

	TEST_ASSERT(table.emplace("0", 0).second);
	TEST_ASSERT(table.size() == 1);
	TEST_ASSERT(table.contains("0"));
	TEST_ASSERT(table.find("0")->second == 0);
	TEST_ASSERT(table.find("0") == table.begin());
	TEST_ASSERT(*table.find("0") == table.front());

	TEST_ASSERT(table.emplace("1", 1).second);
	TEST_ASSERT(table.size() == 2);
	TEST_ASSERT(table.contains("1"));
	TEST_ASSERT(table.find("1")->second == 1);
	TEST_ASSERT(table.find("1") == std::prev(table.end()));
	TEST_ASSERT(*table.find("1") == table.back());
	TEST_ASSERT(table.find("1") != table.find("0"));

	TEST_ASSERT(!table.emplace("0", 1).second);
	TEST_ASSERT(table.size() == 2);
	TEST_ASSERT(table.contains("0"));
	TEST_ASSERT(table.find("0")->second == 0);
	TEST_ASSERT(table.find("0") == table.begin());
	TEST_ASSERT(*table.find("0") == table.front());
	TEST_ASSERT(table.find("1") != table.find("0"));

	auto table2 = std::move(table);
	TEST_ASSERT(table.size() == 0);
	TEST_ASSERT(table.begin() == table.end());
	TEST_ASSERT(table2.size() == 2);
	TEST_ASSERT(table2.begin() != table2.end());
	TEST_ASSERT(table2.contains("0"));
	TEST_ASSERT(table2.contains("1"));
	TEST_ASSERT(table2.find("0") == table2.begin());
	TEST_ASSERT(table2.find("1") == std::prev(table2.end()));
	TEST_ASSERT(*table2.find("0") == table2.front());
	TEST_ASSERT(*table2.find("1") == table2.back());

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

	/* Other tests are handled by `test_table`. */
	table2.clear();
	test_table(table2);
}