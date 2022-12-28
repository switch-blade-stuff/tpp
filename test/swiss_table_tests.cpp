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

#include <tpp/sparse_set.hpp>
#include <tpp/sparse_map.hpp>
#include <tpp/stable_map.hpp>

void test_sparse_set() noexcept {}
void test_sparse_map() noexcept { test_map<tpp::sparse_map>(); }
void test_ordered_sparse_set() noexcept {}
void test_ordered_sparse_map() noexcept
{
	test_map<tpp::ordered_sparse_map>();
	test_ordered_map<tpp::ordered_sparse_map>();
}

void test_stable_set() noexcept {}
void test_stable_map() noexcept {}
void test_ordered_stable_set() noexcept {}
void test_ordered_stable_map() noexcept
{
    test_map<tpp::ordered_stable_map>();
    test_ordered_map<tpp::ordered_stable_map>();
}