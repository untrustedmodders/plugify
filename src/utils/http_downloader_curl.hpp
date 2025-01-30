#pragma once

#include "http_downloader.hpp"

extern "C" {
	typedef void CURL;
	typedef void CURLM;
}

namespace plugify {
	class HTTPDownloaderCurl final : public IHTTPDownloader {
	public:
		HTTPDownloaderCurl();
		~HTTPDownloaderCurl() override;

		bool Initialize(std::string userAgent);

	protected:
		Request* InternalCreateRequest() override;
		void InternalPollRequests() override;
		bool StartRequest(IHTTPDownloader::Request* request) override;
		void CloseRequest(IHTTPDownloader::Request* request) override;

	private:
		struct Request : IHTTPDownloader::Request {
			CURL* handle{ nullptr };
		};

		static size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata);

		CURLM* _multiHandle{ nullptr };
		std::string _userAgent;
	};
}
