#pragma once

#include <memory>
#include <typeindex>
#include <functional>

namespace plugify {
	/**
	 * @enum ServiceLifetime
	 * @brief How long one resolved service instance lives.
	 */
	enum class ServiceLifetime {
		Transient,  ///< A new instance on every resolve.
		Scoped,     ///< One instance per scope, see ServiceLocator::BeginScope.
		Singleton   ///< One instance for the whole container.
	};

	/**
	 * @class ServiceLocator
	 * @brief Dependency injection container, keyed by interface type.
	 *
	 * Services are registered under the interface others resolve them by, either
	 * as a ready instance, as a factory, or as a type the container constructs
	 * itself. The *IfMissing variants only register when nothing claims the
	 * interface yet, which is what lets PlugifyBuilder::WithDefaults fill in the
	 * gaps a caller left.
	 *
	 * The class is a PIMPL wrapper, move-only, and safe to resolve from several
	 * threads.
	 *
	 * @see Plugify::GetServices, Provider::Resolve
	 */
	class PLUGIFY_API ServiceLocator {
	public:
		/**
		 * @brief Construct an empty container.
		 */
		ServiceLocator();

		/**
		 * @brief Destructor.
		 */
		~ServiceLocator();

		/**
		 * @brief Deleted copy constructor; a container is not duplicated.
		 */
		ServiceLocator(const ServiceLocator&) = delete;

		/**
		 * @brief Deleted copy assignment operator; a container is not duplicated.
		 */
		ServiceLocator& operator=(const ServiceLocator&) = delete;

		/**
		 * @brief Move constructor.
		 */
		ServiceLocator(ServiceLocator&&) noexcept;

		/**
		 * @brief Move assignment operator.
		 * @return A reference to this instance after moving.
		 */
		ServiceLocator& operator=(ServiceLocator&&) noexcept;

		// ============================================
		// Registration Methods
		// ============================================

		/**
		 * @brief Register a concrete instance (singleton).
		 * @tparam Interface The interface to register the instance under.
		 * @tparam Implementation The concrete type, which has to derive from @p Interface.
		 * @param instance The instance every resolve hands back.
		 * @note Replaces whatever was registered for @p Interface before.
		 */
		template <typename Interface, typename Implementation = Interface>
		void RegisterInstance(std::shared_ptr<Implementation> instance)
		    requires(std::is_base_of_v<Interface, Implementation>)
		{
			RegisterInstanceInternal(std::type_index(typeid(Interface)), instance);
		}

		/**
		 * @brief Register a factory function.
		 * @tparam Interface The interface to register the factory under.
		 * @param factory Called to produce an instance.
		 * @param lifetime How long one produced instance lives.
		 */
		template <typename Interface>
		void RegisterFactory(
		    std::function<std::shared_ptr<Interface>()> factory,
		    ServiceLifetime lifetime = ServiceLifetime::Transient
		) {
			RegisterFactoryInternal(
			    std::type_index(typeid(Interface)),
			    [factory]() -> std::shared_ptr<void> { return factory(); },
			    lifetime
			);
		}

		/**
		 * @brief Register a type with automatic construction.
		 * @tparam Interface The interface to register the type under.
		 * @tparam Implementation The concrete type, which has to derive from @p Interface and be
		 * default constructible.
		 * @param lifetime How long one constructed instance lives.
		 */
		template <typename Interface, typename Implementation = Interface>
		void RegisterType(ServiceLifetime lifetime = ServiceLifetime::Transient)
		    requires(std::is_base_of_v<Interface, Implementation> && std::is_default_constructible_v<Implementation>)
		{
			RegisterFactory<Interface>([]() { return std::make_shared<Implementation>(); }, lifetime);
		}

