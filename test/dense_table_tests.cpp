/*
 * Created by switchblade on 11/15/22.
 */

#include "tests.hpp"
#include "map_tests.hpp"
#include "set_tests.hpp"

#ifdef TPP_NO_HASH
#undef TPP_NO_HASH
#endif

#ifndef TPP_STL_HASH_ALL
#define TPP_STL_HASH_ALL
#endif

#include <tpp/dense_set.hpp>
#include <tpp/dense_map.hpp>
#include <tpp/dense_multiset.hpp>
#include <tpp/dense_multimap.hpp>

void test_dense_map() noexcept
{
	test_map<tpp::dense_map>();
}
void test_ordered_dense_map() noexcept
{
	test_map<tpp::ordered_dense_map>();
	test_ordered_map<tpp::ordered_dense_map>();
}

void test_dense_set() noexcept
{
	test_set<tpp::dense_set>();
}
void test_ordered_dense_set() noexcept
{
	test_set<tpp::ordered_dense_set>();
	test_ordered_set<tpp::ordered_dense_set>();
}

void test_dense_multimap() noexcept
{
	test_multimap<tpp::dense_multimap>();
}
void test_dense_multiset() noexcept
{
	tpp::dense_multiset<tpp::multikey<int, std::string>> mset;

	TEST_ASSERT(mset.emplace(0, "a").second);
	TEST_ASSERT(mset.emplace(1, "b").second);
	TEST_ASSERT(!mset.emplace(0, "b").second);
	TEST_ASSERT(!mset.emplace(0, "c").second);
	TEST_ASSERT(!mset.emplace(1, "a").second);
	TEST_ASSERT(!mset.emplace(1, "c").second);

	TEST_ASSERT(mset.contains<0>(0));
	TEST_ASSERT(mset.contains<0>(1));
	TEST_ASSERT(!mset.contains<0>(2));
	TEST_ASSERT(mset.contains<1>("a"));
	TEST_ASSERT(mset.contains<1>("b"));
	TEST_ASSERT(!mset.contains<1>("c"));

	TEST_ASSERT(mset.find<0>(0) == mset.find<1>("a"));
	TEST_ASSERT(mset.find<0>(1) == mset.find<1>("b"));

	mset.erase<0>(0);
	TEST_ASSERT(!mset.contains<0>(0));
	TEST_ASSERT(!mset.contains<1>("a"));

	mset.erase<1>("b");
	TEST_ASSERT(!mset.contains<0>(1));
	TEST_ASSERT(!mset.contains<1>("b"));

	mset = {{0, "0"}, {1, "1"}};

	TEST_ASSERT(mset.contains<0>(0));
	TEST_ASSERT(mset.contains<0>(1));
	TEST_ASSERT(!mset.contains<0>(2));
	TEST_ASSERT(mset.contains<1>("0"));
	TEST_ASSERT(mset.contains<1>("1"));
	TEST_ASSERT(!mset.contains<1>("2"));

	TEST_ASSERT(mset.find<0>(0) == mset.find<1>("0"));
	TEST_ASSERT(mset.find<0>(1) == mset.find<1>("1"));

	mset.clear();
	TEST_ASSERT(mset.empty());
	TEST_ASSERT(mset.find<0>(0) == mset.end());
	TEST_ASSERT(mset.find<0>(1) == mset.end());
	TEST_ASSERT(mset.find<1>("0") == mset.end());
	TEST_ASSERT(mset.find<1>("1") == mset.end());

	const int n = 0x10000;
	for (int i = 0; i < n; ++i)
	{
		const auto str = std::to_string(i);
		const auto result = mset.emplace(i, str);

		TEST_ASSERT(result.second);
		TEST_ASSERT(result.first == mset.find<0>(i));
		TEST_ASSERT(result.first == mset.find<1>(str));
	}

	TEST_ASSERT(mset.size() == n);

	for (int i = 0; i < n; ++i)
	{
		const auto str = std::to_string(i);
		TEST_ASSERT(mset.contains<0>(i));
		TEST_ASSERT(mset.contains<1>(str));
		TEST_ASSERT(mset.find<0>(i) == mset.find<1>(str));
	}
}
