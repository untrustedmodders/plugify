#pragma once

namespace plugify {
	template<typename Cnt, typename Pr = std::equal_to<typename Cnt::value_type>>
	constexpr bool RemoveDuplicates(Cnt& cnt, Pr cmp = Pr()) {
		auto size = std::size(cnt);
		Cnt result;
		result.reserve(size);

		std::copy_if(
				std::make_move_iterator(std::begin(cnt)),
				std::make_move_iterator(std::end(cnt)),
				std::back_inserter(result),
				[&](const typename Cnt::value_type& what) {
					return std::find_if(std::begin(result), std::end(result), [&](const typename Cnt::value_type& existing) {
							   return cmp(what, existing);
						   }) == std::end(result);
				}
		);

		cnt = std::move(result);
		return std::size(cnt) != size;
	}

	/*constexpr bool RemoveDuplicates(std::vector<T>& vec) {
		size_t original_size = vec.size();
		std::ranges::sort(vec);
		auto [first, last] = std::ranges::unique(vec);
		vec.erase(first, last);
		return vec.size() != original_size;
	}*/

	constexpr bool SupportsPlatform(const std::optional<std::vector<std::string>>& supportedPlatforms) {
		if (!supportedPlatforms || supportedPlatforms->empty())
			return true;

		constexpr std::string_view platform = PLUGIFY_PLATFORM; // e.g., "linux_x64"

		constexpr auto separator_pos = platform.find('_');
		static_assert(separator_pos != std::string_view::npos,
					  "PLUGIFY_PLATFORM must be in the format 'os_arch'");

		constexpr std::string_view os = platform.substr(0, separator_pos);
		constexpr std::string_view arch = platform.substr(separator_pos + 1);

		return std::ranges::any_of(*supportedPlatforms, [&](const std::string& supported) {
			// Exact match
			if (supported == platform)
				return true;

			// Wildcard support: "linux_*" matches any Linux, "*_x64" matches any x64
			if (supported.ends_with("_*")) {
				return supported.substr(0, supported.size() - 2) == os;
			}
			if (supported.starts_with("*_")) {
				return supported.substr(2) == arch;
			}
			if (supported == "*" || supported == "*_*") {
				return true; // Matches all platforms
			}

			return false;
		});
	}
} // namespace plugify
