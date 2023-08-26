/*
 * Created by switch_blade on 2022-12-15.
 */

#pragma once

#include "assert.hpp"

#include <tpp/detail/multikey.hpp>

template<template<typename...> typename T, typename set_t = T<std::string>>
static void test_set() noexcept
{
	set_t set0 = {"0", "1", "2"};
	set_t set1 = {"2", "1", "0"};
	set_t set2 = {"1", "2", "0"};
	set_t set3 = {"2", "0", "1"};

	TEST_ASSERT(set0.size() == 3);
	TEST_ASSERT(set0.contains("0"));
	TEST_ASSERT(set0.contains("1"));
	TEST_ASSERT(set0.contains("2"));

	TEST_ASSERT(set0 == set1);
	TEST_ASSERT(set0 == set2);
	TEST_ASSERT(set0 == set3);

	set1 = set_t{std::move(set0)};

	TEST_ASSERT(set0.size() == 0);
	TEST_ASSERT(!set0.contains("0"));
	TEST_ASSERT(!set0.contains("1"));
	TEST_ASSERT(!set0.contains("2"));
	TEST_ASSERT(set0.begin() == set0.end());

	TEST_ASSERT(set1.size() == 3);
	TEST_ASSERT(set1.contains("0"));
	TEST_ASSERT(set1.contains("1"));
	TEST_ASSERT(set1.contains("2"));
	TEST_ASSERT(set1.begin() != set1.end());

	set2 = set1;

	TEST_ASSERT(set2.size() == 3);
	TEST_ASSERT(set2.contains("0"));
	TEST_ASSERT(set2.contains("1"));
	TEST_ASSERT(set2.contains("2"));
	TEST_ASSERT(set2.begin() != set2.end());

	TEST_ASSERT(set2 != set0);
	TEST_ASSERT(set2 == set1);

	set1.clear();
	TEST_ASSERT(set1.size() == 0);
	TEST_ASSERT(!set1.contains("0"));
	TEST_ASSERT(!set1.contains("1"));
	TEST_ASSERT(!set1.contains("2"));
	TEST_ASSERT(set1.find("0") == set1.end());
	TEST_ASSERT(set1.find("1") == set1.end());
	TEST_ASSERT(set1.find("2") == set1.end());
	TEST_ASSERT(set1.begin() == set1.end());

	const int n = 0x1000;
	for (int i = 0; i < n; ++i)
	{
		const auto key = std::to_string(i);
		const auto result = set1.emplace(key);

		TEST_ASSERT(result.second);
		TEST_ASSERT(set1.contains(key));
		TEST_ASSERT(set1.find(key) != set1.end());
		TEST_ASSERT(set1.find(key) == result.first);
		TEST_ASSERT(*set1.find(key) == key);
	}
	TEST_ASSERT(set1.size() == static_cast<std::size_t>(n));

	for (int i = 0; i < n; ++i)
	{
		const auto key = std::to_string(i);
		TEST_ASSERT(set1.contains(key));
		TEST_ASSERT(set1.find(key) != set1.end());
		TEST_ASSERT(*set1.find(key) == key);

		set1.erase(key);
		TEST_ASSERT(!set1.contains(key));
		TEST_ASSERT(set1.find(key) == set1.end());
	}
	TEST_ASSERT(set1.size() == 0);
	TEST_ASSERT(set1.begin() == set1.end());
}

