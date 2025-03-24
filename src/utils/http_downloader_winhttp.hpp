#pragma once

#include "http_downloader.hpp"
#include "os.h"

#include <winhttp.h>

namespace plugify {
	class HTTPDownloaderWinHttp final : public IHTTPDownloader {
	public:
		HTTPDownloaderWinHttp();
		~HTTPDownloaderWinHttp() override;

		bool Initialize(std::string_view userAgent);

	protected:
		Request* InternalCreateRequest() override;
		void InternalPollRequests() override;
		bool StartRequest(IHTTPDownloader::Request* request) override;
		void CloseRequest(IHTTPDownloader::Request* request) override;

	private:
		struct Request : IHTTPDownloader::Request {
			HINTERNET hConnection{ NULL };
			HINTERNET hRequest{ NULL };
			uint32_t ioPosition{ 0 };
		};

		static void CALLBACK HTTPStatusCallback(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength);

		HINTERNET _hSession{ NULL };
		std::wstring _userAgent;
	};
}
