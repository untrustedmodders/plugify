#if PLUGIFY_DOWNLOADER

#include "http_downloader.hpp"

#include "hash.hpp"
#include "strings.hpp"

#include <thread>

using namespace plugify;

static constexpr float DEFAULT_TIMEOUT_IN_SECONDS = 30;
static constexpr uint32_t DEFAULT_MAX_ACTIVE_REQUESTS = 4;

IHTTPDownloader::IHTTPDownloader() : _timeout{DEFAULT_TIMEOUT_IN_SECONDS}, _maxActiveRequests{DEFAULT_MAX_ACTIVE_REQUESTS} {}

IHTTPDownloader::~IHTTPDownloader() = default;

void IHTTPDownloader::CreateRequest(std::string url, Request::Callback callback, ProgressCallback progress) {
	Request* req = InternalCreateRequest();
	req->parent = this;
	req->type = Request::Type::Get;
	req->url = std::move(url);
	req->callback = std::move(callback);
	req->progress = std::move(progress);
	req->startTime = DateTime::Now();

	std::unique_lock<std::mutex> lock(_pendingRequestLock);
	if (LockedGetActiveRequestCount() < _maxActiveRequests) {
		if (!StartRequest(req))
			return;
	}

	LockedAddRequest(req);
}

void IHTTPDownloader::CreatePostRequest(std::string url, std::string postData, Request::Callback callback, ProgressCallback progress) {
	Request* req = InternalCreateRequest();
	req->parent = this;
	req->type = Request::Type::Post;
	req->url = std::move(url);
	req->postData = std::move(postData);
	req->callback = std::move(callback);
	req->progress = std::move(progress);
	req->startTime = DateTime::Now();

	std::unique_lock<std::mutex> lock(_pendingRequestLock);
	if (LockedGetActiveRequestCount() < _maxActiveRequests) {
		if (!StartRequest(req))
			return;
	}

	LockedAddRequest(req);
}

void IHTTPDownloader::LockedPollRequests(std::unique_lock<std::mutex>& lock) {
	if (_pendingRequests.empty())
		return;

	InternalPollRequests();

	auto currentTime = DateTime::Now();
	uint32_t activeRequests = 0;
	uint32_t unstartedRequests = 0;

	for (size_t index = 0; index < _pendingRequests.size();) {
		Request* req = _pendingRequests[index];
		if (req->state == Request::State::Pending) {
			unstartedRequests++;
			index++;
			continue;
		}

		bool alive = (req->state == Request::State::Started || req->state == Request::State::Receiving);
		if (alive) {
			if (currentTime >= req->startTime && (currentTime - req->startTime).AsSeconds() >= _timeout) {
				PL_LOG_ERROR("Request for '{}' timed out", req->url);

				req->state.store(Request::State::Cancelled);
				_pendingRequests.erase(_pendingRequests.begin() + static_cast<ptrdiff_t>(index));

				// run callback with lock unheld
				lock.unlock();
				req->callback(HTTP_STATUS_TIMEOUT, {}, {});
				CloseRequest(req);
				lock.lock();
				continue;
			} else if (req->progress && !req->progress(req->lastProgressUpdate, req->contentLength)) {
				PL_LOG_ERROR("Request for '{}' cancelled", req->url);

				req->state.store(Request::State::Cancelled);
				_pendingRequests.erase(_pendingRequests.begin() + static_cast<ptrdiff_t>(index));

				// run callback with lock unheld
				lock.unlock();
				req->callback(HTTP_STATUS_CANCELLED, {}, {});
				CloseRequest(req);
				lock.lock();
				continue;
			}
		}

		if (req->state != Request::State::Complete) {
			req->lastProgressUpdate = static_cast<uint32_t>(req->data.size());
			activeRequests++;
			index++;
			continue;
		}

		PL_LOG_VERBOSE("Request for '{}' complete, returned status code {} and {} bytes", req->url, req->statusCode, req->data.size());
		_pendingRequests.erase(_pendingRequests.begin() + static_cast<ptrdiff_t>(index));

		// run callback with lock unheld
		lock.unlock();
		req->callback(req->statusCode, req->contentType, std::move(req->data));
		CloseRequest(req);
		lock.lock();
	}

	// start new requests when we finished some
	if (unstartedRequests > 0 && activeRequests < _maxActiveRequests) {
		for (size_t index = 0; index < _pendingRequests.size();) {
			Request* req = _pendingRequests[index];
			if (req->state != Request::State::Pending) {
				index++;
				continue;
			}

			if (!StartRequest(req)) {
				_pendingRequests.erase(_pendingRequests.begin() + static_cast<ptrdiff_t>(index));
				continue;
			}

			activeRequests++;
			index++;

			if (activeRequests >= _maxActiveRequests)
				break;
		}
	}
}

