/*
 * Created by switchblade on 2022-23-09
 */

#pragma once

#ifndef TPP_DEBUG
#define TPP_DEBUG
#endif

#include <tpp/detail/utility.hpp>
#define TEST_ASSERT(cnd) TPP_ASSERT(cnd, nullptr)