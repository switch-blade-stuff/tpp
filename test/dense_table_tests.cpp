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

void test_dense_set() noexcept { test_set<tpp::dense_set>(); }
void test_dense_map() noexcept { test_map<tpp::dense_map>(); }

void test_ordered_dense_set() noexcept
{
	test_set<tpp::ordered_dense_set>();
	test_ordered_set<tpp::ordered_dense_set>();
}
void test_ordered_dense_map() noexcept
{
	test_map<tpp::ordered_dense_map>();
	test_ordered_map<tpp::ordered_dense_map>();
}

void test_dense_multiset() noexcept { test_multiset<tpp::dense_multiset>(); }
void test_dense_multimap() noexcept { test_multimap<tpp::dense_multimap>(); }
