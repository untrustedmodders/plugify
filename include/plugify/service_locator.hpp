#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>

namespace plugify {
    /**
     * @brief Service lifetime scope
     */
    enum class ServiceLifetime {
        Transient,  // New instance each time
        Scoped,     // One instance per scope
        Singleton   // One instance globally
    };

    /**
     * @brief DI Container / Service Locator with PIMPL
     */
    class PLUGIFY_API ServiceLocator {
        struct Impl;
    public:
        ServiceLocator();
        ~ServiceLocator();

        // Delete copy operations
        ServiceLocator(const ServiceLocator&) = delete;
        ServiceLocator& operator=(const ServiceLocator&) = delete;

        // Move operations
        ServiceLocator(ServiceLocator&&) noexcept;
        ServiceLocator& operator=(ServiceLocator&&) noexcept;

        // ============================================
        // Registration Methods
        // ============================================

        /**
         * @brief Register a concrete instance (singleton)
         */
        template<typename Interface, typename Implementation = Interface>
        void RegisterInstance(std::shared_ptr<Implementation> instance)
            requires(std::is_base_of_v<Interface, Implementation>) {
            RegisterInstanceInternal(
                std::type_index(typeid(Interface)),
                instance
            );
        }

        /**
         * @brief Register a factory function
         */
        template<typename Interface>
        void RegisterFactory(
            std::function<std::shared_ptr<Interface>()> factory,
            ServiceLifetime lifetime = ServiceLifetime::Transient) {
            RegisterFactoryInternal(
                std::type_index(typeid(Interface)),
                [factory]() -> std::shared_ptr<void> {
                    return factory();
                },
                lifetime
            );
        }

        /**
         * @brief Register a type with automatic construction
         */
        template<typename Interface, typename Implementation = Interface>
        void RegisterType(ServiceLifetime lifetime = ServiceLifetime::Transient)
            requires(std::is_base_of_v<Interface, Implementation> &&
                    std::is_default_constructible_v<Implementation>) {
            RegisterFactory<Interface>(
                []() { return std::make_shared<Implementation>(); },
                lifetime
            );
        }

        /**
         * @brief Register with dependency injection
         */
        template<typename Interface, typename Implementation, typename... Dependencies>
        void RegisterWithDependencies(
            std::function<std::shared_ptr<Implementation>(Dependencies...)> constructor,
            ServiceLifetime lifetime = ServiceLifetime::Transient)
            requires(std::is_base_of_v<Interface, Implementation>) {
            RegisterFactory<Interface>(
                [this, constructor]() -> std::shared_ptr<Interface> {
                    return constructor(Resolve<Dependencies>()...);
                },
                lifetime
            );
        }

        /**
         * @brief Register a concrete instance if missing (singleton)
         */
        template<typename Interface, typename Implementation = Interface>
        void RegisterInstanceIfMissing(std::shared_ptr<Implementation> instance)
            requires(std::is_base_of_v<Interface, Implementation>) {
            auto type = std::type_index(typeid(Interface));
            if (!IsRegisteredInternal(type)) {
                RegisterInstanceInternal(
                type,
                instance
                );
            }
        }

        /**
         * @brief Register a factory function
         */
        template<typename Interface>
        void RegisterFactoryIfMissing(
            std::function<std::shared_ptr<Interface>()> factory,
            ServiceLifetime lifetime = ServiceLifetime::Transient
        ) {
            auto type = std::type_index(typeid(Interface));
            if (!IsRegisteredInternal(type)) {
                RegisterFactoryInternal(
                    type,
                    [factory]() -> std::shared_ptr<void> {
                        return factory();
                    },
                    lifetime
                );
            }
        }

        /**
         * @brief Register a type with automatic construction
         */
        template<typename Interface, typename Implementation = Interface>
        void RegisterTypeIfMissing(ServiceLifetime lifetime = ServiceLifetime::Transient)
            requires(std::is_base_of_v<Interface, Implementation> &&
                    std::is_default_constructible_v<Implementation>) {
            RegisterFactoryIfMissing<Interface>(
                []() { return std::make_shared<Implementation>(); },
                lifetime
            );
        }

