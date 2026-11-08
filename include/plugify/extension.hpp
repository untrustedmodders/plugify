#pragma once

#include <memory>
#include <string>
#include <vector>
#include <filesystem>

#include "plugify/manifest.hpp"
#include "plg/path.hpp"

namespace plugify {
	class IAssembly;
	class ILanguageModule;

	/**
	 * @enum ExtensionState
	 * @brief Where an extension stands in the load pipeline.
	 *
	 * The states form a state machine the pipeline walks in order, from
	 * @ref Discovered to @ref Running and back out through @ref Terminated. Each
	 * stage has a state for the work in progress and one for its outcome, plus
	 * the dead ends it can fall into. Only the transitions
	 * Extension::IsValidTransition accepts ever happen; everything that got as
	 * far as @ref Loading can still be torn down, since it may hold a language
	 * module or an assembly.
	 */
	enum class ExtensionState {
		Unknown,  ///< Not yet seen by the pipeline.
		Discovered,  ///< A manifest file was found on disk.

		Parsing,  ///< The manifest is being read and validated.
		Parsed,  ///< The manifest is valid and its type tables are linked.
		Corrupted,  ///< The manifest could not be read or did not validate.

		Resolving,  ///< Dependencies, conflicts and platforms are being checked.
		Resolved,  ///< Every requirement is satisfied and a load order exists.
		Unresolved,  ///< A dependency is missing, out of range or part of a cycle.

		Disabled,  ///< Excluded by configuration, for instance by the whitelist or the blacklist.
		Skipped,  ///< Not loaded because something it depends on did not come up.
		Failed,  ///< A stage reported an error; see Extension::GetErrors.

		Loading,  ///< The assembly or the language module is bringing the extension up.
		Loaded,  ///< The extension is in memory and its methods are known.

		Exporting,  ///< The methods of a plugin are being handed to its language module.
		Exported,  ///< Every exported method is available to other extensions.

		Starting,  ///< The start callback of the extension is running.
		Started,  ///< The start callback returned successfully.

		Running,  ///< Fully operational; the update callback is being called.

		Ending,  ///< The end callback of the extension is running.
		Ended,  ///< The end callback returned; nothing but teardown follows.

		Terminating,  ///< The assembly and the language module are being released.
		Terminated  ///< Teardown finished; nothing of the extension is held any more.
	};

	/**
	 * @class Extension
	 * @brief A plugin or a language module, from discovery to teardown.
	 *
	 * An extension owns its parsed @ref Manifest and everything the pipeline
	 * attaches to it while it runs: its state, its timings, the errors and
	 * warnings it collected, and, depending on its kind, the loaded assembly of a
	 * module or the exported methods of a plugin. The getters of the kind that
	 * does not apply return empty values rather than failing.
	 *
	 * Instances live in the list of the @ref Manager and are handed out as
	 * pointers that stay valid until the next Manager::Initialize rebuilds it.
	 *
	 * The class is a PIMPL wrapper and is move-only.
	 *
	 * @see Manager, Manifest, ExtensionState
	 */
	class PLUGIFY_API Extension {
	public:
		/**
		 * @brief Construct an empty extension in state @ref ExtensionState::Unknown.
		 */
		Extension();

		/**
		 * @brief Construct an extension that was just discovered on disk.
		 * @param id Unique identifier assigned by the manager.
		 * @param location Path of the manifest file of the extension.
		 */
		Extension(UniqueId id, std::filesystem::path location);

		/**
		 * @brief Destructor.
		 */
		~Extension();

		// Copying would duplicate a loaded extension; moving is what the pipeline
		// does to reorder and filter them, so that one stays

		/**
		 * @brief Deleted copy constructor; copying would duplicate a loaded extension.
		 */
		Extension(const Extension&) = delete;

		/**
		 * @brief Move constructor.
		 */
		Extension(Extension&&) noexcept;

		/**
		 * @brief Deleted copy assignment operator; copying would duplicate a loaded extension.
		 */
		Extension& operator=(const Extension&) = delete;

		/**
		 * @brief Move assignment operator.
		 * @return A reference to this instance after moving.
		 */
		Extension& operator=(Extension&&) noexcept;

		// --- Core Getters ---

