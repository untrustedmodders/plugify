#pragma once

#include <string>
#include <memory>
#include <functional>

namespace plugify {
	using ProgressCallback = std::function<bool(uint32_t bytesDone, uint32_t bytesTotal)>;
	using RequestCallback = std::function<void(int32_t statusCode, std::string_view contentType, std::vector<uint8_t> data)>;

	/**
	 * @brief Downloader interface for handling HTTP requests
	 *
	 * This interface abstracts the downloading of resources over HTTP,
	 * allowing for custom implementations (e.g., using libcurl, WinHTTP).
	 */
	class IDownloader {
	public:
		virtual ~IDownloader() = default;

		virtual void CreateRequest(std::string url, RequestCallback callback, ProgressCallback progress) = 0;
		virtual void CreatePostRequest(std::string url, std::string postData, RequestCallback callback, ProgressCallback progress) = 0;
		virtual void CreateHeadRequest(std::string url, RequestCallback callback) = 0;

		virtual void PollRequests() = 0;
		virtual void WaitForAllRequests() = 0;
		virtual bool HasAnyRequests() = 0;
		virtual void SetTimeout(float timeout) = 0;
		virtual void SetMaxActiveRequests(uint32_t maxActiveRequests) = 0;
	};
} // namespace plugify