		/**
		 * @brief Register with dependency injection.
		 * @tparam Interface The interface to register the type under.
		 * @tparam Implementation The concrete type, which has to derive from @p Interface.
		 * @tparam Dependencies Interfaces the constructor takes, resolved from this container.
		 * @param constructor Called with the resolved dependencies to produce an instance.
		 * @param lifetime How long one produced instance lives.
		 */
		template <typename Interface, typename Implementation, typename... Dependencies>
		void RegisterWithDependencies(
		    std::function<std::shared_ptr<Implementation>(Dependencies...)> constructor,
		    ServiceLifetime lifetime = ServiceLifetime::Transient
		)
		    requires(std::is_base_of_v<Interface, Implementation>)
		{
			RegisterFactory<Interface>(
			    [this, constructor]() -> std::shared_ptr<Interface> {
				    return constructor(Resolve<Dependencies>()...);
			    },
			    lifetime
			);
		}

		/**
		 * @brief Register a concrete instance if missing (singleton).
		 * @tparam Interface The interface to register the instance under.
		 * @tparam Implementation The concrete type, which has to derive from @p Interface.
		 * @param instance The instance every resolve hands back.
		 * @note Does nothing if @p Interface is already registered.
		 */
		template <typename Interface, typename Implementation = Interface>
		void RegisterInstanceIfMissing(std::shared_ptr<Implementation> instance)
		    requires(std::is_base_of_v<Interface, Implementation>)
		{
			auto type = std::type_index(typeid(Interface));
			if (!IsRegisteredInternal(type)) {
				RegisterInstanceInternal(type, instance);
			}
		}

		/**
		 * @brief Register a factory function if missing.
		 * @tparam Interface The interface to register the factory under.
		 * @param factory Called to produce an instance.
		 * @param lifetime How long one produced instance lives.
		 * @note Does nothing if @p Interface is already registered.
		 */
		template <typename Interface>
		void RegisterFactoryIfMissing(
		    std::function<std::shared_ptr<Interface>()> factory,
		    ServiceLifetime lifetime = ServiceLifetime::Transient
		) {
			auto type = std::type_index(typeid(Interface));
			if (!IsRegisteredInternal(type)) {
				RegisterFactoryInternal(
				    type,
				    [factory]() -> std::shared_ptr<void> { return factory(); },
				    lifetime
				);
			}
		}

		/**
		 * @brief Register a type with automatic construction if missing.
		 * @tparam Interface The interface to register the type under.
		 * @tparam Implementation The concrete type, which has to derive from @p Interface and be
		 * default constructible.
		 * @param lifetime How long one constructed instance lives.
		 * @note Does nothing if @p Interface is already registered.
		 */
		template <typename Interface, typename Implementation = Interface>
		void RegisterTypeIfMissing(ServiceLifetime lifetime = ServiceLifetime::Transient)
		    requires(std::is_base_of_v<Interface, Implementation> && std::is_default_constructible_v<Implementation>)
		{
			RegisterFactoryIfMissing<Interface>(
			    []() { return std::make_shared<Implementation>(); },
			    lifetime
			);
		}

		/**
		 * @brief Register with dependency injection if missing.
		 * @tparam Interface The interface to register the type under.
		 * @tparam Implementation The concrete type, which has to derive from @p Interface.
		 * @tparam Dependencies Interfaces the constructor takes, resolved from this container.
		 * @param constructor Called with the resolved dependencies to produce an instance.
		 * @param lifetime How long one produced instance lives.
		 * @note Does nothing if @p Interface is already registered.
		 */
		template <typename Interface, typename Implementation, typename... Dependencies>
		void RegisterWithDependenciesIfMissing(
		    std::function<std::shared_ptr<Implementation>(Dependencies...)> constructor,
		    ServiceLifetime lifetime = ServiceLifetime::Transient
		)
		    requires(std::is_base_of_v<Interface, Implementation>)
		{
			RegisterFactoryIfMissing<Interface>(
			    [this, constructor]() -> std::shared_ptr<Interface> {
				    return constructor(Resolve<Dependencies>()...);
			    },
			    lifetime
			);
		}