		/**
		 * @brief Get the unique identifier of this extension.
		 * @return The identifier the manager assigned at discovery.
		 */
		[[nodiscard]] UniqueId GetId() const noexcept;

		/**
		 * @brief Get the kind of this extension.
		 * @return Whether this is a plugin or a language module.
		 */
		[[nodiscard]] ExtensionType GetType() const noexcept;

		/**
		 * @brief Get the current state of this extension.
		 * @return Where the extension stands in the load pipeline.
		 */
		[[nodiscard]] ExtensionState GetState() const noexcept;

		/**
		 * @brief Get the name of this extension.
		 * @return The unique name from the manifest, as other extensions refer to it.
		 */
		[[nodiscard]] const std::string& GetName() const noexcept;

		/**
		 * @brief Get the version of this extension.
		 * @return The semantic version from the manifest.
		 */
		[[nodiscard]] const Version& GetVersion() const noexcept;

		/**
		 * @brief Get the language of this extension.
		 * @return For a plugin, the language module that runs it; for a module, the language it
		 * provides support for.
		 */
		[[nodiscard]] const std::string& GetLanguage() const noexcept;

		/**
		 * @brief Get the location of this extension.
		 * @return Path of its manifest file.
		 */
		[[nodiscard]] const std::filesystem::path& GetLocation() const noexcept;

		// --- Optional Info Getters ---

		/**
		 * @brief Get the description of this extension.
		 * @return What the extension does, empty if the manifest omits it.
		 */
		[[nodiscard]] const std::string& GetDescription() const noexcept;

		/**
		 * @brief Get the author of this extension.
		 * @return Name or identifier of the author, empty if the manifest omits it.
		 */
		[[nodiscard]] const std::string& GetAuthor() const noexcept;

		/**
		 * @brief Get the website of this extension.
		 * @return Website, repository or documentation URL, empty if the manifest omits it.
		 */
		[[nodiscard]] const std::string& GetWebsite() const noexcept;

		/**
		 * @brief Get the license of this extension.
		 * @return License identifier, empty if the manifest omits it.
		 */
		[[nodiscard]] const std::string& GetLicense() const noexcept;

		// --- Dependencies/Conflicts ---

		/**
		 * @brief Get the platforms this extension supports.
		 * @return Platforms as 'os_arch'; an empty list means every platform.
		 */
		[[nodiscard]] const std::vector<std::string>& GetPlatforms() const noexcept;

		/**
		 * @brief Get the extensions this one needs.
		 * @return The dependencies, which are loaded before this extension.
		 */
		[[nodiscard]] const std::vector<Dependency>& GetDependencies() const noexcept;

		/**
		 * @brief Get the extensions this one cannot be loaded with.
		 * @return The conflicts.
		 */
		[[nodiscard]] const std::vector<Conflict>& GetConflicts() const noexcept;

		/**
		 * @brief Get the extensions this one supersedes.
		 * @return The obsolete relations.
		 */
		[[nodiscard]] const std::vector<Obsolete>& GetObsoletes() const noexcept;

		// --- Plugin-specific (returns empty/null for modules) ---

		/**
		 * @brief Get the entry point of this plugin.
		 * @return Entry point file from the manifest, empty for a module.
		 */
		[[nodiscard]] const std::string& GetEntry() const noexcept;

		/**
		 * @brief Get the methods this plugin declares.
		 * @return The declared signatures, empty for a module.
		 */
		[[nodiscard]] const std::vector<Method>& GetMethods() const noexcept;

		/**
		 * @brief Get the class wrappers this plugin declares.
		 * @return The classes, empty for a module.
		 */
		[[nodiscard]] const std::vector<Class>& GetClasses() const noexcept;

		/**
		 * @brief Get the shared prototype table of this plugin.
		 * @return Every distinct callback signature of the manifest, flattened by Manifest::Resolve.
		 */
		[[nodiscard]] const std::vector<std::shared_ptr<Prototype>>& GetPrototypes() const noexcept;

		/**
		 * @brief Get the shared enumeration table of this plugin.
		 * @return Every distinct enumeration of the manifest, flattened by Manifest::Resolve.
		 */
		[[nodiscard]] const std::vector<std::shared_ptr<Enum>>& GetEnums() const noexcept;

