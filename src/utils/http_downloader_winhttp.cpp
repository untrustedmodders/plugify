#if PLUGIFY_PLATFORM_WINDOWS && PLUGIFY_DOWNLOADER

#include "http_downloader_winhttp.hpp"
#include "strings.hpp"

using namespace plugify;

HTTPDownloaderWinHttp::HTTPDownloaderWinHttp() : IHTTPDownloader() {
}

HTTPDownloaderWinHttp::~HTTPDownloaderWinHttp() {
	if (_hSession) {
		WinHttpSetStatusCallback(_hSession, nullptr, WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS, 0);
		WinHttpCloseHandle(_hSession);
	}
}

std::unique_ptr<IHTTPDownloader> IHTTPDownloader::Create(std::string_view userAgent) {
	auto instance = std::make_unique<HTTPDownloaderWinHttp>();
	if (!instance->Initialize(userAgent))
		return {};
	return instance;
}

static std::string GetErrorMessage() {
	DWORD dwErrorCode = ::GetLastError();
	if (dwErrorCode == 0) {
		return {}; // No error message has been recorded
	}

	LPSTR messageBuffer = NULL;
	const DWORD size = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM  | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ALLOCATE_BUFFER,
									  NULL, // (not used with FORMAT_MESSAGE_FROM_SYSTEM)
									  dwErrorCode,
									  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
									  reinterpret_cast<LPSTR>(&messageBuffer),
									  0,
									  NULL);
	if (!size) {
		return std::format("Unknown error code: {}", dwErrorCode);
	}

	auto deleter = [](void* p) { ::LocalFree(p); };
	std::unique_ptr<char, decltype(deleter)> ptrBuffer(messageBuffer, deleter);
	return { ptrBuffer.get(), size };
}

bool HTTPDownloaderWinHttp::Initialize(std::string_view userAgent) {
	static constexpr DWORD dwAccessType = WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY;

	_userAgent = String::ConvertUtf8ToWide(userAgent);

	_hSession = WinHttpOpen(_userAgent.c_str(), dwAccessType, nullptr, nullptr, WINHTTP_FLAG_ASYNC);
	if (_hSession == NULL) {
		PL_LOG_ERROR("WinHttpOpen() failed: {}", GetErrorMessage());
		return false;
	}

	const DWORD notificationFlags = WINHTTP_CALLBACK_FLAG_ALL_COMPLETIONS | WINHTTP_CALLBACK_FLAG_REQUEST_ERROR | WINHTTP_CALLBACK_FLAG_HANDLES | WINHTTP_CALLBACK_FLAG_SECURE_FAILURE;
	if (WinHttpSetStatusCallback(_hSession, HTTPStatusCallback, notificationFlags, 0) == WINHTTP_INVALID_STATUS_CALLBACK) {
		PL_LOG_ERROR("WinHttpSetStatusCallback() failed: {}", GetErrorMessage());
		return false;
	}

	return true;
}

