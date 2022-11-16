/*
 * Created by switchblade on 11/15/22.
 */

#include "tests.hpp"

/* Use std::hash for testing. */
#ifndef TPP_NO_HASH
#define TPP_NO_HASH
#endif

#include <tpp/detail/dense_table.hpp>

void test_dense_table() noexcept
{
	using value_t = std::pair<std::string, int>;
	using table_t = tpp::detail::dense_table<value_t, const std::string, std::hash<std::string>, std::equal_to<>, tpp::detail::key_first>;

	table_t table;
	assert(table.size() == 0);

	assert(table.emplace("0", 0).second);
	assert(table.size() == 1);
	assert(table.contains("0"));
	assert(table.find("0")->second == 0);

	assert(table.emplace("1", 1).second);
	assert(table.size() == 2);
	assert(table.contains("1"));
	assert(table.find("1")->second == 1);
	assert(table.find("1") != table.find("0"));

	assert(!table.emplace("0", 1).second);
	assert(table.size() == 2);
	assert(table.contains("0"));
	assert(table.find("0")->second == 0);

	assert(!table.emplace_or_replace("0", 1).second);
	assert(table.size() == 2);
	assert(table.contains("0"));
	assert(table.find("0")->second == 1);

	assert(!table.insert_or_assign("0", 2).second);
	assert(table.size() == 2);
	assert(table.contains("0"));
	assert(table.find("0")->second == 2);

	table.erase(table.find("0"));
	assert(table.size() == 1);
	assert(!table.contains("0"));

	assert(table.insert_or_assign("0", 0).second);
	assert(table.size() == 2);
	assert(table.contains("0"));
	assert(table.find("0")->second == 0);

	auto table2 = std::move(table);
	assert(table.size() == 0);
	assert(table2.size() == 2);
	assert(table.begin() == table.end());
	assert(table2.begin() != table2.end());

	table2.clear();
	assert(table2.size() == 0);
	assert(!table2.contains("0"));
	assert(!table2.contains("1"));
	assert(table2.find("0") == table2.end());
	assert(table2.find("1") == table2.end());
	assert(table2.begin() == table2.end());

	const int n = 1000;
	for (int i = 0; i < n; ++i)
	{
		const auto key = std::to_string(i);
		const auto result = table2.emplace(key, i);

		assert(result.second);
		assert(table2.contains(key));
		assert(table2.find(key) != table2.end());
		assert(table2.find(key) == result.first);
		assert(table2.find(key)->first == key);
		assert(table2.find(key)->second == i);
	}
	assert(table2.size() == static_cast<std::size_t>(n));

	for (int i = 0; i < n; ++i)
	{
		const auto key = std::to_string(i);
		assert(table2.contains(key));
		assert(table2.find(key) != table2.end());
		assert(table2.find(key)->first == key);
		assert(table2.find(key)->second == i);

		table2.erase(key);
		assert(!table2.contains(key));
		assert(table2.find(key) == table2.end());
	}
	assert(table2.size() == 0);
	assert(table2.begin() == table2.end());
}

void test_ordered_dense_table() noexcept
{
	using value_t = std::pair<std::string_view, int>;
	using table_t = tpp::detail::dense_table<value_t,
	                                         const std::string_view,
	                                         std::hash<std::string_view>,
	                                         std::equal_to<>,
	                                         tpp::detail::key_first,
	                                         std::allocator<value_t>,
	                                         tpp::detail::ordered_link>;

	table_t table;
	assert(table.size() == 0);

	assert(table.emplace("0", 0).second);
	assert(table.size() == 1);
	assert(table.contains("0"));
	assert(table.find("0")->second == 0);
	assert(table.find("0") == table.begin());
	assert(*table.find("0") == table.front());

	assert(table.emplace("1", 1).second);
	assert(table.size() == 2);
	assert(table.contains("1"));
	assert(table.find("1")->second == 1);
	assert(table.find("1") == std::prev(table.end()));
	assert(*table.find("1") == table.back());
	assert(table.find("1") != table.find("0"));

	assert(!table.emplace("0", 1).second);
	assert(table.size() == 2);
	assert(table.contains("0"));
	assert(table.find("0")->second == 0);
	assert(table.find("0") == table.begin());
	assert(*table.find("0") == table.front());
	assert(table.find("1") != table.find("0"));

	auto table2 = std::move(table);
	assert(table.size() == 0);
	assert(table.begin() == table.end());
	assert(table2.size() == 2);
	assert(table2.begin() != table2.end());
	assert(table2.contains("0"));
	assert(table2.contains("1"));
	assert(table2.find("0") == table2.begin());
	assert(table2.find("1") == std::prev(table2.end()));
	assert(*table2.find("0") == table2.front());
	assert(*table2.find("1") == table2.back());

	/* NOTE: Other tests are handled by `test_dense_table`. */
}