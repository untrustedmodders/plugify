#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <plugify/api/assembly.hpp>

namespace fs = std::filesystem;

namespace plugify {
	/**
	 * @class Assembly
	 * @brief Represents an assembly (module) within a process.
	 */
	class Assembly final : public IAssembly {
	public:
		/**
		 * @struct Section
		 * @brief Represents a section of the assembly.
		 */
		struct Section {
			/**
			 * @brief Default constructor initializing size to 0.
			 */
			Section() : size{0} {}

			/**
			 * @brief Parameterized constructor.
			 * @param sectionName The name of the section.
			 * @param sectionBase The base address of the section.
			 * @param sectionSize The size of the section.
			 */
			Section(std::string_view sectionName, uintptr_t sectionBase, size_t sectionSize)
				: name(sectionName), base{sectionBase}, size{sectionSize} {}

			/**
			 * @brief Checks if the section is valid.
			 * @return True if the section is valid, false otherwise.
			 */
			operator bool() const noexcept { return base; }

			std::string name{}; //!< The name of the section.
			MemAddr base;       //!< The base address of the section.
			size_t size;        //!< The size of the section.
		};

		/**
		 * @struct Handle
		 * @brief Represents a system handle.
		 */
		struct Handle {
			/**
			 * @brief Constructor to initialize the handle.
			 * @param sysHandle The handle value to initialize with.
			 */
			Handle(void* sysHandle) : handle{sysHandle} {}

			/**
			 * @brief Checks if the handle is valid.
			 * @return True if the handle is valid, false otherwise.
			 */
			operator bool() const noexcept { return handle; }

			/**
			 * @brief Converts the handle to a void pointer.
			 * @return The internal handle as a void pointer.
			 */
			operator void*() const noexcept { return handle; }

			void* handle{}; ///< The system handle.
		};

		/**
		 * @brief Default constructor initializing handle to nullptr.
		 */
		Assembly() : _handle{nullptr} {}

		/**
		 * @brief Destructor.
		 */
		~Assembly() override;

		// Delete copy constructor and copy assignment operator.
		Assembly(const Assembly&) = delete;
		Assembly& operator=(const Assembly&) = delete;
		//Assembly& operator=(const Assembly&) && = delete;

		// Delete move constructor and move assignment operator.
		Assembly(Assembly&& rhs) noexcept = delete;
		Assembly& operator=(Assembly&& rhs) noexcept = delete;
		//Assembly& operator=(Assembly&&) && = delete;

		using SearchDirs = std::vector<fs::path>;

		/**
		 * @brief Constructs an Assembly object with the specified module name, flags, and sections.
		 * @param name The name of the module.
		 * @param flags Optional flags for module initialization.
		 * @param searchDirs Optional additional search directories.
		 * @param sections Optional flag indicating if sections should be initialized.
		 */
		explicit Assembly(std::string_view name, LoadFlag flags = LoadFlag::Default, const SearchDirs& searchDirs = {}, bool sections = false);

		/**
		 * @brief Constructs an Assembly object with a char pointer as module name.
		 * @param name The name of the module as a char pointer.
		 * @param flags Optional flags for module initialization.
		 * @param searchDirs Optional additional search directories.
		 * @param sections Optional flag indicating if sections should be initialized.
		 */
		explicit Assembly(const char* name, LoadFlag flags = LoadFlag::Default, const SearchDirs& searchDirs = {}, bool sections = false)
			: Assembly(std::string_view(name), flags, searchDirs, sections) {}

		/**
		 * @brief Constructs an Assembly object with a string as module name.
		 * @param name The name of the module as a string.
		 * @param flags Optional flags for module initialization.
		 * @param searchDirs Optional additional search directories.
		 * @param sections Optional flag indicating if sections should be initialized.
		 */
		explicit Assembly(const std::string& name, LoadFlag flags = LoadFlag::Default, const SearchDirs& searchDirs = {}, bool sections = false)
			: Assembly(std::string_view(name), flags, searchDirs, sections) {}

