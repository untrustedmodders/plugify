#pragma once

#include <filesystem>

namespace std::filesystem {
	// std::hash<std::filesystem::path> not in the C++20 standard by default
	struct hash {
		auto operator()(const path& path) const noexcept {
			return hash_value(path);
		}
	};
}