#pragma once

#include <filesystem>

namespace plugify {
	// std::hash<std::filesystem::path> not in the C++20 standard by default
	struct path_hash {
		auto operator()(const fs::path& path) const noexcept {
			return hash_value(path);
		}
	};

	struct string_hash {
		using is_transparent = void; // Enables heterogeneous lookup

		auto operator()(const char *txt) const {
			return std::hash<std::string_view>{}(txt);
		}
		auto operator()(std::string_view txt) const {
			return std::hash<std::string_view>{}(txt);
		}
		auto operator()(const std::string& txt) const {
			return std::hash<std::string>{}(txt);
		}
	};

	struct case_insensitive_hash {
		using is_transparent = void; // Enables heterogeneous lookup

		template<typename T>
		auto operator()(const T& str_like) const noexcept {
			std::string_view str = str_like;
			std::size_t hash = 0xcbf29ce484222325; // FNV-1a 64-bit basis
			for (char c : str) {
				hash ^= static_cast<unsigned char>(std::tolower(static_cast<unsigned char>(c)));
				hash *= 0x100000001b3;
			}
			return hash;
		}
	};

	struct case_insensitive_equal {
		using is_transparent = void; // Enables heterogeneous lookup

		template<typename T1, typename T2>
		bool operator()(const T1& lhs_like, const T2& rhs_like) const noexcept {
			std::string_view lhs = lhs_like;
			std::string_view rhs = rhs_like;

			if (lhs.size() != rhs.size())
				return false;

			for (size_t i = 0; i < lhs.size(); ++i) {
				if (std::tolower(static_cast<unsigned char>(lhs[i])) !=
					std::tolower(static_cast<unsigned char>(rhs[i])))
					return false;
			}
			return true;
		}
	};
}
