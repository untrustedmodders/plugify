#if !PLUGIFY_PLATFORM_WINDOWS && PLUGIFY_DOWNLOADER

#include "http_downloader_curl.hpp"

#include <curl/curl.h>
#include <csignal>

using namespace plugify;

HTTPDownloaderCurl::HTTPDownloaderCurl() : IHTTPDownloader() {}

HTTPDownloaderCurl::~HTTPDownloaderCurl() {
	if (_multiHandle)
		curl_multi_cleanup(_multiHandle);
}

std::unique_ptr<IHTTPDownloader> IHTTPDownloader::Create(std::string_view userAgent) {
	auto instance = std::make_unique<HTTPDownloaderCurl>();
	if (!instance->Initialize(userAgent))
		return {};
	return instance;
}

static bool curl_initialized = false;
static std::once_flag curl_initialized_once_flag;

bool HTTPDownloaderCurl::Initialize(std::string_view userAgent) {
	if (!curl_initialized) {
		std::call_once(curl_initialized_once_flag, []() {
			curl_initialized = curl_global_init(CURL_GLOBAL_ALL) == CURLE_OK;
			if (curl_initialized) {
				std::atexit([]() {
					curl_global_cleanup();
					curl_initialized = false;
				});
			}
		});
		if (!curl_initialized) {
			PL_LOG_ERROR("curl_global_init() failed");
			return false;
		}
	}

	_multiHandle = curl_multi_init();
	if (!_multiHandle) {
		PL_LOG_ERROR("curl_multi_init() failed");
		return false;
	}

	_userAgent = userAgent;
	return true;
}

size_t HTTPDownloaderCurl::WriteCallback(char* ptr, size_t size, size_t nmemb, void* userdata) {
	auto req = static_cast<Request*>(userdata);
	const size_t currentSize = req->data.size();
	const size_t transferSize = size * nmemb;
	const size_t newSize = currentSize + transferSize;
	req->data.resize(newSize);
	req->startTime = DateTime::Now();
	std::memcpy(&req->data[currentSize], ptr, transferSize);

	if (req->contentLength == 0) {
		curl_off_t length;
		if (curl_easy_getinfo(req->handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD_T, &length) == CURLE_OK)
			req->contentLength = static_cast<uint32_t>(length);
	}

	return nmemb;
}

IHTTPDownloader::Request* HTTPDownloaderCurl::InternalCreateRequest() {
	Request* req = new Request();
	req->handle = curl_easy_init();
	if (!req->handle) {
		delete req;
		return nullptr;
	}

	return req;
}

void HTTPDownloaderCurl::InternalPollRequests() {
	// OpenSSL can fire SIGPIPE...
	sigset_t old_block_mask = {};
	sigset_t new_block_mask = {};
	sigemptyset(&old_block_mask);
	sigemptyset(&new_block_mask);
	sigaddset(&new_block_mask, SIGPIPE);
	if (pthread_sigmask(SIG_BLOCK, &new_block_mask, &old_block_mask) != 0)
		PL_LOG_WARNING("Failed to block SIGPIPE");

	int runningHandles;
	const CURLMcode err = curl_multi_perform(_multiHandle, &runningHandles);
	if (err != CURLM_OK)
		PL_LOG_ERROR("curl_multi_perform() returned {}", curl_multi_strerror(err));

	for (;;) {
		int msgq;
		struct CURLMsg* msg = curl_multi_info_read(_multiHandle, &msgq);
		if (!msg)
			break;

		if (msg->msg != CURLMSG_DONE) {
			PL_LOG_WARNING("Unexpected multi message {}", static_cast<int>(msg->msg));
			continue;
		}

		Request* req;
		CURLcode res = curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &req);
		if (res != CURLE_OK) {
			PL_LOG_ERROR("curl_easy_getinfo() failed: {}", curl_easy_strerror(res));
			continue;
		}

		if (msg->data.result == CURLE_OK) {
			long response_code = 0;
			curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &response_code);
			req->statusCode = static_cast<int32_t>(response_code);

			char* content_type = nullptr;
			if (curl_easy_getinfo(req->handle, CURLINFO_CONTENT_TYPE, &content_type) == CURLE_OK && content_type)
				req->contentType = content_type;

			PL_LOG_VERBOSE("Request for '{}' returned status code {} and {} bytes", req->url, req->statusCode, req->data.size());
		} else {
			PL_LOG_ERROR("Request for '{}' returned error {}", req->url, curl_easy_strerror(msg->data.result));
		}

		req->state.store(Request::State::Complete, std::memory_order_release);
	}

	if (pthread_sigmask(SIG_UNBLOCK, &new_block_mask, &old_block_mask) != 0)
		PL_LOG_WARNING("Failed to unblock SIGPIPE");
}

bool HTTPDownloaderCurl::StartRequest(IHTTPDownloader::Request* request) {
	auto req = static_cast<Request*>(request);
	curl_easy_setopt(req->handle, CURLOPT_URL, request->url.c_str());
	curl_easy_setopt(req->handle, CURLOPT_USERAGENT, _userAgent.c_str());
	curl_easy_setopt(req->handle, CURLOPT_WRITEFUNCTION, &WriteCallback);
	curl_easy_setopt(req->handle, CURLOPT_WRITEDATA, req);
	curl_easy_setopt(req->handle, CURLOPT_NOSIGNAL, 1L);
	curl_easy_setopt(req->handle, CURLOPT_PRIVATE, req);
	curl_easy_setopt(req->handle, CURLOPT_FOLLOWLOCATION, 1L);

	if (request->type == Request::Type::Post) {
		curl_easy_setopt(req->handle, CURLOPT_POST, 1L);
		curl_easy_setopt(req->handle, CURLOPT_POSTFIELDS, request->postData.c_str());
	}

	PL_LOG_VERBOSE("Started HTTP request for '{}'", req->url);
	req->state.store(Request::State::Started, std::memory_order_release);
	req->startTime = DateTime::Now();

	const CURLMcode err = curl_multi_add_handle(_multiHandle, req->handle);
	if (err != CURLM_OK) {
		PL_LOG_ERROR("curl_multi_add_handle() returned {}", curl_multi_strerror(err));
		req->callback(HTTP_STATUS_ERROR, {}, req->data);
		curl_easy_cleanup(req->handle);
		delete req;
		return false;
	}

	return true;
}

void HTTPDownloaderCurl::CloseRequest(IHTTPDownloader::Request* request) {
	auto req = static_cast<Request*>(request);
	PL_ASSERT(req->handle);
	curl_multi_remove_handle(_multiHandle, req->handle);
	curl_easy_cleanup(req->handle);
	delete req;
}

#endif // !PLUGIFY_PLATFORM_WINDOWS && PLUGIFY_DOWNLOADER