template<template<typename...> typename T, typename set_t = T<std::string>>
static void test_ordered_set() noexcept
{
	auto set0 = set_t{};
	TEST_ASSERT(set0.size() == 0);

	TEST_ASSERT(set0.emplace("0").second);
	TEST_ASSERT(set0.size() == 1);
	TEST_ASSERT(set0.contains("0"));
	TEST_ASSERT(set0.find("0") == set0.begin());
	TEST_ASSERT(*set0.find("0") == set0.front());

	TEST_ASSERT(set0.emplace("1").second);
	TEST_ASSERT(set0.size() == 2);
	TEST_ASSERT(set0.contains("1"));
	TEST_ASSERT(set0.find("1") == std::prev(set0.end()));
	TEST_ASSERT(*set0.find("1") == set0.back());
	TEST_ASSERT(set0.find("1") != set0.find("0"));

	TEST_ASSERT(!set0.emplace("0", 1).second);
	TEST_ASSERT(set0.size() == 2);
	TEST_ASSERT(set0.contains("0"));
	TEST_ASSERT(set0.find("0") == set0.begin());
	TEST_ASSERT(*set0.find("0") == set0.front());
	TEST_ASSERT(set0.find("1") != set0.find("0"));

	auto set1 = set_t{std::move(set0)};

	TEST_ASSERT(set0.size() == 0);
	TEST_ASSERT(set0.begin() == set0.end());
	TEST_ASSERT(!set0.contains("0"));
	TEST_ASSERT(!set0.contains("1"));

	TEST_ASSERT(set1.size() == 2);
	TEST_ASSERT(set1.contains("0"));
	TEST_ASSERT(set1.contains("1"));
	TEST_ASSERT(set1.begin() != set1.end());
	TEST_ASSERT(set1.find("0") == set1.begin());
	TEST_ASSERT(set1.find("1") == std::prev(set1.end()));
	TEST_ASSERT(*set1.find("0") == set1.front());
	TEST_ASSERT(*set1.find("1") == set1.back());

	auto set2 = set1;

	TEST_ASSERT(set2.size() == 2);
	TEST_ASSERT(set2.contains("0"));
	TEST_ASSERT(set2.contains("1"));
	TEST_ASSERT(set2.begin() != set2.end());
	TEST_ASSERT(set2.find("0") == set2.begin());
	TEST_ASSERT(set2.find("1") == std::prev(set2.end()));
	TEST_ASSERT(*set2.find("0") == set2.front());
	TEST_ASSERT(*set2.find("1") == set2.back());

	TEST_ASSERT(set2 != set0);
	TEST_ASSERT(set2 == set1);
}

template<template<typename...> typename T, typename set_t = T<tpp::multikey<std::string, int>>>
static void test_multiset() noexcept
{
	auto set0 = set_t{};

	TEST_ASSERT(set0.emplace("a", 0).second);
	TEST_ASSERT(set0.emplace("b", 1).second);
	TEST_ASSERT(!set0.emplace("b", 0).second);
	TEST_ASSERT(!set0.emplace("c", 0).second);
	TEST_ASSERT(!set0.emplace("a", 1).second);
	TEST_ASSERT(!set0.emplace("c", 1).second);

	TEST_ASSERT(set0.template contains<0>("a"));
	TEST_ASSERT(set0.template contains<0>("b"));
	TEST_ASSERT(!set0.template contains<0>("c"));
	TEST_ASSERT(set0.template contains<1>(0));
	TEST_ASSERT(set0.template contains<1>(1));
	TEST_ASSERT(!set0.template contains<1>(2));

	TEST_ASSERT(set0.template find<0>("a") == set0.template find<1>(0));
	TEST_ASSERT(set0.template find<0>("b") == set0.template find<1>(1));

	set0.template erase<0>("a");
	TEST_ASSERT(!set0.template contains<0>("a"));
	TEST_ASSERT(!set0.template contains<1>(0));

	set0.template erase<1>(1);
	TEST_ASSERT(!set0.template contains<0>("b"));
	TEST_ASSERT(!set0.template contains<1>(1));

	set0 = {{"0", 0}, {"1", 1}};

	TEST_ASSERT(set0.template contains<0>("0"));
	TEST_ASSERT(set0.template contains<0>("1"));
	TEST_ASSERT(!set0.template contains<0>("2"));
	TEST_ASSERT(set0.template contains<1>(0));
	TEST_ASSERT(set0.template contains<1>(1));
	TEST_ASSERT(!set0.template contains<1>(2));

	TEST_ASSERT(set0.template find<0>("0") == set0.template find<1>(0));
	TEST_ASSERT(set0.template find<0>("1") == set0.template find<1>(1));

	auto set1 = set_t{std::move(set0)};

	TEST_ASSERT(set0.size() == 0);
	TEST_ASSERT(set0.begin() == set0.end());
	TEST_ASSERT(!set0.template contains<0>("0"));
	TEST_ASSERT(!set0.template contains<0>("1"));
	TEST_ASSERT(!set0.template contains<1>(0));
	TEST_ASSERT(!set0.template contains<1>(1));

	TEST_ASSERT(set1.size() == 2);
	TEST_ASSERT(set1.begin() != set1.end());
	TEST_ASSERT(set1.template contains<0>("0"));
	TEST_ASSERT(set1.template contains<0>("1"));
	TEST_ASSERT(set1.template contains<1>(0));
	TEST_ASSERT(set1.template contains<1>(1));

	auto set2 = set1;

	TEST_ASSERT(set2.size() == 2);
	TEST_ASSERT(set2.template contains<0>("0"));
	TEST_ASSERT(set2.template contains<0>("1"));
	TEST_ASSERT(set2.template contains<1>(0));
	TEST_ASSERT(set2.template contains<1>(1));
	TEST_ASSERT(set2.begin() != set2.end());

	TEST_ASSERT(set2 != set0);
	TEST_ASSERT(set2 == set1);

	set1.clear();
	TEST_ASSERT(set1.empty());
	TEST_ASSERT(set1.template find<0>("0") == set1.end());
	TEST_ASSERT(set1.template find<0>("1") == set1.end());
	TEST_ASSERT(set1.template find<1>(0) == set1.end());
	TEST_ASSERT(set1.template find<1>(1) == set1.end());

	const int n = 0x1000;
	for (int i = 0; i < n; ++i)
	{
		const auto str = std::to_string(i);
		const auto result = set1.emplace(str, i);

		TEST_ASSERT(result.second);
		TEST_ASSERT(result.first == set1.template find<1>(i));
		TEST_ASSERT(result.first == set1.template find<0>(str));
	}
	TEST_ASSERT(set1.size() == n);

	for (int i = 0; i < n; ++i)
	{
		const auto str = std::to_string(i);
		TEST_ASSERT(set1.template contains<0>(str));
		TEST_ASSERT(set1.template contains<1>(i));
		TEST_ASSERT(set1.template find<0>(str) == set1.template find<1>(i));
	}
}