void IHTTPDownloader::PollRequests() {
	std::unique_lock<std::mutex> lock(_pendingRequestLock);
	LockedPollRequests(lock);
}

void IHTTPDownloader::WaitForAllRequests() {
	std::unique_lock<std::mutex> lock(_pendingRequestLock);
	while (!_pendingRequests.empty()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		LockedPollRequests(lock);
	}
}

void IHTTPDownloader::LockedAddRequest(Request* request) {
	_pendingRequests.push_back(request);
}

uint32_t IHTTPDownloader::LockedGetActiveRequestCount() {
	uint32_t count = 0;
	for (Request* req : _pendingRequests) {
		if (req->state == Request::State::Started || req->state == Request::State::Receiving)
			count++;
	}
	return count;
}

bool IHTTPDownloader::HasAnyRequests() {
	std::unique_lock<std::mutex> lock(_pendingRequestLock);
	return !_pendingRequests.empty();
}

std::string_view IHTTPDownloader::GetExtensionForContentType(std::string_view contentType) {
	static const std::unordered_map<std::string_view, std::string_view, case_insensitive_hash, case_insensitive_equal> contentTypeToExt = {
		{"audio/aac", ".aac"},
		{"application/x-abiword", ".abw"},
		{"application/x-freearc", ".arc"},
		{"image/avif", ".avif"},
		{"video/x-msvideo", ".avi"},
		{"application/vnd.amazon.ebook", ".azw"},
		{"application/octet-stream", ".bin"},
		{"image/bmp", ".bmp"},
		{"application/x-bzip", ".bz"},
		{"application/x-bzip2", ".bz2"},
		{"application/x-cdf", ".cda"},
		{"application/x-csh", ".csh"},
		{"text/css", ".css"},
		{"text/csv", ".csv"},
		{"application/msword", ".doc"},
		{"application/vnd.openxmlformats-officedocument.wordprocessingml.document", ".docx"},
		{"application/vnd.ms-fontobject", ".eot"},
		{"application/epub+zip", ".epub"},
		{"application/gzip", ".gz"},
		{"image/gif", ".gif"},
		{"text/html", ".htm"},
		{"image/vnd.microsoft.icon", ".ico"},
		{"text/calendar", ".ics"},
		{"application/java-archive", ".jar"},
		{"image/jpeg", ".jpg"},
		{"text/javascript", ".js"},
		{"application/json", ".json"},
		{"application/ld+json", ".jsonld"},
		{"audio/midi audio/x-midi", ".mid"},
		{"text/javascript", ".mjs"},
		{"audio/mpeg", ".mp3"},
		{"video/mp4", ".mp4"},
		{"video/mpeg", ".mpeg"},
		{"application/vnd.apple.installer+xml", ".mpkg"},
		{"application/vnd.oasis.opendocument.presentation", ".odp"},
		{"application/vnd.oasis.opendocument.spreadsheet", ".ods"},
		{"application/vnd.oasis.opendocument.text", ".odt"},
		{"audio/ogg", ".oga"},
		{"video/ogg", ".ogv"},
		{"application/ogg", ".ogx"},
		{"audio/opus", ".opus"},
		{"font/otf", ".otf"},
		{"image/png", ".png"},
		{"application/pdf", ".pdf"},
		{"application/x-httpd-php", ".php"},
		{"application/vnd.ms-powerpoint", ".ppt"},
		{"application/vnd.openxmlformats-officedocument.presentationml.presentation", ".pptx"},
		{"application/vnd.rar", ".rar"},
		{"application/rtf", ".rtf"},
		{"application/x-sh", ".sh"},
		{"image/svg+xml", ".svg"},
		{"application/x-tar", ".tar"},
		{"image/tiff", ".tif"},
		{"video/mp2t", ".ts"},
		{"font/ttf", ".ttf"},
		{"text/plain", ".txt"},
		{"application/vnd.visio", ".vsd"},
		{"audio/wav", ".wav"},
		{"audio/webm", ".weba"},
		{"video/webm", ".webm"},
		{"image/webp", ".webp"},
		{"font/woff", ".woff"},
		{"font/woff2", ".woff2"},
		{"application/xhtml+xml", ".xhtml"},
		{"application/vnd.ms-excel", ".xls"},
		{"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", ".xlsx"},
		{"application/xml", ".xml"},
		{"text/xml", ".xml"},
		{"application/vnd.mozilla.xul+xml", ".xul"},
		{"application/zip", ".zip"},
		{"video/3gpp", ".3gp"},
		{"audio/3gpp", ".3gp"},
		{"video/3gpp2", ".3g2"},
		{"audio/3gpp2", ".3g2"},
		{"application/x-7z-compressed", ".7z"},
	};

	if (auto it = contentTypeToExt.find(contentType); it != contentTypeToExt.end()) {
		return std::get<std::string_view>(*it);
	}

	return {};
}

#endif // PLUGIFY_DOWNLOADER
