/*
 * Created by switch_blade on 2022-12-15.
 */

#pragma once

#include "assert.hpp"

#include <tpp/detail/multikey.hpp>
#include <string>

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
	TEST_ASSERT(!map0.contains("0"));
	TEST_ASSERT(!map0.contains("1"));
	TEST_ASSERT(map0.begin() == map0.end());

	TEST_ASSERT(map1.size() == 2);
	TEST_ASSERT(map1.contains("0"));
	TEST_ASSERT(map1.contains("1"));
	TEST_ASSERT(map1.begin() != map1.end());

	auto map2 = map1;

	TEST_ASSERT(map2.size() == 2);
	TEST_ASSERT(map2.contains("0"));
	TEST_ASSERT(map2.contains("1"));
	TEST_ASSERT(map2.begin() != map2.end());

	TEST_ASSERT(map2 != map0);
	TEST_ASSERT(map2 == map1);

	map1.clear();
	TEST_ASSERT(map1.size() == 0);
	TEST_ASSERT(!map1.contains("0"));
	TEST_ASSERT(!map1.contains("1"));
	TEST_ASSERT(map1.find("0") == map1.end());
	TEST_ASSERT(map1.find("1") == map1.end());
	TEST_ASSERT(map1.begin() == map1.end());

	const int n = 0x1000;
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
	auto map0 = map_t();
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
	TEST_ASSERT(!map0.contains("0"));
	TEST_ASSERT(!map0.contains("1"));
	TEST_ASSERT(map0.begin() == map0.end());

	TEST_ASSERT(map1.size() == 2);
	TEST_ASSERT(map1.begin() != map1.end());
	TEST_ASSERT(map1.contains("0"));
	TEST_ASSERT(map1.contains("1"));
	TEST_ASSERT(map1.find("0") == map1.begin());
	TEST_ASSERT(map1.find("1") == std::prev(map1.end()));
	TEST_ASSERT(*map1.find("0") == map1.front());
	TEST_ASSERT(*map1.find("1") == map1.back());

	auto map2 = map1;

	TEST_ASSERT(map2.size() == 2);
	TEST_ASSERT(map2.contains("0"));
	TEST_ASSERT(map2.contains("1"));
	TEST_ASSERT(map2.find("0") == map2.begin());
	TEST_ASSERT(map2.find("1") == std::prev(map2.end()));
	TEST_ASSERT(*map2.find("0") == map2.front());
	TEST_ASSERT(*map2.find("1") == map2.back());

	TEST_ASSERT(map2 != map0);
	TEST_ASSERT(map2 == map1);
}

template<template<typename...> typename T, typename map_t = T<tpp::multikey<std::string, int>, float>>
static void test_multimap() noexcept
{
	auto map0 = map_t{};

	TEST_ASSERT(map0.size() == 0);

	TEST_ASSERT(map0.try_emplace(std::forward_as_tuple("0", 0), 0.0f).second);
	TEST_ASSERT(map0.size() == 1);
	TEST_ASSERT(map0.template contains<0>("0"));
	TEST_ASSERT(map0.template contains<1>(0));
	TEST_ASSERT(map0.template find<0>("0")->second == 0.0f);
	TEST_ASSERT(std::get<1>(map0.template find<0>("0")->first) == 0);
	TEST_ASSERT(map0.template find<1>(0)->second == 0.0f);
	TEST_ASSERT(std::get<0>(map0.template find<1>(0)->first) == "0");
	TEST_ASSERT(map0.template find<0>("0") == map0.template find<1>(0));

	TEST_ASSERT(map0.try_emplace(std::forward_as_tuple("1", 1), 1.0f).second);
	TEST_ASSERT(map0.size() == 2);
	TEST_ASSERT(map0.template contains<0>("1"));
	TEST_ASSERT(map0.template contains<1>(1));
	TEST_ASSERT(map0.template find<0>("1")->second == 1.0f);
	TEST_ASSERT(std::get<1>(map0.template find<0>("1")->first) == 1);
	TEST_ASSERT(map0.template find<1>(1)->second == 1.0f);
	TEST_ASSERT(std::get<0>(map0.template find<1>(1)->first) == "1");
	TEST_ASSERT(map0.template find<0>("1") == map0.template find<1>(1));

	TEST_ASSERT(!map0.try_emplace(std::forward_as_tuple("0", 0), 0.0f).second);
	TEST_ASSERT(!map0.try_emplace(std::forward_as_tuple("0", 1), 0.0f).second);
	TEST_ASSERT(!map0.try_emplace(std::forward_as_tuple("0", 2), 0.0f).second);
	TEST_ASSERT(!map0.try_emplace(std::forward_as_tuple("1", 0), 0.0f).second);
	TEST_ASSERT(!map0.try_emplace(std::forward_as_tuple("1", 1), 0.0f).second);
	TEST_ASSERT(!map0.try_emplace(std::forward_as_tuple("1", 2), 0.0f).second);
	TEST_ASSERT(!map0.try_emplace(std::forward_as_tuple("2", 0), 0.0f).second);
	TEST_ASSERT(!map0.try_emplace(std::forward_as_tuple("2", 1), 0.0f).second);

	auto map1 = map_t{std::move(map0)};

	TEST_ASSERT(map0.template find<0>("0") == map0.end());
	TEST_ASSERT(map0.template find<1>(0) == map0.end());
	TEST_ASSERT(map0.template find<0>("1") == map0.end());
	TEST_ASSERT(map0.template find<1>(1) == map0.end());

	TEST_ASSERT(map1.template find<0>("0") != map1.end());
	TEST_ASSERT(map1.template find<0>("1") != map1.end());
	TEST_ASSERT(map1.template find<1>(0) != map1.end());
	TEST_ASSERT(map1.template find<1>(1) != map1.end());
	TEST_ASSERT(map1.template find<0>("0") == map1.template find<1>(0));
	TEST_ASSERT(map1.template find<0>("1") == map1.template find<1>(1));

	auto map2 = map1;

	TEST_ASSERT(map2.template find<0>("0") != map2.end());
	TEST_ASSERT(map2.template find<0>("1") != map2.end());
	TEST_ASSERT(map2.template find<1>(0) != map2.end());
	TEST_ASSERT(map2.template find<1>(1) != map2.end());
	TEST_ASSERT(map2.template find<0>("0") == map2.template find<1>(0));
	TEST_ASSERT(map2.template find<0>("1") == map2.template find<1>(1));

	TEST_ASSERT(map2 != map0);
	TEST_ASSERT(map2 == map1);
}

