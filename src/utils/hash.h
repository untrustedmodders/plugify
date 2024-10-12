#pragma once

#include <filesystem>

namespace plugify {
	// std::hash<std::filesystem::path> not in the C++20 standard by default
	struct path_hash {
		[[nodiscard]] auto operator()(const fs::path& path) const noexcept {
			return hash_value(path);
		}
	};

	// heterogeneous lookup
	struct string_hash {
		using is_transparent = void;
		[[nodiscard]] size_t operator()(const char *txt) const {
			return std::hash<std::string_view>{}(txt);
		}
		[[nodiscard]] size_t operator()(std::string_view txt) const {
			return std::hash<std::string_view>{}(txt);
		}
		[[nodiscard]] size_t operator()(const std::string &txt) const {
			return std::hash<std::string>{}(txt);
		}
	};
}
