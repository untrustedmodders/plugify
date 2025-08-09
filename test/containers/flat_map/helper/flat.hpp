#pragma once

#include "plg/flat_map.hpp"
#include "plg/vector.hpp"

namespace plg::flat::test {
	template<typename Key, typename Value, typename Compare = std::less<Key>, typename Container = vector<std::pair<Key, Value>>>
	using flatmap = flat_map<Key, Value, Compare, Container>;

} // namespace plg::packed::test
