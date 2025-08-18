#pragma once

#include "conflict.hpp"
#include "dependency.hpp"
#include <plugify/api/constraint.hpp>

namespace plugify {
	enum class ManifestType {
		Module,
		Plugin
	};

	struct Manifest {
		//PackageId id;
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
		//std::optional<std::unordered_map<std::string, std::string>> metadata;

		void Validate(StackLogger& logger) const {
			ScopeLogger{logger};

			if (name.empty()) {
				logger.Log("Missing name");
			}

			if (conflicts && !conflicts->empty()) {
				size_t i = 0;
				ScopeLogger{logger};
				for (const auto& dependency : *conflicts) {
					if (dependency->name.empty()) {
						logger.Log(std::format("Missing conflict name at {}", ++i));
					}
				}
			}

			if (dependencies && !dependencies->empty()) {
				size_t i = 0;
				ScopeLogger{logger};
				for (const auto& dependency : *dependencies) {
					if (dependency->name.empty()) {
						logger.Log(std::format("Missing dependency name at {}", ++i));
					}
				}
			}
		}
	};

	struct BasePaths {
		fs::path base;
		fs::path configs;
		fs::path data;
		fs::path logs;
	};
} // namespace plugify
