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
	using value_t = std::pair<std::string_view, int>;
	using table_t = tpp::detail::dense_table<value_t, const std::string_view, std::hash<std::string_view>, std::equal_to<>, tpp::detail::key_first>;

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
	assert(table.find("1") != table.find("0"));
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
}