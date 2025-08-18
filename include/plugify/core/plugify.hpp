#pragma once

#include <plugify/core/provider.hpp>
#include <plugify/core/manager.hpp>
#include <plugify/core/config.hpp>

#include <plugify_export.h>

namespace plugify {
	class PLUGIFY_API Plugify {
	public:
		Plugify();
		~Plugify();

		bool Initialize(const std::filesystem::path& rootDir);
		void Terminate() const;
		bool IsInitialized() const;

		void Update() const;

		const Manager& GetManager() const;
		const Provider& GetProvider() const;
		const Config& GetConfig() const;
		const Version& GetVersion() const;

		void SetAssemblyLoader(std::shared_ptr<IAssemblyLoader> loader) const;
		std::shared_ptr<IAssemblyLoader> GetAssemblyLoader() const;

		void SetFileSystem(std::shared_ptr<IFileSystem> reader) const;
		std::shared_ptr<IFileSystem> GetFileSystem() const;

		void Log(std::string_view msg, Severity severity) const;
		void SetLogger(std::shared_ptr<ILogger> logger) const;

	private:
		class Impl;
		std::unique_ptr<Impl> _impl;
	};

	PLUGIFY_API std::shared_ptr<Plugify> MakePlugify();
}

