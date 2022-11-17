/*
 * Created by switchblade on 11/7/22.
 */

#pragma once

#include <string_view>

#include "assert.hpp"

void test_dense_table() noexcept;
void test_ordered_dense_table() noexcept;

void test_seahash() noexcept;
void test_crc32() noexcept;
void test_fnv1a() noexcept;
void test_sdbm() noexcept;
void test_md5() noexcept;

constexpr static std::pair<std::string_view, void (*)()> tests[] = {
		{"dense_table",         test_dense_table},
		{"ordered_dense_table", test_ordered_dense_table},

		{"seahash",             test_seahash},
		{"crc32",               test_crc32},
		{"fnv1a",               test_fnv1a},
		{"sdbm",                test_sdbm},
		{"md5",                 test_md5},
};