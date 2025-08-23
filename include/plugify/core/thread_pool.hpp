#pragma once

#include <thread>
#include <vector>
#include <future>
#include <queue>
#include <functional>
#include <type_traits>

namespace plugify {
    class ThreadPool {
    public:
        explicit ThreadPool(size_t numThreads = std::thread::hardware_concurrency()) {
            if (numThreads == 0) {
                numThreads = 1; // safe default
            }

            for (size_t i = 0; i < numThreads; ++i) {
                _workers.emplace_back([this](std::stop_token stoken) {
                    while (true) {
                        std::function<void()> task;
                        {
                            std::unique_lock lock(_queueMutex);
                            _condition.wait(lock, stoken, [this] {
                                return _stop || !_tasks.empty();
                            });

                            if ((_stop && _tasks.empty()) || stoken.stop_requested()) {
                                return;
                            }

                            task = std::move(_tasks.front());
                            _tasks.pop();
                        }

                        task();
                    }
                });
            }
        }

        ~ThreadPool() {
            {
                std::unique_lock lock(_queueMutex);
                _stop = true;
            }
            _condition.notify_all();
        }

        template<typename F, typename... Args>
        auto Enqueue(F&& f, Args&&... args)
            -> std::future<std::invoke_result_t<F, Args...>> {
            using return_type = std::invoke_result_t<F, Args...>;

            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );

            std::future<return_type> res = task->get_future();
            {
                std::unique_lock lock(_queueMutex);
                if (_stop) {
                    throw std::runtime_error("enqueue on stopped ThreadPool");
                }
                _tasks.emplace([task]() { (*task)(); });
            }
            _condition.notify_one();
            return res;
        }

        size_t GetThreadCount() const { return _workers.size(); }

        size_t GetQueueSize() const {
            std::scoped_lock lock(_queueMutex);
            return _tasks.size();
        }

    private:
        std::vector<std::jthread> _workers;
        std::queue<std::function<void()>> _tasks;
        mutable std::mutex _queueMutex;
        std::condition_variable_any _condition; // works with stop_token
        bool _stop{false};
    };
}