		/**
		 * @brief Get the methods this plugin actually exported.
		 * @return Each declared method paired with the address its language module resolved it to,
		 * filled in once the extension reaches @ref ExtensionState::Loaded. Empty for a module.
		 */
		[[nodiscard]] const std::vector<MethodData>& GetMethodsData() const noexcept;

		// --- Module-specific (returns empty/null for plugins) ---

		/**
		 * @brief Get the shared library of this module.
		 * @return Path to the library, already resolved to the naming convention of the platform.
		 * Empty for a plugin.
		 */
		[[nodiscard]] const std::filesystem::path& GetRuntime() const noexcept;

		/**
		 * @brief Get the extra library search paths of this module.
		 * @return Directories added to the search path when the module is loaded. Empty for a plugin.
		 */
		[[nodiscard]] const std::vector<std::filesystem::path>& GetDirectories() const noexcept;

		/**
		 * @brief Get the loaded assembly of this module.
		 * @return The assembly, or nullptr for a plugin and for a module that is not loaded.
		 */
		[[nodiscard]] std::shared_ptr<IAssembly> GetAssembly() const noexcept;

		// --- Shared Runtime ---

		/**
		 * @brief Get the data its language module associated with this extension.
		 * @return The address the module handed back from its load callback, null if there is none.
		 */
		[[nodiscard]] Address GetUserData() const noexcept;

		/**
		 * @brief Get which optional callbacks this extension has.
		 * @return The method table, used to avoid calling callbacks that do not exist.
		 */
		[[nodiscard]] MethodTable GetMethodTable() const noexcept;

		/**
		 * @brief Get the language module that runs this extension.
		 * @return The module, or nullptr before the extension is loaded.
		 * @note For a language module this is its own interface, once it is initialized.
		 */
		[[nodiscard]] ILanguageModule* GetLanguageModule() const noexcept;

		/**
		 * @brief Get the parsed manifest of this extension.
		 * @return The manifest, empty until the extension reaches @ref ExtensionState::Parsed.
		 */
		[[nodiscard]] const Manifest& GetManifest() const noexcept;

		// --- State & Error Management ---

		/**
		 * @brief Get the errors collected for this extension.
		 * @return Every error a stage reported, in the order they happened.
		 */
		[[nodiscard]] const std::vector<std::string>& GetErrors() const noexcept;

		/**
		 * @brief Get the warnings collected for this extension.
		 * @return Every warning a stage reported, in the order they happened.
		 */
		[[nodiscard]] const std::vector<std::string>& GetWarnings() const noexcept;

		/**
		 * @brief Check whether anything reported an error for this extension.
		 * @return True if there is at least one error.
		 */
		[[nodiscard]] bool HasErrors() const noexcept;

		/**
		 * @brief Check whether anything reported a warning for this extension.
		 * @return True if there is at least one warning.
		 */
		[[nodiscard]] bool HasWarnings() const noexcept;

		/**
		 * @brief Check whether this extension is a plugin.
		 * @return True if its type is @ref ExtensionType::Plugin.
		 */
		[[nodiscard]] bool IsPlugin() const noexcept {
			return GetType() == ExtensionType::Plugin;
		}

		/**
		 * @brief Check whether this extension is a language module.
		 * @return True if its type is @ref ExtensionType::Module.
		 */
		[[nodiscard]] bool IsModule() const noexcept {
			return GetType() == ExtensionType::Module;
		}

		// --- Timing/Performance ---

		/**
		 * @brief Get how long one stage took.
		 * @param state The state the stage ran in.
		 * @return The measured duration, or zero if that stage never ran.
		 */
		[[nodiscard]] std::chrono::milliseconds GetOperationTime(ExtensionState state) const;

		/**
		 * @brief Get how long every stage took together.
		 * @return The sum of all measured durations.
		 */
		[[nodiscard]] std::chrono::milliseconds GetTotalTime() const;

		/**
		 * @brief Get a human-readable breakdown of the timings.
		 * @return The name of the extension followed by the duration of each stage.
		 */
		[[nodiscard]] std::string GetPerformanceReport() const;

		// --- State Management ---

		/**
		 * @brief Enter a stage and start timing it.
		 * @param newState The state of the stage that is starting.
		 */
		void StartOperation(ExtensionState newState);

