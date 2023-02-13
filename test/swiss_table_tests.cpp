/*
 * Created by switchblade on 2022-11-15.
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

static_assert(std::is_same_v<decltype(tpp::sparse_set{std::declval<std::string>()}), tpp::sparse_set<std::string>>);
static_assert(std::is_same_v<decltype(tpp::ordered_sparse_set{std::declval<std::string>()}), tpp::ordered_sparse_set<std::string>>);
static_assert(std::is_same_v<decltype(tpp::sparse_map{std::declval<std::pair<std::string, int>>()}), tpp::sparse_map<std::string, int>>);
static_assert(std::is_same_v<decltype(tpp::ordered_sparse_map{std::declval<std::pair<std::string, int>>()}), tpp::ordered_sparse_map<std::string, int>>);

void test_sparse_set() noexcept { test_set<tpp::sparse_set>(); }
void test_sparse_map() noexcept { test_map<tpp::sparse_map>(); }
void test_ordered_sparse_set() noexcept
{
	test_set<tpp::ordered_sparse_set>();
	test_ordered_set<tpp::ordered_sparse_set>();
}
void test_ordered_sparse_map() noexcept
{
	test_map<tpp::ordered_sparse_map>();
	test_ordered_map<tpp::ordered_sparse_map>();
}

#include <tpp/stable_set.hpp>
#include <tpp/stable_map.hpp>

static_assert(std::is_same_v<decltype(tpp::stable_set{std::declval<std::string>()}), tpp::stable_set<std::string>>);
static_assert(std::is_same_v<decltype(tpp::ordered_stable_set{std::declval<std::string>()}), tpp::ordered_stable_set<std::string>>);
static_assert(std::is_same_v<decltype(tpp::stable_map{std::declval<std::pair<std::string, int>>()}), tpp::stable_map<std::string, int>>);
static_assert(std::is_same_v<decltype(tpp::ordered_stable_map{std::declval<std::pair<std::string, int>>()}), tpp::ordered_stable_map<std::string, int>>);

void test_stable_set() noexcept
{
	test_set<tpp::stable_set>();
	test_node_set<tpp::stable_set>();
}
void test_stable_map() noexcept
{
	test_map<tpp::stable_map>();
	test_node_map<tpp::stable_map>();
}
void test_ordered_stable_set() noexcept
{
	test_set<tpp::ordered_stable_set>();
	test_ordered_set<tpp::ordered_stable_set>();
	test_node_set<tpp::ordered_stable_set>();
}
void test_ordered_stable_map() noexcept
{
	test_map<tpp::ordered_stable_map>();
	test_ordered_map<tpp::ordered_stable_map>();
	test_node_map<tpp::ordered_stable_map>();
}