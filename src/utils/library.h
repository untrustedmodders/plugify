#pragma once

namespace wizard {
    class Library {
    public:
        static std::unique_ptr<Library> LoadFromPath(const std::filesystem::path& libraryPath);
        static std::string GetError();

        ~Library();

        void* GetFunction(const char* functionName) const;
        template<class _Fn> requires(std::is_pointer_v<_Fn> && std::is_function_v<std::remove_pointer_t<_Fn>>)
        _Fn GetFunction(const char* functionName) const {
            return reinterpret_cast<_Fn>(GetFunction(functionName));
        }

    private:
        explicit Library(void* handle);

    private:
        void* _handle{ nullptr };
    };
}