		/**
		 * @brief Leave the current stage, recording how long it took.
		 * @param newState The state to move to.
		 */
		void EndOperation(ExtensionState newState);

		/**
		 * @brief Move this extension to another state.
		 * @param state The state to move to.
		 * @note The transition has to be one @ref IsValidTransition accepts.
		 */
		void SetState(ExtensionState state);

		// --- Error/Warning Management ---

		/**
		 * @brief Record an error for this extension.
		 * @param error The error message.
		 */
		void AddError(std::string error);

		/**
		 * @brief Record a warning for this extension.
		 * @param warning The warning message.
		 */
		void AddWarning(std::string warning);

		/**
		 * @brief Drop every recorded error.
		 */
		void ClearErrors();

		/**
		 * @brief Drop every recorded warning.
		 */
		void ClearWarnings();

		// --- Runtime Updates ---

		/**
		 * @brief Associate language-module data with this extension.
		 * @param data The address the module handed back from its load callback.
		 */
		void SetUserData(Address data);

		/**
		 * @brief Set which optional callbacks this extension has.
		 * @param table The method table.
		 */
		void SetMethodTable(MethodTable table);

		/**
		 * @brief Set the language module that runs this extension.
		 * @param module The module interface.
		 */
		void SetLanguageModule(ILanguageModule* module);

		/**
		 * @brief Set the parsed manifest of this extension.
		 * @param manifest The manifest, already resolved and validated.
		 */
		void SetManifest(Manifest manifest);

		// --- Plugin-specific setters ---

		/**
		 * @brief Set the methods this plugin exported.
		 * @param methodsData Each declared method paired with its resolved address.
		 * @note Ignored for a language module.
		 */
		void SetMethodsData(std::vector<MethodData> methodsData);

		// --- Module-specific setters ---

		/**
		 * @brief Set the loaded assembly of this module.
		 * @param assembly The assembly.
		 * @note Ignored for a plugin.
		 */
		void SetAssembly(std::shared_ptr<IAssembly> assembly);

		// --- Comparison ---

		/**
		 * @brief Compare two extensions for equality.
		 * @param other The extension to compare against.
		 * @return True if both have the same identifier.
		 */
		[[nodiscard]] bool operator==(const Extension& other) const noexcept;

		/**
		 * @brief Order two extensions.
		 * @param other The extension to compare against.
		 * @return The relative ordering of their identifiers.
		 */
		[[nodiscard]] auto operator<=>(const Extension& other) const noexcept;

		// --- File extensions ---

		/**
		 * @brief Work out the kind of extension a manifest file describes.
		 * @param path Path of the manifest file.
		 * @return @ref ExtensionType::Plugin for a '.pplugin' file,
		 * @ref ExtensionType::Module for a '.pmodule' file, and
		 * @ref ExtensionType::Unknown for anything else.
		 */
		[[nodiscard]] static ExtensionType GetExtensionType(const std::filesystem::path& path);

		// --- Helpers ---

		/**
		 * @brief Check whether the state machine allows a transition.
		 * @param from The current state.
		 * @param to The state to move to.
		 * @return True if the pipeline is allowed to make that move.
		 * @note Teardown reaches further than the forward path: anything that got as far as
		 * @ref ExtensionState::Loading can still move to @ref ExtensionState::Terminating,
		 * because it may hold a language module or an assembly that has to be given back.
		 */
		[[nodiscard]] static bool IsValidTransition(ExtensionState from, ExtensionState to);

		/**
		 * @brief Describe this extension in one line.
		 * @return Its identifier, name, type, state and the number of errors and warnings.
		 */
		[[nodiscard]] std::string ToString() const;

		/**
		 * @brief Get the version of this extension as text.
		 * @return The version, formatted once and cached.
		 */
		[[nodiscard]] const std::string& GetVersionString() const noexcept;

		/**
		 * @brief Add a dependency to the manifest of this extension.
		 * @param dep Name of the required extension.
		 * @note Used by the pipeline to record a requirement that is implied rather than declared,
		 * such as the language module a plugin needs.
		 */
		void AddDependency(std::string dep);

		/**
		 * @brief Return this extension to its freshly constructed state.
		 */
		void Reset();

	private:
		struct Impl;  ///< Implementation details, hidden from consumers.
		PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};
}
