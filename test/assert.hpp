/*
 * Created by switchblade on 2022-23-09
 */

#pragma once

#ifndef TPP_DEBUG
#define TPP_DEBUG
#define UNDEF_DEBUG
#endif

#include <tpp/detail/utility.hpp>

#define TEST_ASSERT(cnd) tpp::detail::assert_msg((cnd), (#cnd), (__FILE__), (__LINE__), (TPP_PRETTY_FUNC), nullptr)

#ifdef UNDEF_DEBUG
#undef UNDEF_DEBUG
#undef TPP_DEBUG
#endif