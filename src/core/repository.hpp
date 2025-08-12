#pragma once

#include <plugify/repository.hpp>

namespace plugify {
	/**
	 * @brief Package constraint for flexible version requirements
	 */
	struct PackageRelease {
		PackageVersion version;
		RepositoryInfo download;
		std::optional<std::vector<std::string>> platforms;
		std::optional<std::vector<PackageDependency>> dependencies;
		std::optional<std::vector<PackageConflict>> conflicts;
		std::optional<Security> security;
	};

	/**
	 * @brief Package representation
	 */
	struct PackageInfo {
		std::string name;
		std::string type;
		std::string author;
		std::string licence;
		std::string description;
		std::vector<PackageRelease> versions;
		std::optional<std::unordered_map<std::string, std::string>> metadata;
	};

	/**
	 * @brief Database containing packages
	 */
	struct PackageDatabase {
		std::unordered_map<std::string, PackageInfo> content;
	};

	class IHTTPDownloader;

	/**
	 * @brief Local filesystem repository with scanner integration
	 */
	class LocalRepository : public IRepository {
	public:
		LocalRepository(std::string name, fs::path rootPath, int priority = 0)
			: _name{std::move(name)}
			, _rootPath{std::move(rootPath)}
			, _priority{priority} {}

		std::string_view GetName() const override { return _name; }
		int GetPriority() const override { return _priority; }
		Result<bool> IsAvailable() override;
		std::vector<Package> GetPackages() const override;

	private:
		std::string _name;
		fs::path _rootPath;
		int _priority;

		template<typename F>
		static void ScanDirectory(const fs::path& directory, const F& func, int depth);
		//std::optional<PackageManifest> ParseManifestFile(const fs::path& path) const;
	};

	/**
	 * @brief Remote HTTP repository
	 */
	class RemoteRepository : public IRepository {
	public:
		RemoteRepository(std::string name, std::string repositoryUrl, std::shared_ptr<IHTTPDownloader> httpDownloader, int priority = 0)
			: _name{std::move(name)}
			, _repositoryUrl{std::move(repositoryUrl)}
			, _httpDownloader{std::move(httpDownloader)}
			, _priority{priority} {}

		std::string_view GetName() const override { return _repositoryUrl; }
		int GetPriority() const override { return _priority; }
		Result<bool> IsAvailable() override;
		std::vector<Package> GetPackages() const override;

	private:
		std::string _name;
		std::string _repositoryUrl;
		std::shared_ptr<IHTTPDownloader> _httpDownloader;
		int _priority;

		//Result<void> FetchRepositoryIndex();
		//std::optional<PackageManifest> FetchManifest(std::string_view packageId) const;
	};
} // nnamespace plugify