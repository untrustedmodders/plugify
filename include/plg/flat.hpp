#pragma once

#include <version>
#include <functional>

#ifdef __cpp_lib_flat_map
#  include <flat_map>
#else
#  include <map>
#endif

#ifdef __cpp_lib_flat_set
#  include <flat_set>
#else
#  include <set>
#endif

namespace std {
#ifndef __cpp_lib_flat_map
	template<typename Key, typename T, typename Compare = less<Key>>
	using flat_map = map<Key, T, Compare>;
#endif

#ifndef __cpp_lib_flat_set
	template<typename Key, typename Compare = less<Key>>
	using flat_set = set<Key, Compare>;
#endif
}