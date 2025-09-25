#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "plg/string.hpp"

namespace plg {
	// std::hash<std::filesystem::path> not in the C++20 standard by default
	struct path_hash {
		auto operator()(const std::filesystem::path& path) const noexcept {
			return hash_value(path);
		}
	};

	struct string_hash {
		using is_transparent = void;  // Enables heterogeneous lookup

		auto operator()(const char* txt) const {
			return std::hash<std::string_view>{}(txt);
		}

		auto operator()(std::string_view txt) const {
			return std::hash<std::string_view>{}(txt);
		}

		auto operator()(const std::string& txt) const {
			return std::hash<std::string>{}(txt);
		}

		auto operator()(const plg::string& txt) const {
			return std::hash<plg::string>{}(txt);
		}
	};

	// --- Hash traits depending on pointer size ---
	template <std::size_t Size>
	struct hash_traits;

	template <>
	struct hash_traits<4> { // 32-bit
		static constexpr std::size_t fnv_basis = 0x811C9DC5u;
		static constexpr std::size_t fnv_prime = 0x01000193u;
		static constexpr std::size_t golden_ratio = 0x9e3779b9u;
	};

	template <>
	struct hash_traits<8> { // 64-bit
		static constexpr std::size_t fnv_basis = 0xcbf29ce484222325ULL;
		static constexpr std::size_t fnv_prime = 0x100000001b3ULL;
		static constexpr std::size_t golden_ratio = 0x9e3779b97f4a7c15ULL;
	};

	using active_hash_traits = hash_traits<sizeof(std::size_t)>;

	struct case_insensitive_hash {
		using is_transparent = void;  // Enables heterogeneous lookup

		template <typename T>
		auto operator()(const T& str_like) const noexcept {
			std::string_view str = str_like;
			std::size_t hash = active_hash_traits::fnv_basis; // FNV-1a
			for (const char& c : str) {
				hash ^= static_cast<unsigned char>(std::tolower(static_cast<unsigned char>(c)));
				hash *= active_hash_traits::fnv_prime;
			}
			return hash;
		}
	};

	struct case_insensitive_equal {
		using is_transparent = void;  // Enables heterogeneous lookup

		template <typename T1, typename T2>
		bool operator()(const T1& lhs_like, const T2& rhs_like) const noexcept {
			std::string_view lhs = lhs_like;
			std::string_view rhs = rhs_like;

			if (lhs.size() != rhs.size()) {
				return false;
			}

			for (size_t i = 0; i < lhs.size(); ++i) {
				if (std::tolower(static_cast<unsigned char>(lhs[i]))
				    != std::tolower(static_cast<unsigned char>(rhs[i]))) {
					return false;
				}
			}
			return true;
		}
	};

	inline void hash_combine(size_t&) {
	}

	template <class T>
	inline void hash_combine(std::size_t& seed, const T& v) {
		std::hash<T> hasher;
		seed ^= hasher(v) + active_hash_traits::golden_ratio + (seed << 6) + (seed >> 2);
	}

	template <class... Ts>
	inline std::size_t hash_combine_all(const Ts&... args) {
		std::size_t seed = 0;
		(hash_combine(seed, args), ...);  // fold expression
		return seed;
	}

	template <typename T1, typename T2>
	struct pair_hash {
		size_t operator()(const std::pair<T1, T2>& p) const {
			return hash_combine_all(p.first, p.second);
		}
	};
}
