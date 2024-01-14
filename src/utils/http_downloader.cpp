#include "http_downloader.h"
#include "strings.h"

#include <thread>

using namespace wizard;

static constexpr float DEFAULT_TIMEOUT_IN_SECONDS = 30;
static constexpr uint32_t DEFAULT_MAX_ACTIVE_REQUESTS = 4;

HTTPDownloader::HTTPDownloader() : _timeout{DEFAULT_TIMEOUT_IN_SECONDS}, _maxActiveRequests{DEFAULT_MAX_ACTIVE_REQUESTS} {
}

HTTPDownloader::~HTTPDownloader() = default;

void HTTPDownloader::CreateRequest(std::string url, Request::Callback callback/*, ProgressCallback* progress*/) {
	Request* req = InternalCreateRequest();
	req->parent = this;
	req->type = Request::Type::Get;
	req->url = std::move(url);
	req->callback = std::move(callback);
	//req->progress = progress;
	req->startTime = DateTime::Now();

	std::unique_lock<std::mutex> lock{_pendingRequestLock};
	if (LockedGetActiveRequestCount() < _maxActiveRequests) {
		if (!StartRequest(req))
			return;
	}

	LockedAddRequest(req);
}

void HTTPDownloader::CreatePostRequest(std::string url, std::string postData, Request::Callback callback/*, ProgressCallback* progress*/) {
	Request* req = InternalCreateRequest();
	req->parent = this;
	req->type = Request::Type::Post;
	req->url = std::move(url);
	req->postData = std::move(postData);
	req->callback = std::move(callback);
	//req->progress = progress;
	req->startTime = DateTime::Now();

	std::unique_lock<std::mutex> lock{_pendingRequestLock};
	if (LockedGetActiveRequestCount() < _maxActiveRequests) {
		if (!StartRequest(req))
			return;
	}

	LockedAddRequest(req);
}

void HTTPDownloader::LockedPollRequests(std::unique_lock<std::mutex>& lock) {
	if (_pendingRequests.empty())
		return;

	InternalPollRequests();

	auto currentTime =  DateTime::Now();
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
		if (alive && currentTime >= req->startTime && (currentTime - req->startTime).AsSeconds() >= _timeout) {
			WZ_LOG_ERROR("Request for '{}' timed out", req->url);

			req->state.store(Request::State::Cancelled);
			_pendingRequests.erase(_pendingRequests.begin() + static_cast<intmax_t>(index));
			lock.unlock();

			req->callback(HTTP_STATUS_TIMEOUT, {}, {});

			CloseRequest(req);

			lock.lock();
			continue;
		}/* else if (alive && req->progress && req->progress->IsCancelled()) {
			WZ_LOG_ERROR("Request for '{}' cancelled", req->url);

			req->state.store(Request::State::Cancelled);
			_pendingRequests.erase(_pendingRequests.begin() + static_cast<intmax_t>(index));
			lock.unlock();

			req->callback(HTTP_STATUS_CANCELLED, {}, {});

			CloseRequest(req);

			lock.lock();
			continue;
		}*/

		if (req->state != Request::State::Complete) {
			/*if (req->progress) {
				auto size = static_cast<uint32_t>(req->data.size());
				if (size != req->lastProgressUpdate) {
					req->lastProgressUpdate = size;
					req->progress->SetProgressRange(req->contentLength);
					req->progress->SetProgressValue(req->lastProgressUpdate);
				}
			}*/

			activeRequests++;
			index++;
			continue;
		}

		WZ_LOG_VERBOSE("Request for '{}' complete, returned status code {} and {} bytes", req->url, req->statusCode, req->data.size());
		_pendingRequests.erase(_pendingRequests.begin() + static_cast<intmax_t>(index));

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
				_pendingRequests.erase(_pendingRequests.begin() + static_cast<intmax_t>(index));
				continue;
			}

			activeRequests++;
			index++;

			if (activeRequests >= _maxActiveRequests)
				break;
		}
	}
}

void HTTPDownloader::PollRequests() {
	std::unique_lock<std::mutex> lock{_pendingRequestLock};
	LockedPollRequests(lock);
}

void HTTPDownloader::WaitForAllRequests() {
	std::unique_lock<std::mutex> lock{_pendingRequestLock};
	while (!_pendingRequests.empty()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		LockedPollRequests(lock);
	}
}

void HTTPDownloader::LockedAddRequest(Request* request) {
	_pendingRequests.push_back(request);
}

uint32_t HTTPDownloader::LockedGetActiveRequestCount() {
	uint32_t count = 0;
	for (Request* req : _pendingRequests) {
		if (req->state == Request::State::Started || req->state == Request::State::Receiving)
			count++;
	}
	return count;
}

bool HTTPDownloader::HasAnyRequests() {
	std::unique_lock<std::mutex> lock{_pendingRequestLock};
	return !_pendingRequests.empty();
}

