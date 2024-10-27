#pragma once

#include "http_downloader.hpp"
#include "os.h"

#include <winhttp.h>

namespace plugify {
	class HTTPDownloaderWinHttp final : public HTTPDownloader {
	public:
		HTTPDownloaderWinHttp();
		~HTTPDownloaderWinHttp() override;

		bool Initialize(std::string userAgent);

	protected:
		Request* InternalCreateRequest() override;
		void InternalPollRequests() override;
		bool StartRequest(HTTPDownloader::Request* request) override;
		void CloseRequest(HTTPDownloader::Request* request) override;

	private:
		struct Request : HTTPDownloader::Request {
			std::wstring objectName;
			HINTERNET hConnection{ NULL };
			HINTERNET hRequest{ NULL };
			uint32_t ioPosition{ 0 };
		};

		static void CALLBACK HTTPStatusCallback(HINTERNET hInternet, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength);

		HINTERNET _hSession{ NULL };
		std::string _userAgent;
	};
}
