#pragma once

#include <plugify/api/version.hpp>

namespace plugify {
	enum class ManifestType {
		LanguageModule,
		Plugin
	};

	struct Manifest {
		fs::path path;
		std::string name;
		ManifestType type{};
		Version version{};
		std::optional<std::string> description;
		std::optional<std::string> author;
		std::optional<std::string> website;
		std::optional<std::string> license;
		std::optional<std::vector<std::string>> platforms;
	};

} // namespace plugify
