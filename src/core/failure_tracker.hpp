#pragma once

#include "plugify/registrar.hpp"

namespace plugify {
	// Shared failure tracker that can be passed between stages
	class FailureTracker {
		std::unordered_set<UniqueId> _failedExtensions;
		mutable std::shared_mutex _mutex;

	public:
		explicit FailureTracker(size_t capacity) {
			_failedExtensions.reserve(capacity);
		}

		void MarkFailed(UniqueId id) {
			std::unique_lock lock(_mutex);
			_failedExtensions.insert(id);
		}

		bool HasFailed(UniqueId id) const {
			std::shared_lock lock(_mutex);
			return _failedExtensions.contains(id);
		}

		bool HasAnyDependencyFailed(
			const Extension& ext,
			const std::flat_map<UniqueId, std::vector<UniqueId>>& reverseDeps
		) const {
			std::shared_lock lock(_mutex);

			// Check if any of this extension's dependencies have failed
			if (auto it = reverseDeps.find(ext.GetId()); it != reverseDeps.end()) {
				for (const auto& depId : it->second) {
					if (_failedExtensions.contains(depId)) {
						return true;
					}
				}
			}
			return false;
		}

		std::string GetFailedDependencyName(
			const Extension& ext,
			const std::flat_map<UniqueId, std::vector<UniqueId>>& reverseDeps
		) const {
			std::shared_lock lock(_mutex);

			if (auto it = reverseDeps.find(ext.GetId()); it != reverseDeps.end()) {
				for (const auto& depId : it->second) {
					if (_failedExtensions.contains(depId)) {
						// Find the name of the failed dependency
						return ToShortString(depId);
					}
				}
			}
			return {};
		}
	};
}