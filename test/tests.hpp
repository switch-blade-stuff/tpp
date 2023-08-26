/*
 * Created by switchblade on 2022-11-7.
 */

#pragma once

#include "assert.hpp"

#ifndef TPP_USE_MODULES
#include <string_view>
#endif

void test_dense_set() noexcept;
void test_dense_map() noexcept;
void test_ordered_dense_set() noexcept;
void test_ordered_dense_map() noexcept;
void test_dense_multiset() noexcept;
void test_dense_multimap() noexcept;

void test_sparse_set() noexcept;
void test_sparse_map() noexcept;
void test_ordered_sparse_set() noexcept;
void test_ordered_sparse_map() noexcept;

void test_stable_set() noexcept;
void test_stable_map() noexcept;
void test_ordered_stable_set() noexcept;
void test_ordered_stable_map() noexcept;

static constexpr std::pair<std::string_view, void (*)()> tests[] = {
		{"dense_set", test_dense_set},
		{"dense_map", test_dense_map},
		{"ordered_dense_set", test_ordered_dense_set},
		{"ordered_dense_map", test_ordered_dense_map},
		{"dense_multiset", test_dense_multiset},
		{"dense_multimap", test_dense_multimap},

		{"sparse_set", test_sparse_set},
		{"sparse_map", test_sparse_map},
		{"ordered_sparse_set", test_ordered_sparse_set},
		{"ordered_sparse_map", test_ordered_sparse_map},

		{"stable_set", test_stable_set},
		{"stable_map", test_stable_map},
		{"ordered_stable_set", test_ordered_stable_set},
		{"ordered_stable_map", test_ordered_stable_map},
};