std::string ClientErrorToString(HRESULT value) {
	switch (value) {
		case ERROR_WINHTTP_OUT_OF_HANDLES: return "Out of handles.";
		case ERROR_WINHTTP_TIMEOUT: return "Timeout.";
		case ERROR_WINHTTP_INTERNAL_ERROR: return "Internal error.";
		case ERROR_WINHTTP_INVALID_URL: return "Invalid URL.";
		case ERROR_WINHTTP_UNRECOGNIZED_SCHEME: return "Unrecognized scheme.";
		case ERROR_WINHTTP_NAME_NOT_RESOLVED: return "Name not resolved.";
		case ERROR_WINHTTP_INVALID_OPTION: return "Invalid option.";
		case ERROR_WINHTTP_OPTION_NOT_SETTABLE: return "Option not settable.";
		case ERROR_WINHTTP_SHUTDOWN: return "Shutdown.";
		case ERROR_WINHTTP_LOGIN_FAILURE: return "Login failure.";
		case ERROR_WINHTTP_OPERATION_CANCELLED: return "Operation cancelled.";
		case ERROR_WINHTTP_INCORRECT_HANDLE_TYPE: return "Incorrect handle type.";
		case ERROR_WINHTTP_INCORRECT_HANDLE_STATE: return "Incorrect handle state.";
		case ERROR_WINHTTP_CANNOT_CONNECT: return "Cannot connect.";
		case ERROR_WINHTTP_CONNECTION_ERROR: return "Connection error.";
		case ERROR_WINHTTP_RESEND_REQUEST: return "Resend request.";
		case ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED: return "The server requires SSL client authentication.";
		case ERROR_WINHTTP_CANNOT_CALL_BEFORE_OPEN: return "Cannot call before open.";
		case ERROR_WINHTTP_CANNOT_CALL_BEFORE_SEND: return "Cannot call before send.";
		case ERROR_WINHTTP_CANNOT_CALL_AFTER_SEND: return "Cannot call after send.";
		case ERROR_WINHTTP_CANNOT_CALL_AFTER_OPEN: return "Cannot call after open.";
		case ERROR_WINHTTP_HEADER_NOT_FOUND: return "Header not found.";
		case ERROR_WINHTTP_INVALID_SERVER_RESPONSE: return "Invalid server response.";
		case ERROR_WINHTTP_INVALID_HEADER: return "Invalid header.";
		case ERROR_WINHTTP_INVALID_QUERY_REQUEST: return "Invalid query request.";
		case ERROR_WINHTTP_HEADER_ALREADY_EXISTS: return "Header already exists.";
		case ERROR_WINHTTP_REDIRECT_FAILED: return "Redirect failed.";
		case ERROR_WINHTTP_AUTO_PROXY_SERVICE_ERROR: return "A proxy for the specified URL cannot be located.";
		case ERROR_WINHTTP_BAD_AUTO_PROXY_SCRIPT: return "An error occurred executing the script code in the Proxy Auto-Configuration (PAC) file.";
		case ERROR_WINHTTP_UNABLE_TO_DOWNLOAD_SCRIPT: return "The PAC file cannot be downloaded.";
		case ERROR_WINHTTP_NOT_INITIALIZED: return "Not initialized.";
		case ERROR_WINHTTP_SECURE_FAILURE: return "SSL certificate error.";
		case ERROR_WINHTTP_SECURE_CERT_DATE_INVALID: return "Certificate not within validity period.";
		case ERROR_WINHTTP_SECURE_CERT_CN_INVALID: return "Certificate CN name mismatch.";
		case ERROR_WINHTTP_SECURE_INVALID_CA: return "Invalid certificate authority.";
		case ERROR_WINHTTP_SECURE_CERT_REV_FAILED: return "Certificate revocation check failed.";
		case ERROR_WINHTTP_SECURE_CHANNEL_ERROR: return "Secure channel error.";
		case ERROR_WINHTTP_SECURE_INVALID_CERT: return "Invalid certificate.";
		case ERROR_WINHTTP_SECURE_CERT_REVOKED: return "Certificate revoked.";
		case ERROR_WINHTTP_SECURE_CERT_WRONG_USAGE: return "Certificate not valid for requested usage.";
		case ERROR_WINHTTP_AUTODETECTION_FAILED: return "Proxy detection failed.";
		case ERROR_WINHTTP_HEADER_COUNT_EXCEEDED: return "Too many headers in response.";
		case ERROR_WINHTTP_HEADER_SIZE_OVERFLOW: return "Header size exceeds limit.";
		case ERROR_WINHTTP_CHUNKED_ENCODING_HEADER_SIZE_OVERFLOW: return "Chunked encoding header overflow.";
		case ERROR_WINHTTP_RESPONSE_DRAIN_OVERFLOW: return "Response size exceeds limit.";
		case ERROR_WINHTTP_CLIENT_CERT_NO_PRIVATE_KEY: return "Client certificate has no private key.";
		case ERROR_WINHTTP_CLIENT_CERT_NO_ACCESS_PRIVATE_KEY: return "No access to private key of client certificate.";
		case ERROR_WINHTTP_CLIENT_AUTH_CERT_NEEDED_PROXY: return "Client authentication certificate needed for proxy.";
		case ERROR_WINHTTP_SECURE_FAILURE_PROXY: return "SSL secure failure with proxy.";
		case ERROR_WINHTTP_RESERVED_189: return "Reserved error code 189.";
		case ERROR_WINHTTP_HTTP_PROTOCOL_MISMATCH: return "HTTP protocol mismatch.";
		case ERROR_WINHTTP_GLOBAL_CALLBACK_FAILED: return "Global callback failed.";
		case ERROR_WINHTTP_FEATURE_DISABLED: return "Requested WinHTTP feature is disabled.";
		case E_UNEXPECTED: return "Unexpected value.";
		default: return "Unknown error 0x" + std::to_string(value);
	}
}

