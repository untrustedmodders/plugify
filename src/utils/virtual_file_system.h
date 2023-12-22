#pragma once

namespace wizard {
    using FileHandler = std::function<void(std::span<const uint8_t>)>;

    class WIZARD_API VirtualFileSystem {
    public:
        VirtualFileSystem() = delete;

        static void Initialize();
        static void Shutdown();

        /**
         * Add an archive or directory to the search path.
         * @param path The path to the directory or archive.
         */
        static void Mount(const fs::path& path, const fs::path& mount);

        /**
         * Add an archive, contained in a PHYSFE_File handle, to the search path.
         * @param path The path to the directory or archive.
         */
        static void Unmount(const fs::path& path);

        /**
         * Reads a file found by real or partial path with a lambda.
         * @param filepath The path to read.
         * @param handler The lambda with data read from the file.
         */
        static void ReadBytes(const fs::path& filepath, const FileHandler& handler);

        /**
         * Opens a text file, reads all the text in the file into a string, and then closes the file.
         * @param filepath The path to read.
         * @return The data read from the file.
         */
        static std::string ReadText(const fs::path& filepath);

        /**
         * Gets if the path is found in one of the search paths.
         * @param filepath The path to look for.
         * @return If the path is found in one of the searches.
         */
        static bool IsExists(const fs::path& filepath);

        /**
         * Checks that file is a directory.
         * @param filepath The path to the file.
         * @return True if path has a directory.
         */
        static bool IsDirectory(const fs::path& filepath);

        /**
         * Finds all the files in a path.
         * @param filepath The path to search.
         * @param recursive If paths will be recursively searched.
         * @param ext The extension string.
         * @return The files found.
         */
        static std::vector<fs::path> GetFiles(const fs::path& filepath, bool recursive = false, std::string_view ext = "");

        /**
         * Get the current search path.
         */
        static std::vector<fs::path> GetSearchPaths();
    };
}