#pragma once

#include "plugify/core/config.hpp"
#include "plugify/core/config_provider.hpp"
#include "plugify/core/event_bus.hpp"
#include "plugify/core/file_system.hpp"
#include "plugify/core/logger.hpp"
#include "plugify/core/service_locator.hpp"

#include "plugify_export.h"

namespace plugify {
    class Context {
    public:
        Context(ServiceLocator services, Config config)
            : _services(std::move(services)), _config(std::move(config)) {}

        //[[nodiscard]] const ServiceLocator& GetServices() const noexcept {  return _services; }
        [[nodiscard]] const Config& GetConfig() const noexcept { return _config; }

        // Service access with type safety
        template<typename T>
        [[nodiscard]] std::shared_ptr<T> GetService() const {
            return _services.Get<T>();
        }

        // Convenience getters
        /*[[nodiscard]] std::shared_ptr<ILogger> GetLogger() const {
            return GetService<ILogger>();
        }

        [[nodiscard]] std::shared_ptr<IFileSystem> GetFileSystem() const {
            return GetService<IFileSystem>();
        }

        [[nodiscard]] std::shared_ptr<IEventBus> GetEventBus() const {
            return GetService<IEventBus>();
        }

        [[nodiscard]] std::shared_ptr<IConfigProvider> GetConfigProvider() const {
            return GetService<IConfigProvider>();
        }*/


    private:
        ServiceLocator _services;
        Config _config;
    };
}
