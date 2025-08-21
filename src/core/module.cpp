#include "plugify/core/module.hpp"

#include "plugify/_/module_handle.hpp"
#include "plugify/_/provider_handle.hpp"
#include "plugify/core/plugify.hpp"
#include "plugify/asm/mem_protector.hpp"
#include "plugify/core/plugin.hpp"
#include "plugify/core/provider.hpp"

using namespace plugify;

Module::Module(UniqueId id, BasePaths paths, std::shared_ptr<Manifest> manifest)
	: _id{id}, _paths{std::move(paths)}, _manifest{std::static_pointer_cast<ModuleManifest>(std::move(manifest))} {
	PL_ASSERT(_manifest->type != PackageType::Plugin && "Invalid package type for module ctor");
	PL_ASSERT(_manifest->path.has_parent_path() && "Package path doesn't contain parent path");
}

Module::Module(Module&& module) noexcept {
	*this = std::move(module);
}

bool Module::Initialize(Plugify& plugify) {
	PL_ASSERT(GetState() != ModuleState::Loaded && "Module already was initialized");

	auto loader = plugify.GetAssemblyLoader();

	if (loader->CanLinkSearchPaths()) {
		std::vector<std::string> errors;

		if (const auto& directories = _manifest->directories) {
			for (const auto& rawPath : *directories) {
				std::filesystem::path libraryDirectory = _paths.base / rawPath;
				if (!loader->AddSearchPath(libraryDirectory)) {
					errors.push_back(libraryDirectory.string());
					return false;
				}
			}
		}

		if (!errors.empty()) {
			SetError(std::format("Found invalid {} directory(s)", plg::join(errors, ", ")));
			return false;
		}
	}

	LoadFlag flags = LoadFlag::Lazy | LoadFlag::Global | /**/ LoadFlag::SearchUserDirs | LoadFlag::SearchSystem32 | LoadFlag::SearchDllLoadDir;
	if (plugify.GetConfig().preferOwnSymbols.value_or(false)) {
		flags |= LoadFlag::Deepbind;
	}

	// Language module library must be named 'lib${module name}(.dylib|.so|.dll)'.
	auto filePath = _paths.base / "bin" / std::format(PLUGIFY_LIBRARY_PREFIX "{}" PLUGIFY_LIBRARY_SUFFIX, _manifest->name);

	auto res = loader->Load(filePath, flags);
	if (!res) {
		SetError(std::format("Failed to load library: '{}' at: '{}' - {}", GetName(), filePath.string(), res.error()));
		return false;
	}

	auto& assembly = *res;

	auto GetLanguageModuleFunc = assembly->GetSymbol(kGetLanguageModuleFn).RCast<ILanguageModule*(*)()>();
	if (!GetLanguageModuleFunc) {
		SetError(std::format("Function '{}' not exist inside '{}' library", kGetLanguageModuleFn, filePath.string()));
		Terminate();
		return false;
	}

	auto* languageModule = GetLanguageModuleFunc();
	if (!languageModule) {
		SetError(std::format("Function '{}' inside '{}' library. Returned invalid address of 'ILanguageModule' implementation!", kGetLanguageModuleFn, filePath.string()));
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

	auto result = languageModule->Initialize(plugify.GetProvider(), *this);
	if (!result) {
		SetError(std::format("Failed to initialize module: '{}' error: '{}' at: '{}'", GetName(), result.error(), filePath.string()));
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

	_loadedPlugins.clear();

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

	auto result = _languageModule->OnPluginLoad(plugin);
	if (!result) {
		plugin.SetError(std::format("Failed to load plugin: '{}' error: '{}' at: '{}'", plugin.GetName(), result.error(), plugin.GetBaseDir().string()));
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

			if (method != *exportedMethod || !addr) {
				errors.emplace_back(exportedMethod->name);
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

	_loadedPlugins.emplace_back(plugin);

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
	PL_ASSERT(error.empty() && "Empty error string!");
	_error = std::move(error);
	_state = ModuleState::Error;
	PL_LOG_ERROR("Module '{}': {}", GetName(), GetError());
}