		// ============================================
		// Resolution Methods
		// ============================================

		/**
		 * @brief Resolve a service.
		 * @tparam Interface The interface to resolve.
		 * @return The service.
		 * @note Throws std::runtime_error if nothing is registered for @p Interface; use
		 * @ref TryResolve to test instead.
		 */
		template <typename Interface>
		[[nodiscard]] std::shared_ptr<Interface> Resolve() const {
			return std::static_pointer_cast<Interface>(
			    ResolveInternal(std::type_index(typeid(Interface)))
			);
		}

		/**
		 * @brief Try to resolve a service.
		 * @tparam Interface The interface to resolve.
		 * @return The service, or nullptr if nothing is registered for @p Interface.
		 */
		template <typename Interface>
		[[nodiscard]] std::shared_ptr<Interface> TryResolve() const noexcept {
			auto result = TryResolveInternal(std::type_index(typeid(Interface)));
			return result ? std::static_pointer_cast<Interface>(result) : nullptr;
		}

		/**
		 * @brief Check if a service is registered.
		 * @tparam Interface The interface to look for.
		 * @return True if something is registered for @p Interface.
		 */
		template <typename Interface>
		[[nodiscard]] bool IsRegistered() const {
			return IsRegisteredInternal(std::type_index(typeid(Interface)));
		}

		// ============================================
		// Non-template public methods
		// ============================================

		/**
		 * @brief Create a new scope (for scoped services).
		 * @note Scopes nest; prefer the RAII @ref ScopedServiceLocator guard over calling this
		 * directly.
		 */
		void BeginScope();

		/**
		 * @brief End current scope.
		 * @note Throws std::runtime_error when no scope is open.
		 */
		void EndScope();

		/**
		 * @brief Clear all registrations.
		 */
		void Clear();

		/**
		 * @brief Get the number of registered services.
		 * @return How many interfaces have something registered for them.
		 */
		[[nodiscard]] size_t Count() const;

		// ============================================
		// Builder Pattern (Nested class for convenience)
		// ============================================

		/**
		 * @class ServiceLocator::ServiceBuilder
		 * @brief Chainable front end for registering several services at once.
		 *
		 * Each Add* method registers into the container the builder was made from
		 * and returns the builder, so the calls chain. The container has to
		 * outlive the builder.
		 */
		class PLUGIFY_API ServiceBuilder {
		public:
			/**
			 * @brief Construct a builder over a container.
			 * @param locator The container to register into.
			 */
			explicit ServiceBuilder(ServiceLocator& locator);

			/**
			 * @brief Destructor.
			 */
			~ServiceBuilder();

			/**
			 * @brief Register a type as a singleton.
			 * @tparam Interface The interface to register the type under.
			 * @tparam Implementation The concrete type to construct.
			 * @return A reference to this builder for chaining.
			 */
			template <typename Interface, typename Implementation = Interface>
			ServiceBuilder& AddSingleton() {
				_locator.RegisterType<Interface, Implementation>(ServiceLifetime::Singleton);
				return *this;
			}

			/**
			 * @brief Register a ready instance as a singleton.
			 * @tparam Interface The interface to register the instance under.
			 * @param instance The instance every resolve hands back.
			 * @return A reference to this builder for chaining.
			 */
			template <typename Interface>
			ServiceBuilder& AddSingleton(std::shared_ptr<Interface> instance) {
				_locator.RegisterInstance<Interface>(std::move(instance));
				return *this;
			}

			/**
			 * @brief Register a type that is constructed anew on every resolve.
			 * @tparam Interface The interface to register the type under.
			 * @tparam Implementation The concrete type to construct.
			 * @return A reference to this builder for chaining.
			 */
			template <typename Interface, typename Implementation = Interface>
			ServiceBuilder& AddTransient() {
				_locator.RegisterType<Interface, Implementation>(ServiceLifetime::Transient);
				return *this;
			}