		/**
		 * @brief Constructs an Assembly object with a filesystem path as module path.
		 * @param path The filesystem path of the module.
		 * @param flags Optional flags for module initialization.
		 * @param searchDirs Optional additional search directories.
		 * @param sections Optional flag indicating if sections should be initialized.
		 */
		explicit Assembly(const fs::path& path, LoadFlag flags = LoadFlag::Default, const SearchDirs& searchDirs = {}, bool sections = false);

		/**
		 * @brief Constructs an Assembly object with a memory address.
		 * @param memory The memory address of the module.
		 * @param flags Optional flags for module initialization.
		 * @param searchDirs Optional additional search directories.
		 * @param sections Optional flag indicating if sections should be initialized.
		 */
		explicit Assembly(MemAddr memory, LoadFlag flags = LoadFlag::Default, const SearchDirs& searchDirs = {}, bool sections = false);

		/**
		 * @brief Constructs an Assembly object with a memory address.
		 * @param handle The system handle of the module.
		 * @param flags Optional flags for module initialization.
		 * @param searchDirs Optional additional search directories.
		 * @param sections Optional flag indicating if sections should be initialized.
		 */
		explicit Assembly(Handle handle, LoadFlag flags = LoadFlag::Default, const SearchDirs& searchDirs = {}, bool sections = false);

		/**
		 * @brief Converts a string pattern with wildcards to an array of bytes and mask.
		 * @param input The input pattern string.
		 * @return A pair containing the byte array and the mask string.
		 */
		static std::pair<std::vector<uint8_t>, std::string> PatternToMaskedBytes(std::string_view input);

		/**
		 * @brief Finds an array of bytes in process memory using SIMD instructions.
		 * @param pattern The byte pattern to search for.
		 * @param mask The mask corresponding to the byte pattern.
		 * @param startAddress The start address for the search.
		 * @param moduleSection The module section to search within.
		 * @return The memory address where the pattern is found, or nullptr if not found.
		 */
		MemAddr FindPattern(MemAddr pattern, std::string_view mask, MemAddr startAddress = nullptr, const Section * moduleSection = nullptr) const;

		/**
		 * @brief Finds a string pattern in process memory using SIMD instructions.
		 * @param pattern The string pattern to search for.
		 * @param startAddress The start address for the search.
		 * @param moduleSection The module section to search within.
		 * @return The memory address where the pattern is found, or nullptr if not found.
		 */
		MemAddr FindPattern(std::string_view pattern, MemAddr startAddress = nullptr, Section* moduleSection = nullptr) const;

		/**
		 * @brief Gets an address of a virtual method table by RTTI type descriptor name.
		 * @param tableName The name of the virtual table.
		 * @param decorated Indicates whether the name is decorated.
		 * @return The memory address of the virtual table, or nullptr if not found.
		 */
		MemAddr GetVirtualTableByName(std::string_view tableName, bool decorated = false) const;

		/**
		 * @brief Gets an address of a function by its name.
		 * @param functionName The name of the function.
		 * @return The memory address of the function, or nullptr if not found.
		 */
		MemAddr GetFunctionByName(std::string_view functionName) const noexcept;

		/**
		 * @brief Gets a module section by name.
		 * @param sectionName The name of the section (e.g., ".rdata", ".text").
		 * @return The Section object representing the module section.
		 */
		Section GetSectionByName(std::string_view sectionName) const noexcept;

		// IAssembly interface methods

		/**
		 * @brief Checks if the assembly is valid.
		 * @return True if the assembly is valid, false otherwise.
		 */
		bool IsValid() const noexcept;

		/**
		 * @brief Returns the module handle.
		 * @return The module handle.
		 */
		void* GetHandle() const noexcept;

		/**
		 * @brief Returns the module base address.
		 * @return The base address of the module.
		 */
		MemAddr GetBase() const noexcept;

		/**
		 * @brief Returns the module path.
		 * @return The path of the module.
		 */
		fs::path_view GetPath() const noexcept;

		/**
		 * @brief Returns the module error.
		 * @return The error string of the module.
		 */
		std::string_view GetError() const noexcept;

