#pragma once

#include <atomic>
#include <plugify/date_time.hpp>

namespace plugify {
	class HTTPDownloader {
	public:
		enum : int32_t {
			HTTP_STATUS_CANCELLED = -3,
			HTTP_STATUS_TIMEOUT = -2,
			HTTP_STATUS_ERROR = -1,
			HTTP_STATUS_OK = 200
		};

		// Progress callback. If you return false, then the operation is cancelled
		using ProgressCallback = std::function<bool(uint32_t bytesDone, uint32_t bytesTotal)>;

		struct Request {
			using Data = std::vector<uint8_t>;
			using Callback = std::function<void(int32_t statusCode, std::string_view contentType, Data data)>;

			enum class Type {
				Get,
				Post,
			};

			enum class State {
				Pending,
				Cancelled,
				Started,
				Receiving,
				Complete,
			};

			HTTPDownloader* parent{};
			Callback callback;
			ProgressCallback progress;
			std::string url;
			std::string postData;
			std::string contentType;
			Data data;
			DateTime startTime;
			int32_t statusCode{};
			uint32_t contentLength{};
			uint32_t lastProgressUpdate{};
			Type type{ Type::Get };
			std::atomic<State> state{ State::Pending };
		};

		HTTPDownloader();
		virtual ~HTTPDownloader();

		static std::unique_ptr<HTTPDownloader> Create(std::string userAgent = kDefaultUserAgent);
		static std::string_view GetExtensionForContentType(std::string_view contentType);

		void CreateRequest(std::string url, Request::Callback callback, ProgressCallback progress = nullptr);
		void CreatePostRequest(std::string url, std::string postData, Request::Callback callback, ProgressCallback progress = nullptr);
		void PollRequests();
		void WaitForAllRequests();
		bool HasAnyRequests();

		void SetTimeout(float timeout) {
			_timeout = timeout;
		}

		void SetMaxActiveRequests(uint32_t maxActiveRequests) {
			_maxActiveRequests = maxActiveRequests;
		}

		static inline std::string_view kDefaultUserAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:85.0) Gecko/20100101 Firefox/85.0";

	protected:
		virtual Request* InternalCreateRequest() = 0;
		virtual void InternalPollRequests() = 0;

		virtual bool StartRequest(Request* request) = 0;
		virtual void CloseRequest(Request* request) = 0;

		void LockedAddRequest(Request* request);
		uint32_t LockedGetActiveRequestCount();
		void LockedPollRequests(std::unique_lock<std::mutex>& lock);

		float _timeout;
		uint32_t _maxActiveRequests;

		std::mutex _pendingRequestLock;
		std::vector<Request*> _pendingRequests;
	};
}