			/**
			 * @brief Register a factory that is called on every resolve.
			 * @tparam Interface The interface to register the factory under.
			 * @param factory Called to produce an instance.
			 * @return A reference to this builder for chaining.
			 */
			template <typename Interface>
			ServiceBuilder& AddFactory(std::function<std::shared_ptr<Interface>()> factory) {
				_locator.RegisterFactory<Interface>(std::move(factory));
				return *this;
			}

		private:
			ServiceLocator& _locator;  //!< The container being registered into.
		};

		/**
		 * @brief Start registering services in a chain.
		 * @return A builder over this container.
		 */
		ServiceBuilder Services();

	private:
		// Internal non-template methods (implemented in .cpp)

		/**
		 * @brief Register an instance for a type-erased interface.
		 * @param type The interface the instance is registered under.
		 * @param instance The instance.
		 */
		void RegisterInstanceInternal(std::type_index type, std::shared_ptr<void> instance);

		/**
		 * @brief Register a factory for a type-erased interface.
		 * @param type The interface the factory is registered under.
		 * @param factory Called to produce an instance.
		 * @param lifetime How long one produced instance lives.
		 */
		void RegisterFactoryInternal(
		    std::type_index type,
		    std::function<std::shared_ptr<void>()> factory,
		    ServiceLifetime lifetime
		);

		/**
		 * @brief Resolve a type-erased interface.
		 * @param type The interface to resolve.
		 * @return The service; throws std::runtime_error if there is none.
		 */
		[[nodiscard]] std::shared_ptr<void> ResolveInternal(std::type_index type) const;

		/**
		 * @brief Try to resolve a type-erased interface.
		 * @param type The interface to resolve.
		 * @return The service, or nullptr if there is none.
		 */
		[[nodiscard]] std::shared_ptr<void> TryResolveInternal(std::type_index type) const noexcept;

		/**
		 * @brief Check whether a type-erased interface is registered.
		 * @param type The interface to look for.
		 * @return True if something is registered for it.
		 */
		[[nodiscard]] bool IsRegisteredInternal(std::type_index type) const;

		PLUGIFY_ACCESS : struct Impl;  ///< Implementation details, hidden from consumers.
		PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
	};

	// ============================================
	// RAII Scope Guard
	// ============================================

	/**
	 * @class ScopedServiceLocator
	 * @brief RAII guard that keeps a service scope open for the current scope.
	 *
	 * Calls ServiceLocator::BeginScope on construction and
	 * ServiceLocator::EndScope on destruction, so the two stay balanced even when
	 * the scope is left early. Neither copyable nor movable.
	 *
	 * @see ServiceLifetime::Scoped
	 */
	class PLUGIFY_API ScopedServiceLocator {
	public:
		/**
		 * @brief Open a scope on a container.
		 * @param locator The container to open a scope on; it has to outlive the guard.
		 */
		explicit ScopedServiceLocator(ServiceLocator& locator);

		/**
		 * @brief Close the scope again.
		 */
		~ScopedServiceLocator();

		/**
		 * @brief Deleted copy constructor; a scope is closed exactly once.
		 */
		ScopedServiceLocator(const ScopedServiceLocator&) = delete;

		/**
		 * @brief Deleted copy assignment operator; a scope is closed exactly once.
		 */
		ScopedServiceLocator& operator=(const ScopedServiceLocator&) = delete;

		/**
		 * @brief Deleted move constructor; a scope is closed exactly once.
		 */
		ScopedServiceLocator(ScopedServiceLocator&&) = delete;

		/**
		 * @brief Deleted move assignment operator; a scope is closed exactly once.
		 */
		ScopedServiceLocator& operator=(ScopedServiceLocator&&) = delete;

	private:
		ServiceLocator& _locator;  //!< The container the scope was opened on.
	};
}