template<template<typename...> typename T, typename set_t = T<std::string>>
static void test_node_set() noexcept
{
	auto set0 = set_t{};

	TEST_ASSERT(set0.size() == 0);
	TEST_ASSERT(set0.emplace("0").second);
	TEST_ASSERT(set0.emplace("1").second);
	TEST_ASSERT(set0.contains("0"));
	TEST_ASSERT(set0.contains("1"));
	TEST_ASSERT(set0.size() == 2);

	auto set1 = set_t{};

	TEST_ASSERT(set1.size() == 0);
	TEST_ASSERT(set1.emplace("0").second);
	TEST_ASSERT(set1.contains("0"));
	TEST_ASSERT(set1.size() == 1);

	set1.merge(set0);

	TEST_ASSERT(set0.contains("0"));
	TEST_ASSERT(!set0.contains("1"));
	TEST_ASSERT(set0.size() == 1);
	TEST_ASSERT(set1.contains("0"));
	TEST_ASSERT(set1.contains("1"));
	TEST_ASSERT(set1.size() == 2);

	TEST_ASSERT(set0.emplace("2").second);
	TEST_ASSERT(set0.contains("2"));
	TEST_ASSERT(set0.size() == 2);

	TEST_ASSERT(set1.insert(set0.extract("2")).inserted);

	TEST_ASSERT(!set0.contains("2"));
	TEST_ASSERT(set0.size() == 1);
	TEST_ASSERT(set1.contains("2"));
	TEST_ASSERT(set1.size() == 3);

	auto node = set0.extract("0");
	TEST_ASSERT(!set0.contains("0"));
	TEST_ASSERT(!node.empty());

	auto result = set1.insert(std::move(node));
	TEST_ASSERT(!result.inserted);
	TEST_ASSERT(!result.node.empty());

	TEST_ASSERT(set0.emplace("4").second);
	TEST_ASSERT(set0.contains("4"));

	node = set0.extract("4");
	TEST_ASSERT(!set0.contains("4"));
	TEST_ASSERT(!node.empty());

	result = set1.insert(std::move(node));
	TEST_ASSERT(result.inserted);
	TEST_ASSERT(result.node.empty());
	TEST_ASSERT(set1.contains("4"));
}