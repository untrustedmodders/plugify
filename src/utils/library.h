#pragma once

namespace wizard {
    class Library {
    public:
        static std::unique_ptr<Library> LoadFromPath(const std::filesystem::path& libraryPath);
        static std::string GetError();

        ~Library();

        void* GetFunction(const char* functionName) const;
        template<class Func> requires(std::is_pointer_v<Func> && std::is_function_v<std::remove_pointer_t<Func>>)
        Func GetFunction(const char* functionName) const {
            return reinterpret_cast<Func>(GetFunction(functionName));
        }

    private:
        explicit Library(void* handle);

    private:
        void* _handle{ nullptr };
    };
}