template<template<typename...> typename T, typename map_t = T<std::string, int>>
static void test_node_map() noexcept
{
	auto map0 = map_t{};

	TEST_ASSERT(map0.size() == 0);
	TEST_ASSERT(map0.try_emplace("0", 0).second);
	TEST_ASSERT(map0.try_emplace("1", 1).second);
	TEST_ASSERT(map0.contains("0"));
	TEST_ASSERT(map0.contains("1"));
	TEST_ASSERT(map0.size() == 2);

	auto map1 = map_t{};

	TEST_ASSERT(map1.size() == 0);
	TEST_ASSERT(map1.try_emplace("0", 0).second);
	TEST_ASSERT(map1.contains("0"));
	TEST_ASSERT(map1.size() == 1);

	map1.merge(map0);

	TEST_ASSERT(map0.contains("0"));
	TEST_ASSERT(!map0.contains("1"));
	TEST_ASSERT(map0.size() == 1);
	TEST_ASSERT(map1.contains("0"));
	TEST_ASSERT(map1.contains("1"));
	TEST_ASSERT(map1.size() == 2);

	TEST_ASSERT(map0.try_emplace("2", 2).second);
	TEST_ASSERT(map0.contains("2"));
	TEST_ASSERT(map0.size() == 2);

	TEST_ASSERT(map1.insert(map0.extract("2")).inserted);

	TEST_ASSERT(!map0.contains("2"));
	TEST_ASSERT(map0.size() == 1);
	TEST_ASSERT(map1.contains("2"));
	TEST_ASSERT(map1.size() == 3);

	auto node = map0.extract("0");
	TEST_ASSERT(!map0.contains("0"));
	TEST_ASSERT(!node.empty());
	node.mapped() = 10;

	TEST_ASSERT(map1.at("0") == 0);

	auto result = map1.insert(std::move(node));
	TEST_ASSERT(!result.inserted);
	TEST_ASSERT(!result.node.empty());
	TEST_ASSERT(map1.at("0") == 0);

	TEST_ASSERT(!map1.insert_or_assign(std::move(result.node)).second);
	TEST_ASSERT(map1.at("0") == 10);

	TEST_ASSERT(map0.try_emplace("4", 4).second);
	TEST_ASSERT(map0.contains("4"));

	node = map0.extract("4");
	TEST_ASSERT(!map0.contains("4"));
	TEST_ASSERT(!node.empty());

	result = map1.insert(std::move(node));
	TEST_ASSERT(result.inserted);
	TEST_ASSERT(result.node.empty());
	TEST_ASSERT(map1.contains("4"));
	TEST_ASSERT(map1.at("4") == 4);
}