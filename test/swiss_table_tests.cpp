/*
 * Created by switchblade on 11/15/22.
 */

#include "tests.hpp"

#ifdef TPP_NO_HASH
#undef TPP_NO_HASH
#endif

#ifndef TPP_STL_HASH_ALL
#define TPP_STL_HASH_ALL
#endif

#include <tpp/swiss_set.hpp>
#include <tpp/swiss_map.hpp>

using namespace tpp;
