#pragma once

#include <plugify/api/version.hpp>

namespace plugify {
	enum class Type {
		LanguageModule,
		Plugin
	};

	struct Manifest {
		std::string name;
		fs::path path;
		Type type{};
		Version version{};
		std::optional<std::string> description;
		std::optional<std::string> author;
		std::optional<std::string> website;
		std::optional<std::string> license;
		std::optional<std::vector<std::string>> platforms;
		//std::optional<std::vector<Dependency>> dependencies;
		//std::optional<std::vector<Conflict>> conflicts;
		//std::optional<std::unordered_map<std::string, std::string>> metadata;
	};
} // namespace plugify
