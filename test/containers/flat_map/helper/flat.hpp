#pragma once

#include <plugify/flat_map.hpp>

namespace plg::flat::test {
	template<typename Key, typename Value, typename Compare = std::less<Key>, typename Container = plg::vector<std::pair<Key, Value>>>
	using flatmap = flat_map<Key, Value, Compare, Container>;

} // namespace plg::packed::test
