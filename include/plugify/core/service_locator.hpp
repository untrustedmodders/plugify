#pragma once

#include <memory>
#include <typeindex>
#include <unordered_map>

namespace plugify {
    class PLUGIFY_API ServiceLocator {
        struct Impl;
    public:
        ServiceLocator();
        ~ServiceLocator();
        ServiceLocator(const ServiceLocator&) = delete;
        ServiceLocator& operator=(const ServiceLocator&) = delete;
        ServiceLocator(ServiceLocator&&) noexcept;
        ServiceLocator& operator=(ServiceLocator&&) noexcept;

        // Template methods must stay in header but delegate to non-template methods
        template<typename Interface, typename Implementation = Interface>
        void Register(std::shared_ptr<Implementation> service)
            requires(std::is_base_of_v<Interface, Implementation>) {
            RegisterInternal(std::type_index(typeid(Interface)), std::move(service));
        }

        template<typename Interface>
        [[nodiscard]] std::shared_ptr<Interface> Get() const {
            return std::static_pointer_cast<Interface>(
                GetInternal(std::type_index(typeid(Interface)))
            );
        }

        template<typename Interface>
        [[nodiscard]] bool Has() const {
            return HasInternal(std::type_index(typeid(Interface)));
        }

        void Clear();

    private:
        // Non-template methods that can be implemented in .cpp
        void RegisterInternal(std::type_index type, std::shared_ptr<void> service);
        [[nodiscard]] std::shared_ptr<void> GetInternal(std::type_index type) const;
        [[nodiscard]] bool HasInternal(std::type_index type) const;

    PLUGIFY_ACCESS:
        PLUGIFY_NO_DLL_EXPORT_WARNING(std::unique_ptr<Impl> _impl;)
    };

    /*template<typename... Services>
    class ServiceContext {
        std::tuple<std::shared_ptr<Services>...> _services;

    public:
        explicit ServiceContext(const ServiceLocator& locator)
            : _services(locator.Get<Services>()...) {}

        template<typename T>
        std::shared_ptr<T> Get() const {
            return std::get<std::shared_ptr<T>>(_services);
        }

        bool IsValid() const {
            return (... && std::get<std::shared_ptr<Services>>(_services));
        }

        template<typename Func>
        auto Apply(Func&& func) const {
            return std::apply(std::forward<Func>(func), _services);
        }
    };*/
}