std::string_view HTTPDownloader::GetExtensionForContentType(std::string_view contentType) {
	static std::array<std::pair<std::string_view, std::string_view>, 75> table = {
		std::pair{"audio/aac", ".aac"},
		std::pair{"application/x-abiword", ".abw"},
		std::pair{"application/x-freearc", ".arc"},
		std::pair{"image/avif", ".avif"},
		std::pair{"video/x-msvideo", ".avi"},
		std::pair{"application/vnd.amazon.ebook", ".azw"},
		std::pair{"application/octet-stream", ".bin"},
		std::pair{"image/bmp", ".bmp"},
		std::pair{"application/x-bzip", ".bz"},
		std::pair{"application/x-bzip2", ".bz2"},
		std::pair{"application/x-cdf", ".cda"},
		std::pair{"application/x-csh", ".csh"},
		std::pair{"text/css", ".css"},
		std::pair{"text/csv", ".csv"},
		std::pair{"application/msword", ".doc"},
		std::pair{"application/vnd.openxmlformats-officedocument.wordprocessingml.document", ".docx"},
		std::pair{"application/vnd.ms-fontobject", ".eot"},
		std::pair{"application/epub+zip", ".epub"},
		std::pair{"application/gzip", ".gz"},
		std::pair{"image/gif", ".gif"},
		std::pair{"text/html", ".htm"},
		std::pair{"image/vnd.microsoft.icon", ".ico"},
		std::pair{"text/calendar", ".ics"},
		std::pair{"application/java-archive", ".jar"},
		std::pair{"image/jpeg", ".jpg"},
		std::pair{"text/javascript", ".js"},
		std::pair{"application/json", ".json"},
		std::pair{"application/ld+json", ".jsonld"},
		std::pair{"audio/midi audio/x-midi", ".mid"},
		std::pair{"text/javascript", ".mjs"},
		std::pair{"audio/mpeg", ".mp3"},
		std::pair{"video/mp4", ".mp4"},
		std::pair{"video/mpeg", ".mpeg"},
		std::pair{"application/vnd.apple.installer+xml", ".mpkg"},
		std::pair{"application/vnd.oasis.opendocument.presentation", ".odp"},
		std::pair{"application/vnd.oasis.opendocument.spreadsheet", ".ods"},
		std::pair{"application/vnd.oasis.opendocument.text", ".odt"},
		std::pair{"audio/ogg", ".oga"},
		std::pair{"video/ogg", ".ogv"},
		std::pair{"application/ogg", ".ogx"},
		std::pair{"audio/opus", ".opus"},
		std::pair{"font/otf", ".otf"},
		std::pair{"image/png", ".png"},
		std::pair{"application/pdf", ".pdf"},
		std::pair{"application/x-httpd-php", ".php"},
		std::pair{"application/vnd.ms-powerpoint", ".ppt"},
		std::pair{"application/vnd.openxmlformats-officedocument.presentationml.presentation", ".pptx"},
		std::pair{"application/vnd.rar", ".rar"},
		std::pair{"application/rtf", ".rtf"},
		std::pair{"application/x-sh", ".sh"},
		std::pair{"image/svg+xml", ".svg"},
		std::pair{"application/x-tar", ".tar"},
		std::pair{"image/tiff", ".tif"},
		std::pair{"video/mp2t", ".ts"},
		std::pair{"font/ttf", ".ttf"},
		std::pair{"text/plain", ".txt"},
		std::pair{"application/vnd.visio", ".vsd"},
		std::pair{"audio/wav", ".wav"},
		std::pair{"audio/webm", ".weba"},
		std::pair{"video/webm", ".webm"},
		std::pair{"image/webp", ".webp"},
		std::pair{"font/woff", ".woff"},
		std::pair{"font/woff2", ".woff2"},
		std::pair{"application/xhtml+xml", ".xhtml"},
		std::pair{"application/vnd.ms-excel", ".xls"},
		std::pair{"application/vnd.openxmlformats-officedocument.spreadsheetml.sheet", ".xlsx"},
		std::pair{"application/xml", ".xml"},
		std::pair{"text/xml", ".xml"},
		std::pair{"application/vnd.mozilla.xul+xml", ".xul"},
		std::pair{"application/zip", ".zip"},
		std::pair{"video/3gpp", ".3gp"},
		std::pair{"audio/3gpp", ".3gp"},
		std::pair{"video/3gpp2", ".3g2"},
		std::pair{"audio/3gpp2", ".3g2"},
		std::pair{"application/x-7z-compressed", ".7z"},
	};

	for (const auto& [type, ext] : table) {
		if (type.size() == contentType.size() && String::Strncasecmp(type.data(), contentType.data(), contentType.size()) == 0)
			return ext;
	}

	return {};
}