		/**
		 * @brief Retrieves a raw symbol pointer by name.
		 * @param name The name of the symbol to retrieve.
		 * @return The memory address of the symbol, or nullptr if not found.
		 */
		MemAddr GetSymbol(std::string_view name) const override;

		/**
		 * @brief Conversion operator to check if the assembly is valid.
		 * @return True if the assembly is valid, false otherwise.
		 */
		explicit operator bool() const noexcept { return _handle != nullptr; }

		/**
		 * @brief Equality operator.
		 * @param assembly The other Assembly object to compare with.
		 * @return True if both Assembly objects are equal, false otherwise.
		 */
		bool operator==(const Assembly& assembly) const noexcept { return _handle == assembly._handle; }

	private:
		/**
		 * @brief Initializes module descriptors.
		 * @param path The path of the module.
		 * @param flags Flags for module initialization.
		 * @param searchDirs Additional search directories.
		 * @param sections Flag indicating if sections should be initialized.
		 * @return True if initialization was successful, false otherwise.
		 */
		bool Init(const fs::path& path, LoadFlag flags, const SearchDirs& searchDirs, bool sections);

		/**
		 * @brief Initializes the assembly from a module name.
		 * @param name The name of the module.
		 * @param flags Flags for module initialization.
		 * @param searchDirs Additional search directories.
		 * @param sections Flag indicating if sections should be initialized.
		 * @param extension Indicates if an extension is used.
		 * @return True if initialization was successful, false otherwise.
		 */
		bool InitFromName(std::string_view name, LoadFlag flags, const SearchDirs& searchDirs, bool sections, bool extension = false);

		/**
		 * @brief Initializes the assembly from memory.
		 * @param memory The memory address of the module.
		 * @param flags Flags for module initialization.
		 * @param searchDirs Additional search directories.
		 * @param sections Flag indicating if sections should be initialized.
		 * @return True if initialization was successful, false otherwise.
		 */
		bool InitFromMemory(MemAddr memory, LoadFlag flags, const SearchDirs& searchDirs, bool sections);

		/**
		 * @brief Initializes the assembly from handle.
		 * @param handle The system handle of the module.
		 * @param flags Flags for module initialization.
		 * @param searchDirs Additional search directories.
		 * @param sections Flag indicating if sections should be initialized.
		 * @return True if initialization was successful, false otherwise.
		 */
		bool InitFromHandle(Handle handle, LoadFlag flags, const SearchDirs& searchDirs, bool sections);

		/**
		 * @brief Loads the sections of the module into memory.
		 *
		 * This function is responsible for loading the individual sections of
		 * a module into memory. Sections typically represent different parts
		 * of a module, such as executable code, read-only data, or other data
		 * segments. The function ensures that these sections are properly loaded
		 * and ready for execution or inspection.
		 *
		 * @return True if the sections were successfully loaded, false otherwise.
		 */
		bool LoadSections();

		/**
		 * @brief Checks if a module exists at the specified path.
		 * @param path The path to check for the existence of the module.
		 * @return True if the module exists, false otherwise.
		 */
		bool IsExist(const fs::path& path);

	private:
		void* _handle;                          //!< The handle to the module.
		fs::path _path;            //!< The path of the module.
		std::string _error;                     //!< The error of the module.
		//LoadFlag flags{LoadFlag::Default};      //!< Flags for loading the module.
		//bool sections{false};			          //!< Flag indicating if sections should load.
		//SearchDirs searchDirs; //!< Additional search directories for the module.
		//Section _executableCode;                //!< The section representing executable code.
		std::vector<Section> _sections;         //!< A vector of sections in the module.
	};

	/**
	 * @brief Translates loading flags to an integer representation.
	 *
	 * @param flags The loading flags to translate.
	 * @return An integer representation of the loading flags.
	 */
	int TranslateLoading(LoadFlag flags) noexcept;

	/**
	 * @brief Translates an integer representation of loading flags to LoadFlag.
	 *
	 * @param flags The integer representation of the loading flags.
	 * @return The corresponding LoadFlag.
	 */
	LoadFlag TranslateLoading(int flags) noexcept;

} // namespace plugify
