/*
 * Created by switchblade on 23/09/22
 */

#pragma once

#define TPP_DEBUG
#include <tpp/detail/utility.hpp>

#define TEST_ASSERT(cnd) tpp::detail::assert_msg((cnd), (#cnd), (__FILE__), (__LINE__), (TPP_PRETTY_FUNC), nullptr)