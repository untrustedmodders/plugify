#include "plugify/core/plugify.hpp"
#include "core/glaze.hpp"

using namespace plugify;

struct Plugify::Impl {
	explicit Impl(Plugify& self) {
	    provider = std::make_shared<Provider>(self);
	    manager = std::make_shared<Manager>(self);
	}

	std::shared_ptr<Provider> provider;
	std::shared_ptr<Manager> manager;
    std::shared_ptr<Config> config;

	DateTime deltaTime;
	DateTime lastTime;
	Version version;
	bool inited{false};

	std::shared_ptr<IAssemblyLoader> loader;
	std::shared_ptr<IFileSystem> fs;
};

// -------------------- Plugify API --------------------

Plugify::Plugify()
	: _impl(std::make_unique<Impl>(*this)) {
    plg::parse(PLUGIFY_VERSION, _impl->version);
}

Plugify::~Plugify() {
	Terminate();
}

bool Plugify::Initialize(const std::filesystem::path&) {
	/*if (IsInitialized())
		return false;

	if (!_impl->fs) {
		PL_LOG_ERROR("Plugify: File reader is not provided!");
		return false;
	}
	if (!_impl->loader) {
		PL_LOG_ERROR("Plugify: Assembly loader is not provided!");
		return false;
	}

	auto configPath = rootDir / "plugify.pconfig";
	PL_LOG_DEBUG("Config: '{}'", configPath.string());
	auto result = _impl->fs->ReadTextFile(configPath);
	if (!result) {
		PL_LOG_ERROR("Config: '{}' has reading error: {}", configPath.string(), result.error());
		return false;
	}

	auto config = glz::read_jsonc<Config>(*result);
	if (!config.has_value()) {
		PL_LOG_ERROR("Config: '{}' has JSON parsing error: {}", configPath.string(), glz::format_error(config.error(), *result));
		return false;
	}


	// ---- path validation block unchanged ----
	{
		const auto checkPath = [](const std::filesystem::path& p) {
			return !p.empty() && p.lexically_normal() == p;
		};

		std::vector<std::string> errors;

		if (!checkPath(config->configsDir)) {
			errors.push_back(std::format("configsDir: - '{}'", config->configsDir.string()));
		}
		if (!checkPath(config->dataDir)) {
			errors.push_back(std::format("dataDir: - '{}'", config->dataDir.string()));
		}
		if (!checkPath(config->logsDir)) {
			errors.push_back(std::format("logsDir: '{}'", config->logsDir.string()));
		}

		if (!errors.empty()) {
			PL_LOG_ERROR("Config: {} must be relative directory path(s)", plg::join(errors, ", "));
			return false;
		}

		std::array<std::filesystem::path, 5> dirs = {
				"packages",
				config->configsDir,
				config->dataDir,
				config->logsDir,
		};

		const auto isPathCollides = [](const std::filesystem::path& first, const std::filesystem::path& second) {
			auto [itFirst, itSecond] = std::ranges::mismatch(first, second);
			return itFirst == first.end() || itSecond == second.end();
		};

		for (auto first = dirs.begin(); first != dirs.end(); ++first) {
			for (auto second = first + 1; second != dirs.end(); ++second) {
				if (isPathCollides(*first, *second)) {
					errors.push_back(std::format("'{}' - '{}'", first->string(), second->string()));
				}
			}
		}

		if (!errors.empty()) {
			PL_LOG_ERROR("Config: {} must be not collide between each other", plg::join(errors, " and "));
			return false;
		}
	}

	_impl->config = {};//std::move(*config);

	if (!rootDir.empty())
		_impl->config.baseDir = rootDir / _impl->config.baseDir;*/
	_impl->config = std::make_shared<Config>();
    _impl->config->logSeverity = Severity::Verbose;
	_impl->inited = true;

	PL_LOG_INFO("Plugify Init!");
	PL_LOG_INFO("Version: {}", _impl->version);
	PL_LOG_INFO("Git: [{}]:({}) - {} on {} at '{}'", PLUGIFY_GIT_COMMIT_HASH, PLUGIFY_GIT_TAG, PLUGIFY_GIT_COMMIT_SUBJECT, PLUGIFY_GIT_BRANCH, PLUGIFY_GIT_COMMIT_DATE);//-V001
	PL_LOG_INFO("Compiled on: {} from: {} with: '{}'", PLUGIFY_COMPILED_SYSTEM, PLUGIFY_COMPILED_GENERATOR, PLUGIFY_COMPILED_COMPILER);

	return true;
}

void Plugify::Terminate() const {
	if (!IsInitialized())
		return;

	_impl->lastTime = DateTime::Now();
	auto _ = _impl->manager->Terminate();
	_impl->inited = false;

	PL_LOG_INFO("Plugify Terminated!");
}

bool Plugify::IsInitialized() const {
	return _impl->inited;
}

void Plugify::Update() const {
	if (!IsInitialized())
		return;

	auto currentTime = DateTime::Now();
	_impl->deltaTime = (currentTime - _impl->lastTime);
	_impl->lastTime = currentTime;

	auto it = _impl->manager->Update(/*_impl->deltaTime*/);
}

void Plugify::Log(std::string_view msg, Severity severity) const {
	LogSystem::Log(msg, severity);
}

void Plugify::SetLogger(std::shared_ptr<ILogger> logger) {
	LogSystem::SetLogger(std::move(logger));
}

std::shared_ptr<Manager> Plugify::GetManager() const {
	return _impl->manager;
}

std::shared_ptr<Provider> Plugify::GetProvider() const {
	return _impl->provider;
}

std::shared_ptr<Config> Plugify::GetConfig() const {
	return _impl->config;
}

Version Plugify::GetVersion() const {
	return _impl->version;
}

void Plugify::SetAssemblyLoader(std::shared_ptr<IAssemblyLoader> loader) {
	_impl->loader = std::move(loader);
}

std::shared_ptr<IAssemblyLoader> Plugify::GetAssemblyLoader() const {
	return _impl->loader;
}

void Plugify::SetFileSystem(std::shared_ptr<IFileSystem> reader) {
	_impl->fs = std::move(reader);
}

std::shared_ptr<IFileSystem> Plugify::GetFileSystem() const {
	return _impl->fs;
}

PLUGIFY_API std::shared_ptr<Plugify> plugify::MakePlugify() {
	return std::make_shared<Plugify>();
}
