#include "module.hpp"
#include "plugin.hpp"
#include <plugify/api/module.hpp>
#include <plugify/api/plugify_provider.hpp>
#include <plugify/asm/mem_protector.hpp>

using namespace plugify;

Module::Module(UniqueId id, std::unique_ptr<Manifest> manifest, fs::path path)
	: _id{id}
	, _manifest{std::unique_ptr<ModuleManifest>(static_cast<ModuleManifest*>(manifest.release()))} {
	PL_ASSERT(package.type != PackageType::Plugin && "Invalid package type for module ctor");
	PL_ASSERT(package.path.has_parent_path() && "Package path doesn't contain parent path");
	// Language module library must be named 'lib${module name}(.dylib|.so|.dll)'.
	_baseDir = path.parent_path();
	_filePath = _baseDir / "bin" / std::format(PLUGIFY_LIBRARY_PREFIX "{}" PLUGIFY_LIBRARY_SUFFIX, _manifest->name);
}

Module::Module(Module&& module) noexcept {
	*this = std::move(module);
}

bool Module::Initialize(const std::shared_ptr<IPlugifyProvider>& provider) {
	PL_ASSERT(GetState() != ModuleState::Loaded && "Module already was initialized");

	IAssemblyLoader* loader;// = provider->GetAssemblyLoader();
	if (!loader) {
		SetError("Assembly loader is not provided!");
		return false;
	}

	std::vector<std::string> errors;

	if (const auto& libraryDirectoriesSettings = _manifest->directories) {
		for (const auto& rawPath : *libraryDirectoriesSettings) {
			fs::path libraryDirectory = _baseDir / rawPath;
			if (!loader->AddSearchPath(libraryDirectory.native())) {
				errors.emplace_back(libraryDirectory.string());
				return false;
			}
		}
	}

	if (!errors.empty()) {
		SetError(std::format("Found invalid {} directory(s)", plg::join(errors, ", ")));
		return false;
	}

	LoadFlag flags = LoadFlag::Lazy | LoadFlag::Global | /**/ LoadFlag::SearchUserDirs | LoadFlag::SearchSystem32 | LoadFlag::SearchDllLoadDir;
	if (provider->IsPreferOwnSymbols()) {
		flags |= LoadFlag::Deepbind;
	}

	AssemblyResult res = loader->Load(_filePath.native(), flags);
	if (!res) {
		SetError(std::format("Failed to load library: '{}' at: '{}' - {}", _name, _filePath.string(), res.error().string()));
		return false;
	}

	auto& assembly = *res;

	auto GetLanguageModuleFunc = assembly->GetSymbol(kGetLanguageModuleFn).RCast<ILanguageModule*(*)()>();
	if (!GetLanguageModuleFunc) {
		SetError(std::format("Function '{}' not exist inside '{}' library", kGetLanguageModuleFn, _filePath.string()));
		Terminate();
		return false;
	}

	ILanguageModule* languageModule = GetLanguageModuleFunc();
	if (!languageModule) {
		SetError(std::format("Function '{}' inside '{}' library. Returned invalid address of 'ILanguageModule' implementation!", kGetLanguageModuleFn, _filePath.string()));
		Terminate();
		return false;
	}

#if PLUGIFY_PLATFORM_WINDOWS
	constexpr bool plugifyBuildType = PLUGIFY_IS_DEBUG;
	bool moduleBuildType = languageModule->IsDebugBuild();
	if (moduleBuildType != plugifyBuildType) {
		SetError(std::format("Mismatch between plugify ({}) build type and module ({}) build type.", (plugifyBuildType ? "debug" : "release"), (moduleBuildType ? "debug" : "release"))); //-V547
		Terminate();
		return false;
	}
#endif // PLUGIFY_PLATFORM_WINDOWS

	InitResult result = languageModule->Initialize(provider, *this);
	if (!result) {
		SetError(std::format("Failed to initialize module: '{}' error: '{}' at: '{}'", _name, result.error().string(), _filePath.string()));
		Terminate();
		return false;
	}

	_assembly = std::move(assembly);
	_languageModule = languageModule;
	_table = result->table;

	SetLoaded();
	return true;
}

void Module::Terminate() {
	if (_languageModule) {
		_languageModule->Shutdown();
		_languageModule = nullptr;
	}
	_assembly.reset();

#if PLUGIFY_IS_DEBUG
	_loadedPlugins.clear();
#endif

	SetUnloaded();
}

void Module::Update(DateTime dt) {
	if (_languageModule && _table.hasUpdate) {
		_languageModule->OnUpdate(dt);
	}
}

bool Module::LoadPlugin(Plugin& plugin) const {
	if (_state != ModuleState::Loaded)
		return false;

	LoadResult result = _languageModule->OnPluginLoad(plugin);
	if (!result) {
		plugin.SetError(std::format("Failed to load plugin: '{}' error: '{}' at: '{}'", plugin.GetName(), result.error().string(), plugin.GetBaseDir().string()));
		return false;
	}

	if (const auto& methods = plugin.GetManifest().methods) {
		if (result->methods.size() != methods->size()) {
			plugin.SetError(std::format("Mismatch in methods count, expected: {} but provided: {}", methods->size(), result->methods.size()));
			return false;
		}

		std::vector<std::string_view> errors;

		for (size_t i = 0; i < result->methods.size(); ++i) {
			const auto& [method, addr] = result->methods[i];
			const auto& exportedMethod = (*methods)[i];

			if (method != exportedMethod || !addr) {
				errors.emplace_back(exportedMethod.name);
			}
		}

		if (!errors.empty()) {
			plugin.SetError(std::format("Found invalid {} method(s)", plg::join(errors, ", ")));
			return false;
		}

		plugin.SetMethods(std::move(result->methods));
	}

	plugin.SetTable(result->table);
	plugin.SetData(result->data);

	plugin.SetLoaded();

#if PLUGIFY_IS_DEBUG
	_loadedPlugins.emplace_back(&plugin);
#endif

	return true;
}

void Module::MethodExport(Plugin& plugin) const {
	if (_state != ModuleState::Loaded)
		return;

	if (plugin.HasExport()) {
		_languageModule->OnMethodExport(plugin);
	}
}

void Module::StartPlugin(Plugin& plugin) const  {
	if (_state != ModuleState::Loaded)
		return;

	if (plugin.HasStart()) {
		_languageModule->OnPluginStart(plugin);
	}

	plugin.SetRunning();
}

void Module::UpdatePlugin(Plugin& plugin, DateTime dt) const  {
	if (_state != ModuleState::Loaded)
		return;

	if (plugin.HasUpdate()) {
		_languageModule->OnPluginUpdate(plugin, dt);
	}
}

void Module::EndPlugin(Plugin& plugin) const {
	if (_state != ModuleState::Loaded)
		return;

	if (plugin.HasEnd()) {
		_languageModule->OnPluginEnd(plugin);
	}

	plugin.SetTerminating();
}

void Module::SetError(std::string error) {
	_error = std::move(error);
	_state = ModuleState::Error;
	PL_LOG_ERROR("Module '{}': {}", _name, _error);
}
