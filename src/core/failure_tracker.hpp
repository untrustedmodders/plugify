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

		// Takes the forward graph - what each extension depends on - not the reverse
		// one. Handed the reverse graph it asks whether any extension that depends on
		// this one has failed, which is a different question and almost always no.
		bool HasAnyDependencyFailed(
			const Extension& ext,
			const std::unordered_map<UniqueId, std::vector<UniqueId>>& dependencies
		) const {
			std::shared_lock lock(_mutex);

			// Check if any of this extension's dependencies have failed
			if (auto it = dependencies.find(ext.GetId()); it != dependencies.end()) {
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
			const std::unordered_map<UniqueId, std::vector<UniqueId>>& dependencies
		) const {
			std::shared_lock lock(_mutex);

			if (auto it = dependencies.find(ext.GetId()); it != dependencies.end()) {
				for (const auto& depId : it->second) {
					if (_failedExtensions.contains(depId)) {
						// Find the name of the failed dependency
						return ToString(depId);
					}
				}
			}
			return {};
		}
	};
}
