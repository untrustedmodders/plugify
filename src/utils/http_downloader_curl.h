#pragma once

#include "http_downloader.h"

extern "C" {
	typedef void CURL;
	typedef void CURLM;
}

namespace plugify {
	class HTTPDownloaderCurl final : public HTTPDownloader {
	public:
		HTTPDownloaderCurl();
		~HTTPDownloaderCurl() override;

		bool Initialize(std::string userAgent);

	protected:
		Request* InternalCreateRequest() override;
		void InternalPollRequests() override;
		bool StartRequest(HTTPDownloader::Request* request) override;
		void CloseRequest(HTTPDownloader::Request* request) override;

	private:
		struct Request : HTTPDownloader::Request {
			CURL* handle{ nullptr };
		};

		static size_t WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata);

		CURLM* _multiHandle{ nullptr };
		std::string _userAgent;
	};
}
