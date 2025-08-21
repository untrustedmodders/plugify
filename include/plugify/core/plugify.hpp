#pragma once

#include "plugify/core/provider.hpp"
#include "plugify/core/manager.hpp"
#include "plugify/core/config.hpp"

#include "plugify_export.h"

namespace plugify {
	class PLUGIFY_API Plugify {
		struct Impl;
	public:
		Plugify();
		~Plugify();

		bool Initialize(const std::filesystem::path& rootDir);
		void Terminate() const;
		bool IsInitialized() const;
		void Update() const;

		std::shared_ptr<Manager> GetManager() const;
		std::shared_ptr<Provider> GetProvider() const;
		std::shared_ptr<Config> GetConfig() const;
		Version GetVersion() const;

		void SetAssemblyLoader(std::shared_ptr<IAssemblyLoader> loader);
		std::shared_ptr<IAssemblyLoader> GetAssemblyLoader() const;

		void SetFileSystem(std::shared_ptr<IFileSystem> reader);
		std::shared_ptr<IFileSystem> GetFileSystem() const;

		void Log(std::string_view msg, Severity severity) const;
		void SetLogger(std::shared_ptr<ILogger> logger);

	private:
		std::unique_ptr<Impl> _impl;
	};

	PLUGIFY_API std::shared_ptr<Plugify> MakePlugify();
}

