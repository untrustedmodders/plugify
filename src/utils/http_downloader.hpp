#pragma once

#include <plugify/date_time.hpp>
#include <plugify/downloader.hpp>

#include <atomic>

namespace plugify {
	class IHTTPDownloader : public IDownloader {
	public:
		enum {
			HTTP_STATUS_CANCELLED = -3,
			HTTP_STATUS_TIMEOUT = -2,
			HTTP_STATUS_ERROR = -1,
			HTTP_STATUS_OK = 200
		};

		struct Request {
			enum class Type {
				Get,
				Post,
				Head
			};

			enum class State {
				Pending,
				Cancelled,
				Started,
				Receiving,
				Complete,
			};

			IHTTPDownloader* parent{};
			RequestCallback callback;
			ProgressCallback progress;
			std::string url;
			std::string postData;
			std::string contentType;
			std::vector<uint8_t> data;
			DateTime startTime;
			int32_t statusCode{};
			uint32_t contentLength{};
			uint32_t lastProgressUpdate{};
			Type type{ Type::Get };
			std::atomic<State> state{ State::Pending };
		};

		IHTTPDownloader();
		~IHTTPDownloader() override;

		static inline const char* const kDefaultUserAgent = "Plugify/1.0";

		static std::unique_ptr<IHTTPDownloader> Create(std::string_view userAgent = kDefaultUserAgent);
		static std::string_view GetExtensionForContentType(std::string_view contentType);

		void CreateRequest(std::string url, RequestCallback callback, ProgressCallback progress) override;
		void CreatePostRequest(std::string url, std::string postData, RequestCallback callback, ProgressCallback progress) override;
		void CreateHeadRequest(std::string url, RequestCallback callback) override;

		void PollRequests() override;
		void WaitForAllRequests() override;
		bool HasAnyRequests() override;

		void SetTimeout(float timeout) override {
			_timeout = timeout;
		}

		void SetMaxActiveRequests(uint32_t maxActiveRequests) override {
			_maxActiveRequests = maxActiveRequests;
		}

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