void CALLBACK HTTPDownloaderWinHttp::HTTPStatusCallback(HINTERNET hRequest, DWORD_PTR dwContext, DWORD dwInternetStatus, LPVOID lpvStatusInformation, DWORD dwStatusInformationLength) {
	Request* req = reinterpret_cast<Request*>(dwContext);
	switch (dwInternetStatus) {
		case WINHTTP_CALLBACK_STATUS_HANDLE_CREATED:
			return;

		case WINHTTP_CALLBACK_STATUS_HANDLE_CLOSING: {
			if (!req)
				return;

			PL_ASSERT(hRequest == req->hRequest);

			auto parent = static_cast<HTTPDownloaderWinHttp*>(req->parent);
			std::unique_lock<std::mutex> lock(parent->_pendingRequestLock);
			PL_ASSERT(std::none_of(parent->_pendingRequests.begin(), parent->_pendingRequests.end(), [req](IHTTPDownloader::Request* it) { return it == req; }));

			// we can clean up the connection as well
			PL_ASSERT(req->hConnection != NULL);
			WinHttpCloseHandle(req->hConnection);
			delete req;
			return;
		}

		case WINHTTP_CALLBACK_STATUS_REQUEST_ERROR: {
			const WINHTTP_ASYNC_RESULT* res = reinterpret_cast<const WINHTTP_ASYNC_RESULT*>(lpvStatusInformation);
			PL_LOG_ERROR("WinHttp '{}' async function {} returned error {}", req->url, res->dwResult, ClientErrorToString(res->dwError));
			req->statusCode = HTTP_STATUS_ERROR;
			req->state.store(Request::State::Complete);
			return;
		}

		case WINHTTP_CALLBACK_STATUS_SENDREQUEST_COMPLETE: {
			PL_LOG_VERBOSE("SendRequest complete");
			if (!WinHttpReceiveResponse(hRequest, nullptr)) {
				PL_LOG_ERROR("WinHttpReceiveResponse() failed: {}", GetErrorMessage());
				req->statusCode = HTTP_STATUS_ERROR;
				req->state.store(Request::State::Complete);
			}
			return;
		}

		case WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE: {
			PL_LOG_VERBOSE("Headers available");

			DWORD bufferSize = sizeof(req->statusCode);
			if (!WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
									 WINHTTP_HEADER_NAME_BY_INDEX, &req->statusCode, &bufferSize, WINHTTP_NO_HEADER_INDEX)) {
				PL_LOG_ERROR("WinHttpQueryHeaders() for status code failed: {}", GetErrorMessage());
				req->statusCode = HTTP_STATUS_ERROR;
				req->state.store(Request::State::Complete);
				return;
			}

			bufferSize = sizeof(req->contentLength);
			if (!WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER,
									 WINHTTP_HEADER_NAME_BY_INDEX, &req->contentLength, &bufferSize,
									 WINHTTP_NO_HEADER_INDEX)) {
				if (GetLastError() != ERROR_WINHTTP_HEADER_NOT_FOUND)
					PL_LOG_WARNING("WinHttpQueryHeaders() for content length failed: {}", GetErrorMessage());

				req->contentLength = 0;
			}

			DWORD contentTypeLength = 0;
			if (!WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_TYPE, WINHTTP_HEADER_NAME_BY_INDEX,
									 WINHTTP_NO_OUTPUT_BUFFER, &contentTypeLength, WINHTTP_NO_HEADER_INDEX) &&
				GetLastError() == ERROR_INSUFFICIENT_BUFFER && contentTypeLength >= sizeof(contentTypeLength)) {
				std::wstring contentTypeWstring;
				contentTypeWstring.resize((contentTypeLength / sizeof(wchar_t)) - 1);
				if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_CONTENT_TYPE, WINHTTP_HEADER_NAME_BY_INDEX,
										contentTypeWstring.data(), &contentTypeLength, WINHTTP_NO_HEADER_INDEX)) {
					req->contentType = String::ConvertWideToUtf8(contentTypeWstring);
				}
			}

			PL_LOG_VERBOSE("Status code {}, content-length is {}", req->statusCode, req->contentLength);
			req->data.reserve(req->contentLength);
			req->state = Request::State::Receiving;

			// start reading
			if (!WinHttpQueryDataAvailable(hRequest, nullptr) && GetLastError() != ERROR_IO_PENDING) {
				PL_LOG_ERROR("WinHttpQueryDataAvailable() failed: {}", GetErrorMessage());
				req->statusCode = HTTP_STATUS_ERROR;
				req->state.store(Request::State::Complete);
			}

			return;
		}

		case WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE: {
			DWORD bytesAvailable;
			std::memcpy(&bytesAvailable, lpvStatusInformation, sizeof(bytesAvailable));
			if (bytesAvailable == 0) {
				// end of request
				PL_LOG_VERBOSE("End of request '{}', {} bytes received", req->url, req->data.size());
				req->state.store(Request::State::Complete);
				return;
			}

			// start the transfer
			PL_LOG_VERBOSE("{} bytes available", bytesAvailable);
			req->ioPosition = static_cast<uint32_t>(req->data.size());
			req->data.resize(req->ioPosition + bytesAvailable);
			if (!WinHttpReadData(hRequest, req->data.data() + req->ioPosition, bytesAvailable, nullptr) &&
				GetLastError() != ERROR_IO_PENDING) {
				PL_LOG_ERROR("WinHttpReadData() failed: {}", GetErrorMessage());
				req->statusCode = HTTP_STATUS_ERROR;
				req->state.store(Request::State::Complete);
			}

			return;
		}

		case WINHTTP_CALLBACK_STATUS_READ_COMPLETE: {
			PL_LOG_VERBOSE("Read of {} complete", dwStatusInformationLength);

			const uint32_t newSize = req->ioPosition + dwStatusInformationLength;
			PL_ASSERT(newSize <= req->data.size());
			req->data.resize(newSize);
			req->startTime = DateTime::Now();

			if (!WinHttpQueryDataAvailable(hRequest, nullptr) && GetLastError() != ERROR_IO_PENDING) {
				PL_LOG_ERROR("WinHttpQueryDataAvailable() failed: {}", GetErrorMessage());
				req->statusCode = HTTP_STATUS_ERROR;
				req->state.store(Request::State::Complete);
			}

			return;
		}

		default:
			// unhandled, ignore
			return;
	}
}

