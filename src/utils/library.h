#pragma once

namespace plugify {
    class Library {
    public:
        static std::unique_ptr<Library> LoadFromPath(fs::path libraryPath);
        static std::string GetError();

        ~Library();

        void* GetFunction(std::string_view functionName) const;
        template<class Func> requires(std::is_pointer_v<Func> && std::is_function_v<std::remove_pointer_t<Func>>)
        Func GetFunction(std::string_view functionName) const {
            return reinterpret_cast<Func>(GetFunction(functionName));
        }

    private:
        explicit Library(void* handle);

	public:
		Library(Library&& rhs) noexcept;
		Library& operator=(Library&& rhs) noexcept;

	private:
		void* _handle{ nullptr };
	};
}