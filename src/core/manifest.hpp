#pragma once

#include "conflict.hpp"
#include "dependency.hpp"
#include "algorithm.hpp"
#include <plugify/api/version.hpp>

namespace plugify {
	enum class ManifestType {
		Module,
		Plugin
	};

	struct Manifest {
		fs::path path;
		std::string name;
		ManifestType type{};
		Version version{};
		std::string description;
		std::string author;
		std::string website;
		std::string license;
		std::optional<std::vector<std::string>> platforms;
		std::optional<std::vector<std::unique_ptr<Dependency>>> dependencies;
		std::optional<std::vector<std::unique_ptr<Conflict>>> conflicts;

		std::vector<std::string> Validate() {
			std::vector<std::string> errors;

			if (name.empty()) {
				errors.emplace_back("Missing name");
			}

			if (conflicts && !conflicts->empty()) {
				if (RemoveDuplicates(*conflicts)) {
					PL_LOG_WARNING("Manifest: '{}' has multiple same conflicts!", name);
				}
				size_t i = 0;
				for (const auto& dependency : *conflicts) {
					if (dependency->name.empty()) {
						errors.emplace_back(std::format("Missing conflict name at {}", ++i));
					}
				}
			}

			if (dependencies && !dependencies->empty()) {
				if (RemoveDuplicates(*dependencies)) {
					PL_LOG_WARNING("Manifest: '{}' has multiple same dependencies!", name);
				}
				size_t i = 0;
				for (const auto& dependency : *dependencies) {
					if (dependency->name.empty()) {
						errors.emplace_back(std::format("Missing dependency name at {}", ++i));
					}
				}
			}

			return errors;
		}
	};

	struct BasePaths {
		fs::path base;
		fs::path configs;
		fs::path data;
		fs::path logs;
	};
} // namespace plugify
