#pragma once

#include <memory>
#include <string>
#include <vector>
#include <chrono>

#include "plugify/extension.hpp"
#include "plugify/global.h"
#include "plugify/types.hpp"

namespace plugify {
	struct Config;
	class Extension;
	class ServiceLocator;

	/**
	 * @class Manager
	 * @brief Owns every extension and runs them through the load pipeline.
	 *
	 * The manager discovers manifests under the configured extensions directory,
	 * parses them, resolves their dependencies, loads them in the resulting
	 * order, and tears them down again. It also owns the extension list that the
	 * query methods below read from, and is the single place that keeps the
	 * language modules and the plugins they run in step.
	 *
	 * The class is a PIMPL wrapper, neither copyable nor movable, and is normally
	 * reached through Plugify::GetManager.
	 *
	 * @see Extension, Plugify, Provider
	 */
	class PLUGIFY_API Manager {
	public:
		/**
		 * @brief Construct a manager.
		 * @param services The service container the manager resolves its services from.
		 * @param config The configuration that decides the paths, the timeouts and the filters.
		 * @note Both references have to outlive the manager.
		 */
		Manager(const ServiceLocator& services, const Config& config);

		/**
		 * @brief Destructor.
		 */
		~Manager();

		/**
		 * @brief Deleted copy constructor.
		 */
		Manager(const Manager& other) = delete;

		/**
		 * @brief Deleted move constructor.
		 */
		Manager(Manager&& other) noexcept = delete;

		/**
		 * @brief Deleted copy assignment operator.
		 */
		Manager& operator=(const Manager& other) = delete;

		/**
		 * @brief Deleted move assignment operator.
		 */
		Manager& operator=(Manager&& other) noexcept = delete;

		// Lifecycle

		/**
		 * @brief Run the whole load pipeline.
		 *
		 * Discovers manifests, parses and validates them, resolves dependencies
		 * and conflicts, then loads, exports and starts every extension that
		 * survived in dependency order.
		 *
		 * @return Nothing on success, an error message if the pipeline could not run.
		 * @note An extension that fails on its own does not fail the call; it ends up in
		 * @ref ExtensionState::Failed with its reason in Extension::GetErrors.
		 */
		Result<void> Initialize() const;

		/**
		 * @brief Check whether the pipeline has run.
		 * @return True between a successful Initialize() and the next Terminate().
		 */
		[[nodiscard]] bool IsInitialized() const;

		/**
		 * @brief Call the update callback of every running extension.
		 * @param deltaTime Time since the previous update.
		 */
		void Update(std::chrono::milliseconds deltaTime) const;

		/**
		 * @brief Tear down every extension, in reverse dependency order.
		 */
		void Terminate() const;

		// Query operations. The pointers point into the manager's own list and stay
		// valid until the next Initialize rebuilds it, so nothing worth keeping
		// across a reload should be kept as a pointer.

		/**
		 * @brief Check whether an extension is loaded and in range.
		 * @param name Name of the extension.
		 * @param constraint Version constraints the extension has to satisfy; omit to accept any
		 * version.
		 * @return True if the extension is running and its version matches.
		 */
		[[nodiscard]] bool
		IsExtensionLoaded(std::string_view name, std::optional<Constraint> constraint = {}) const noexcept;

		/**
		 * @brief Look an extension up by name.
		 * @param name Name of the extension, as written in its manifest.
		 * @return The extension, or nullptr if no running extension has that name.
		 */
		[[nodiscard]] const Extension* FindExtension(std::string_view name) const noexcept;

		/**
		 * @brief Look an extension up by identifier.
		 * @param id Identifier the manager assigned at discovery.
		 * @return The extension, or nullptr if no extension has that identifier.
		 */
		[[nodiscard]] const Extension* FindExtension(UniqueId id) const noexcept;

		/**
		 * @brief Get every extension the manager knows about.
		 * @return Pointers to all extensions, whatever state they are in.
		 */
		[[nodiscard]] std::vector<const Extension*> GetExtensions() const;

		/**
		 * @brief Get the extensions that are in one state.
		 * @param state The state to filter by.
		 * @return Pointers to the matching extensions.
		 */
		[[nodiscard]] std::vector<const Extension*> GetExtensionsByState(ExtensionState state) const;

		/**
		 * @brief Get the extensions of one kind.
		 * @param type Whether to return plugins or language modules.
		 * @return Pointers to the matching extensions.
		 */
		[[nodiscard]] std::vector<const Extension*> GetExtensionsByType(ExtensionType type) const;

		// Dump operations

		/**
		 * @brief Describe the order extensions were loaded in.
		 * @return A human-readable listing of the load order.
		 */
		[[nodiscard]] std::string GenerateLoadOrder() const;

		/**
		 * @brief Describe how the extensions depend on each other.
		 * @return A human-readable rendering of the dependency graph.
		 */
		[[nodiscard]] std::string GenerateDependencyGraph() const;

		/**
		 * @brief Describe the dependency graph in Graphviz DOT format.
		 * @return The graph as DOT source, ready to be written out or rendered.
		 */
		[[nodiscard]] std::string GenerateDependencyGraphDOT() const;

		/**
		 * @brief Compare two managers for equality.
		 * @param other The manager to compare against.
		 * @return True if both refer to the same manager.
		 */
		[[nodiscard]] bool operator==(const Manager& other) const noexcept;

		/**
		 * @brief Order two managers.
		 * @param other The manager to compare against.
		 * @return The relative ordering of the two managers.
		 */
		[[nodiscard]] auto operator<=>(const Manager& other) const noexcept;

	private:
		struct Impl;  ///< Implementation details, hidden from consumers.
		PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};
}
