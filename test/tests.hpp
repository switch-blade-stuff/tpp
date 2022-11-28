/*
 * Created by switchblade on 11/7/22.
 */

#pragma once

#include <string_view>

#include "assert.hpp"

void test_seahash() noexcept;
void test_crc32() noexcept;
void test_fnv1a() noexcept;
void test_sdbm() noexcept;
void test_md5() noexcept;

void test_dense_map() noexcept;
void test_ordered_dense_map() noexcept;
void test_dense_set() noexcept;
void test_ordered_dense_set() noexcept;
void test_dense_multiset() noexcept;

constexpr static std::pair<std::string_view, void (*)()> tests[] = {
		{"dense_map", test_dense_map},
		{"ordered_dense_map", test_ordered_dense_map},
		{"dense_set", test_dense_set},
		{"ordered_dense_set", test_ordered_dense_set},
		{"dense_multiset", test_dense_multiset},

		{"seahash", test_seahash},
		{"crc32", test_crc32},
		{"fnv1a", test_fnv1a},
		{"sdbm", test_sdbm},
		{"md5", test_md5},
};