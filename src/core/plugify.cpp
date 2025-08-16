#include "plugify.hpp"

#include <plugify/api/config.hpp>
#include <util/json.hpp>

using namespace plugify;

Plugify::Plugify() : _provider(*this), _manager(*this) {
};

Plugify::~Plugify() {
	Terminate();
};

bool Plugify::Initialize(const fs::path& rootDir) {
	if (IsInitialized())
		return false;

	auto configPath = rootDir / "plugify.pconfig";
	PL_LOG_DEBUG("Config path: '{}'", configPath.string());
	auto json = "";//FileSystem::ReadText(configPath);
	auto config = glz::read_jsonc<Config>(json);
	if (!config.has_value()) {
		PL_LOG_ERROR("Config: '{}' has JSON parsing error: {}", configPath.string(), glz::format_error(config.error(), json));
		return false;
	}

	{
		const auto checkPath = [](const fs::path& p) {
			return !p.empty() && p.lexically_normal() == p;
		};

		if (!checkPath(config->configsDir)) {
			PL_LOG_ERROR("Config configsDir must be relative directory path");
			return false;
		}
		if (!checkPath(config->dataDir)) {
			PL_LOG_ERROR("Config dataDir must be relative directory path");
			return false;
		}
		if (!checkPath(config->logsDir)) {
			PL_LOG_ERROR("Config logsDir must be relative directory path");
			return false;
		}
		std::array<fs::path, 5> dirs = {
			"packages",
			config->configsDir,
			config->dataDir,
			config->logsDir,
		};

		const auto isPathCollides = [](const fs::path& first, const fs::path& second) {
			auto [itFirst, itSecond] = std::ranges::mismatch(first, second);
			return itFirst == first.end() || itSecond == second.end();
		};

		for (auto first = dirs.begin(); first != dirs.end(); ++first) {
			for (auto second = first + 1; second != dirs.end(); ++second) {
				if (isPathCollides(*first, *second)) {
					PL_LOG_ERROR("Config configsDir, dataDir, logsDir must not share paths with each other or with 'packages'");
					return false;
				}
			}
		}
	}

	_config = std::move(*config);

	if (!rootDir.empty())
		_config.baseDir = rootDir / _config.baseDir;

	_inited = true;

	PL_LOG_INFO("Plugify Init!");
	PL_LOG_INFO("Version: {}", _version);
	PL_LOG_INFO("Git: [{}]:({}) - {} on {} at '{}'", PLUGIFY_GIT_COMMIT_HASH, PLUGIFY_GIT_TAG, PLUGIFY_GIT_COMMIT_SUBJECT, PLUGIFY_GIT_BRANCH, PLUGIFY_GIT_COMMIT_DATE); //-V001
	PL_LOG_INFO("Compiled on: {} from: {} with: '{}'", PLUGIFY_COMPILED_SYSTEM, PLUGIFY_COMPILED_GENERATOR, PLUGIFY_COMPILED_COMPILER);

	return true;
}

void Plugify::Terminate() {
	if (!IsInitialized())
		return;

	_lastTime = DateTime::Now();

	_manager.Terminate();

	_inited = false;

	PL_LOG_INFO("Plugify Terminated!");
}

bool Plugify::IsInitialized() const {
	return _inited;
}

void Plugify::Update() {
	if (!IsInitialized())
		return;

	auto currentTime = DateTime::Now();
	_deltaTime = (currentTime - _lastTime);
	_lastTime = currentTime;

	_manager.Update(_deltaTime);
}

void Plugify::Log(std::string_view msg, Severity severity) const {
	LogSystem::Log(msg, severity);
}

void Plugify::SetLogger(std::shared_ptr<ILogger> logger) const {
	LogSystem::SetLogger(std::move(logger));
}

ManagerHandle Plugify::GetManager() const {
	return _manager;
}

ProviderHandle Plugify::GetProvider() const {
	return _provider;
}

const Config& Plugify::GetConfig() const {
	return _config;
}

Version Plugify::GetVersion() const {
	return _version;
}

void Plugify::SetAssemblyLoader(std::shared_ptr<IAssemblyLoader> loader) {
	_loader = std::move(loader);
}

std::shared_ptr<IAssemblyLoader> Plugify::GetAssemblyLoader() const {
	return _loader;
}

PLUGIFY_API PlugifyHandle plugify::MakePlugify() {
	static Plugify plugify;
	return plugify;
}