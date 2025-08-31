#pragma once

#include "plugify/core/types.hpp"

#include "plugify_export.h"

namespace plugify {
    PLUGIFY_API std::string ToDebugString(UniqueId id) noexcept;
    PLUGIFY_API std::string_view ToShortString(UniqueId id) noexcept; // returns either registered name or ""

    // RAII registrar helper: registers in ctor, unregisters in dtor
    class Registrar {
    public:
        // DebugInfo contains the human readable metadata
        struct DebugInfo {
            std::string name;
            ExtensionType type;
            std::string version;
            std::filesystem::path location;
        };

        Registrar(UniqueId id, DebugInfo info);
        ~Registrar();

        Registrar(const Registrar&) = delete;
        Registrar& operator=(const Registrar&) = delete;
        Registrar(Registrar&&) noexcept;
        Registrar& operator=(Registrar&&) noexcept;

    private:
        UniqueId _id;
    };
}