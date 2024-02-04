#pragma once

#include <plugify_export.h>
#include <filesystem>
#include <string>
#include <memory>

namespace plugify {
	class PlugifyProvider;
	enum class Severity : uint8_t;

	// Plugify provided to user, which implemented in core
	class PLUGIFY_API IPlugifyProvider {
	protected:
		explicit IPlugifyProvider(PlugifyProvider& impl);
		~IPlugifyProvider();

	public:
		void Log(const std::string& msg, Severity severity) const;

		const std::filesystem::path& GetBaseDir() const;

	private:
		PlugifyProvider& _impl;
	};
}