        /**
         * @brief Register with dependency injection
         */
        template<typename Interface, typename Implementation, typename... Dependencies>
        void RegisterWithDependenciesIfMissing(
            std::function<std::shared_ptr<Implementation>(Dependencies...)> constructor,
            ServiceLifetime lifetime = ServiceLifetime::Transient)
            requires(std::is_base_of_v<Interface, Implementation>) {
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
         * @brief Resolve a service
         */
        template<typename Interface>
        [[nodiscard]] std::shared_ptr<Interface> Resolve() const {
            return std::static_pointer_cast<Interface>(
                ResolveInternal(std::type_index(typeid(Interface)))
            );
        }

        /**
         * @brief Try to resolve a service (returns nullptr if not found)
         */
        template<typename Interface>
        [[nodiscard]] std::shared_ptr<Interface> TryResolve() const noexcept {
            auto result = TryResolveInternal(std::type_index(typeid(Interface)));
            return result ? std::static_pointer_cast<Interface>(result) : nullptr;
        }

        /**
         * @brief Check if a service is registered
         */
        template<typename Interface>
        [[nodiscard]] bool IsRegistered() const {
            return IsRegisteredInternal(std::type_index(typeid(Interface)));
        }

        // ============================================
        // Non-template public methods
        // ============================================

        /**
         * @brief Create a new scope (for scoped services)
         */
        void BeginScope();

        /**
         * @brief End current scope
         */
        void EndScope();

        /**
         * @brief Clear all registrations
         */
        void Clear();

        /**
         * @brief Get the number of registered services
         */
        [[nodiscard]] size_t Count() const;

        // ============================================
        // Builder Pattern (Nested class for convenience)
        // ============================================
        class PLUGIFY_API ServiceBuilder {
        public:
            explicit ServiceBuilder(ServiceLocator& locator);
            ~ServiceBuilder();

            template<typename Interface, typename Implementation = Interface>
            ServiceBuilder& AddSingleton() {
                _locator.RegisterType<Interface, Implementation>(ServiceLifetime::Singleton);
                return *this;
            }

            template<typename Interface>
            ServiceBuilder& AddSingleton(std::shared_ptr<Interface> instance) {
                _locator.RegisterInstance<Interface>(std::move(instance));
                return *this;
            }

            template<typename Interface, typename Implementation = Interface>
            ServiceBuilder& AddTransient() {
                _locator.RegisterType<Interface, Implementation>(ServiceLifetime::Transient);
                return *this;
            }

            template<typename Interface>
            ServiceBuilder& AddFactory(std::function<std::shared_ptr<Interface>()> factory) {
                _locator.RegisterFactory<Interface>(std::move(factory));
                return *this;
            }

        private:
            ServiceLocator& _locator;
        };

        ServiceBuilder Services();

    private:
        // Internal non-template methods (implemented in .cpp)
        void RegisterInstanceInternal(std::type_index type, std::shared_ptr<void> instance);
        void RegisterFactoryInternal(std::type_index type,
                                    std::function<std::shared_ptr<void>()> factory,
                                    ServiceLifetime lifetime);
        [[nodiscard]] std::shared_ptr<void> ResolveInternal(std::type_index type) const;
        [[nodiscard]] std::shared_ptr<void> TryResolveInternal(std::type_index type) const noexcept;
        [[nodiscard]] bool IsRegisteredInternal(std::type_index type) const;

        // PIMPL
    PLUGIFY_ACCESS:
        PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
    };

    // ============================================
    // RAII Scope Guard
    // ============================================
    class PLUGIFY_API ScopedServiceLocator {
    public:
        explicit ScopedServiceLocator(ServiceLocator& locator);
        ~ScopedServiceLocator();

        // Delete copy/move
        ScopedServiceLocator(const ScopedServiceLocator&) = delete;
        ScopedServiceLocator& operator=(const ScopedServiceLocator&) = delete;
        ScopedServiceLocator(ScopedServiceLocator&&) = delete;
        ScopedServiceLocator& operator=(ScopedServiceLocator&&) = delete;

    private:
        ServiceLocator& _locator;
    };
}
