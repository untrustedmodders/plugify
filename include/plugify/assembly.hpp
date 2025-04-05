#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <plugify/load_flag.hpp>
#include <plugify/mem_addr.hpp>
#include <plugify_export.h>

namespace plugify {
	/**
	 * @class Assembly
	 * @brief Represents an assembly (module) within a process.
	 */
	class Assembly {
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
			 * @param systemHandle The handle value to initialize with.
			 */
			Handle(void* systemHandle) : handle{systemHandle} {}

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
		~Assembly();

		// Delete copy constructor and copy assignment operator.
		Assembly(const Assembly&) = delete;
		Assembly& operator=(const Assembly&) = delete;
		//Assembly& operator=(const Assembly&) && = delete;

		// Delete move constructor and move assignment operator.
		Assembly(Assembly&& rhs) noexcept = delete;
		Assembly& operator=(Assembly&& rhs) noexcept = delete;
		//Assembly& operator=(Assembly&&) && = delete;

		using SearchDirs = std::vector<std::filesystem::path>;

		/**
		 * @brief Constructs an Assembly object with the specified module name, flags, and sections.
		 * @param moduleName The name of the module.
		 * @param flags Optional flags for module initialization.
		 * @param additionalSearchDirectories Optional additional search directories.
		 * @param sections Optional flag indicating if sections should be initialized.
		 */
		explicit Assembly(std::string_view moduleName, LoadFlag flags = LoadFlag::Default, const SearchDirs& additionalSearchDirectories = {}, bool sections = false);

		/**
		 * @brief Constructs an Assembly object with a char pointer as module name.
		 * @param moduleName The name of the module as a char pointer.
		 * @param flags Optional flags for module initialization.
		 * @param additionalSearchDirectories Optional additional search directories.
		 * @param sections Optional flag indicating if sections should be initialized.
		 */
		explicit Assembly(const char* moduleName, LoadFlag flags = LoadFlag::Default, const SearchDirs& additionalSearchDirectories = {}, bool sections = false)
			: Assembly(std::string_view(moduleName), flags, additionalSearchDirectories, sections) {}

		/**
		 * @brief Constructs an Assembly object with a string as module name.
		 * @param moduleName The name of the module as a string.
		 * @param flags Optional flags for module initialization.
		 * @param additionalSearchDirectories Optional additional search directories.
		 * @param sections Optional flag indicating if sections should be initialized.
		 */
		explicit Assembly(const std::string& moduleName, LoadFlag flags = LoadFlag::Default, const SearchDirs& additionalSearchDirectories = {}, bool sections = false)
			: Assembly(std::string_view(moduleName), flags, additionalSearchDirectories, sections) {}

		/**
		 * @brief Constructs an Assembly object with a filesystem path as module path.
		 * @param modulePath The filesystem path of the module.
		 * @param flags Optional flags for module initialization.
		 * @param additionalSearchDirectories Optional additional search directories.
		 * @param sections Optional flag indicating if sections should be initialized.
		 */
		explicit Assembly(const std::filesystem::path& modulePath, LoadFlag flags = LoadFlag::Default, const SearchDirs& additionalSearchDirectories = {}, bool sections = false);

		/**
		 * @brief Constructs an Assembly object with a memory address.
		 * @param moduleMemory The memory address of the module.
		 * @param flags Optional flags for module initialization.
		 * @param additionalSearchDirectories Optional additional search directories.
		 * @param sections Optional flag indicating if sections should be initialized.
		 */
		explicit Assembly(MemAddr moduleMemory, LoadFlag flags = LoadFlag::Default, const SearchDirs& additionalSearchDirectories = {}, bool sections = false);

		/**
		 * @brief Constructs an Assembly object with a memory address.
		 * @param moduleHandle The system handle of the module.
		 * @param flags Optional flags for module initialization.
		 * @param additionalSearchDirectories Optional additional search directories.
		 * @param sections Optional flag indicating if sections should be initialized.
		 */
		explicit Assembly(Handle moduleHandle, LoadFlag flags = LoadFlag::Default, const SearchDirs& additionalSearchDirectories = {}, bool sections = false);

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
		MemAddr FindPattern(MemAddr pattern, std::string_view mask, MemAddr startAddress = nullptr, Section* moduleSection = nullptr) const;

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
		const std::filesystem::path& GetPath() const noexcept;

		/**
		 * @brief Returns the module error.
		 * @return The error string of the module.
		 */
		const std::string& GetError() const noexcept;

		/**
		 * @brief Checks if the assembly is valid.
		 * @return True if the assembly is valid, false otherwise.
		 */
		bool IsValid() const noexcept { return _handle != nullptr; }

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
		 * @param modulePath The path of the module.
		 * @param flags Flags for module initialization.
		 * @param additionalSearchDirectories Additional search directories.
		 * @param sections Flag indicating if sections should be initialized.
		 * @return True if initialization was successful, false otherwise.
		 */
		bool Init(std::filesystem::path modulePath, LoadFlag flags, const SearchDirs& additionalSearchDirectories, bool sections);

		/**
		 * @brief Initializes the assembly from a module name.
		 * @param moduleName The name of the module.
		 * @param flags Flags for module initialization.
		 * @param additionalSearchDirectories Additional search directories.
		 * @param sections Flag indicating if sections should be initialized.
		 * @param extension Indicates if an extension is used.
		 * @return True if initialization was successful, false otherwise.
		 */
		bool InitFromName(std::string_view moduleName, LoadFlag flags, const SearchDirs& additionalSearchDirectories, bool sections, bool extension = false);

		/**
		 * @brief Initializes the assembly from memory.
		 * @param moduleMemory The memory address of the module.
		 * @param flags Flags for module initialization.
		 * @param additionalSearchDirectories Additional search directories.
		 * @param sections Flag indicating if sections should be initialized.
		 * @return True if initialization was successful, false otherwise.
		 */
		bool InitFromMemory(MemAddr moduleMemory, LoadFlag flags, const SearchDirs& additionalSearchDirectories, bool sections);

		/**
		 * @brief Initializes the assembly from handle.
		 * @param moduleHandle The system handle of the module.
		 * @param flags Flags for module initialization.
		 * @param additionalSearchDirectories Additional search directories.
		 * @param sections Flag indicating if sections should be initialized.
		 * @return True if initialization was successful, false otherwise.
		 */
		bool InitFromHandle(Handle moduleHandle, LoadFlag flags, const SearchDirs& additionalSearchDirectories, bool sections);

		bool LoadSections();

	private:
		void* _handle;                //!< The handle to the module.
		std::filesystem::path _path;  //!< The path of the module.
		std::string _error;           //!< The error of the module.
		Section _executableCode;      //!< The section representing executable code.
		std::vector<Section> _sections; //!< A vector of sections in the module.
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