plugify::IHTTPDownloader::Request* HTTPDownloaderWinHttp::InternalCreateRequest() {
	Request* req = new Request();
	return req;
}

void HTTPDownloaderWinHttp::InternalPollRequests() {
	// it uses windows's worker threads
}

bool HTTPDownloaderWinHttp::StartRequest(IHTTPDownloader::Request* request) {
	auto req = static_cast<Request*>(request);

	URL_COMPONENTSW uc = {
		.dwStructSize = sizeof(uc),
		.dwSchemeLength = static_cast<DWORD>(-1),
		.dwHostNameLength = static_cast<DWORD>(-1),
		.dwUrlPathLength = static_cast<DWORD>(-1),
		.dwExtraInfoLength = static_cast<DWORD>(-1)
	};

	const std::wstring urlWide = String::ConvertUtf8ToWide(req->url);
	if (!WinHttpCrackUrl(urlWide.c_str(), static_cast<DWORD>(urlWide.size()), 0, &uc)) {
		PL_LOG_ERROR("WinHttpCrackUrl() failed: {}", GetErrorMessage());
		req->callback(HTTP_STATUS_ERROR, {}, req->data);
		delete req;
		return false;
	}

	std::wstring hostName(uc.lpszHostName, uc.dwHostNameLength);
	std::wstring objectName(uc.lpszUrlPath, uc.dwUrlPathLength);

	req->hConnection = WinHttpConnect(_hSession, hostName.c_str(), uc.nPort, 0);
	if (!req->hConnection) {
		PL_LOG_ERROR("Failed to start HTTP request for '{}': {}", req->url, GetErrorMessage());
		req->callback(HTTP_STATUS_ERROR, {}, req->data);
		delete req;
		return false;
	}

	const DWORD requestFlags = uc.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0;
	req->hRequest = WinHttpOpenRequest(req->hConnection, (req->type == IHTTPDownloader::Request::Type::Post) ? L"POST" : L"GET", objectName.c_str(), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, requestFlags);
	if (!req->hRequest) {
		PL_LOG_ERROR("WinHttpOpenRequest() failed: {}", GetErrorMessage());
		WinHttpCloseHandle(req->hConnection);
		return false;
	}

	BOOL result;
	if (req->type == IHTTPDownloader::Request::Type::Post) {
		const std::wstring_view additionalHeaders = L"Content-Type: application/x-www-form-urlencoded\r\n";
		result = WinHttpSendRequest(req->hRequest, additionalHeaders.data(), static_cast<DWORD>(additionalHeaders.size()), req->postData.data(), static_cast<DWORD>(req->postData.size()), static_cast<DWORD>(req->postData.size()), reinterpret_cast<DWORD_PTR>(req));
	} else {
		result = WinHttpSendRequest(req->hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, reinterpret_cast<DWORD_PTR>(req));
	}

	if (!result && GetLastError() != ERROR_IO_PENDING) {
		PL_LOG_ERROR("WinHttpSendRequest() failed: {}", GetErrorMessage());
		req->statusCode = HTTP_STATUS_ERROR;
		req->state.store(Request::State::Complete);
	}

	PL_LOG_VERBOSE("Started HTTP request for '{}'", req->url);
	req->state = Request::State::Started;
	req->startTime = DateTime::Now();
	return true;
}

void HTTPDownloaderWinHttp::CloseRequest(IHTTPDownloader::Request* request) {
	auto req = static_cast<Request*>(request);

	if (req->hRequest != NULL) {
		// req will be freed by the callback.
		// the callback can fire immediately here if there's nothing running async, so don't touch req afterwards
		WinHttpCloseHandle(req->hRequest);
		return;
	}

	if (req->hConnection != NULL)
		WinHttpCloseHandle(req->hConnection);

	delete req;
}

#endif // PLUGIFY_PLATFORM_WINDOWS && PLUGIFY_DOWNLOADER
