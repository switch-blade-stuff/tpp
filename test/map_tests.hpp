/*
 * Created by switch_blade on 2022-12-15.
 */

#pragma once

#include "assert.hpp"

template<template<typename...> typename T, typename map_t = T<std::string, int>>
static void test_map() noexcept
{
	auto map0 = map_t{};

	TEST_ASSERT(map0.size() == 0);

	TEST_ASSERT(map0.try_emplace("0", 0).second);
	TEST_ASSERT(map0.size() == 1);
	TEST_ASSERT(map0.contains("0"));
	TEST_ASSERT(map0.find("0")->second == 0);

	TEST_ASSERT(map0.try_emplace("1", 1).second);
	TEST_ASSERT(map0.size() == 2);
	TEST_ASSERT(map0.contains("1"));
	TEST_ASSERT(map0.find("1")->second == 1);
	TEST_ASSERT(map0.find("1") != map0.find("0"));

	TEST_ASSERT(!map0.try_emplace("0", 1).second);
	TEST_ASSERT(map0.size() == 2);
	TEST_ASSERT(map0.contains("0"));
	TEST_ASSERT(map0.find("0")->second == 0);

	TEST_ASSERT(!map0.emplace_or_replace("0", 1).second);
	TEST_ASSERT(map0.size() == 2);
	TEST_ASSERT(map0.contains("0"));
	TEST_ASSERT(map0.find("0")->second == 1);

	TEST_ASSERT(!map0.insert_or_assign("0", 2).second);
	TEST_ASSERT(map0.size() == 2);
	TEST_ASSERT(map0.contains("0"));
	TEST_ASSERT(map0.find("0")->second == 2);

	map0.erase(map0.find("0"));
	TEST_ASSERT(map0.size() == 1);
	TEST_ASSERT(!map0.contains("0"));

	TEST_ASSERT(map0.insert_or_assign("0", 0).second);
	TEST_ASSERT(map0.size() == 2);
	TEST_ASSERT(map0.contains("0"));
	TEST_ASSERT(map0.find("0")->second == 0);

	auto map1 = map_t{std::move(map0)};
	TEST_ASSERT(map0.size() == 0);
	TEST_ASSERT(map1.size() == 2);
	TEST_ASSERT(map0.begin() == map0.end());
	TEST_ASSERT(map1.begin() != map1.end());

	map1.clear();
	TEST_ASSERT(map1.size() == 0);
	TEST_ASSERT(!map1.contains("0"));
	TEST_ASSERT(!map1.contains("1"));
	TEST_ASSERT(map1.find("0") == map1.end());
	TEST_ASSERT(map1.find("1") == map1.end());
	TEST_ASSERT(map1.begin() == map1.end());

	const int n = 0x10000;
	for (int i = 0; i < n; ++i)
	{
		const auto key = std::to_string(i);
		const auto result = map1.emplace(key, i);

		TEST_ASSERT(result.second);
		TEST_ASSERT(map1.contains(key));
		TEST_ASSERT(map1.find(key) != map1.end());
		TEST_ASSERT(map1.find(key) == result.first);
		TEST_ASSERT(map1.find(key)->first == key);
		TEST_ASSERT(map1.find(key)->second == i);
	}
	TEST_ASSERT(map1.size() == static_cast<std::size_t>(n));

	for (int i = 0; i < n; ++i)
	{
		const auto key = std::to_string(i);
		TEST_ASSERT(map1.contains(key));
		TEST_ASSERT(map1.find(key) != map1.end());
		TEST_ASSERT(map1.find(key)->first == key);
		TEST_ASSERT(map1.find(key)->second == i);

		map1.erase(key);
		TEST_ASSERT(!map1.contains(key));
		TEST_ASSERT(map1.find(key) == map1.end());
	}
	TEST_ASSERT(map1.size() == 0);
	TEST_ASSERT(map1.begin() == map1.end());
}

template<template<typename...> typename T, typename map_t = T<std::string, int>>
static void test_ordered_map() noexcept
{
	auto map0 = map_t{};
	TEST_ASSERT(map0.size() == 0);

	TEST_ASSERT(map0.try_emplace("0", 0).second);
	TEST_ASSERT(map0.size() == 1);
	TEST_ASSERT(map0.contains("0"));
	TEST_ASSERT(map0.find("0")->second == 0);
	TEST_ASSERT(map0.find("0") == map0.begin());
	TEST_ASSERT(*map0.find("0") == map0.front());

	TEST_ASSERT(map0.try_emplace("1", 1).second);
	TEST_ASSERT(map0.size() == 2);
	TEST_ASSERT(map0.contains("1"));
	TEST_ASSERT(map0.find("1")->second == 1);
	TEST_ASSERT(map0.find("1") == std::prev(map0.end()));
	TEST_ASSERT(*map0.find("1") == map0.back());
	TEST_ASSERT(map0.find("1") != map0.find("0"));

	TEST_ASSERT(!map0.try_emplace("0", 1).second);
	TEST_ASSERT(map0.size() == 2);
	TEST_ASSERT(map0.contains("0"));
	TEST_ASSERT(map0.find("0")->second == 0);
	TEST_ASSERT(map0.find("0") == map0.begin());
	TEST_ASSERT(*map0.find("0") == map0.front());
	TEST_ASSERT(map0.find("1") != map0.find("0"));

	auto map1 = map_t{std::move(map0)};
	TEST_ASSERT(map0.size() == 0);
	TEST_ASSERT(map0.begin() == map0.end());
	TEST_ASSERT(map1.size() == 2);
	TEST_ASSERT(map1.begin() != map1.end());
	TEST_ASSERT(map1.contains("0"));
	TEST_ASSERT(map1.contains("1"));
	TEST_ASSERT(map1.find("0") == map1.begin());
	TEST_ASSERT(map1.find("1") == std::prev(map1.end()));
	TEST_ASSERT(*map1.find("0") == map1.front());
	TEST_ASSERT(*map1.find("1") == map1.back());
}