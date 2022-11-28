/*
 * Created by switchblade on 11/27/22.
 */

#pragma once

#include "table_util.hpp"

#ifndef TPP_USE_IMPORT

#include <vector>
#include <limits>
#include <tuple>

#endif

namespace tpp::detail
{
	template<typename V, typename K, typename Traits, typename KHash, typename KCmp, typename KGet, typename MGet, typename Alloc,
	         typename Link = empty_link>
	class swiss_table : Link, ebo_container<KHash>, ebo_container<KCmp>
	{

	};
}