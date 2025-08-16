#pragma once

#include "plugify.hpp"
#include "manager.hpp"
#include "provider.hpp"

#include <plugify/api/plugify.hpp>
#include <plugify/api/date_time.hpp>

namespace plugify {
	class Plugify {
	public:
		Plugify();
		~Plugify();

		bool Initialize(const fs::path& rootDir);
		void Terminate();
		bool IsInitialized() const;
		void Update();

		void Log(std::string_view msg, Severity severity);
		void SetLogger(std::shared_ptr<ILogger> logger);

		ManagerHandle GetManager() const;
		ProviderHandle GetProvider() const;
		const Config& GetConfig() const;
		Version GetVersion() const;


		void SetAssemblyLoader(std::shared_ptr<IAssemblyLoader> loader);
		std::shared_ptr<IAssemblyLoader> GetAssemblyLoader() const;

	private:
		Provider _provider;
		Manager _manager;
		Config _config;
		DateTime _deltaTime;
		DateTime _lastTime;
		Version _version{ PLUGIFY_VERSION };
		bool _inited{ false };

		std::shared_ptr<IAssemblyLoader> _loader;
	};
}

