#pragma once

namespace wizard {
    class Library {
    public:
        explicit Library(const fs::path& libraryPath);
        ~Library();

        void* GetFunction(std::string_view functionName);

    private:
        void* _handle{ nullptr };
    };
}