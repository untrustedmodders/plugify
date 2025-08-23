#pragma once

#include <thread>
#include <vector>
#include <future>
#include <queue>

namespace plugify {
    class ThreadPool {
    public:
        explicit ThreadPool(size_t numThreads = std::thread::hardware_concurrency()) : m_stop(false) {
            for (size_t i = 0; i < numThreads; ++i) {
                m_workers.emplace_back([this] {
                    while (true) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(m_queueMutex);
                            m_condition.wait(lock, [this] { return m_stop || !m_tasks.empty(); });

                            if (m_stop && m_tasks.empty()) {
                                return;
                            }

                            task = std::move(m_tasks.front());
                            m_tasks.pop();
                        }
                        task();
                    }
                });
            }
        }

        ~ThreadPool() {
            {
                std::unique_lock<std::mutex> lock(m_queueMutex);
                m_stop = true;
            }
            m_condition.notify_all();

            for (std::thread& worker : m_workers) {
                worker.join();
            }
        }

        template<typename F, typename... Args>
        auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
            using return_type = typename std::result_of<F(Args...)>::type;

            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );

            std::future<return_type> res = task->get_future();
            {
                std::unique_lock<std::mutex> lock(m_queueMutex);

                if (m_stop) {
                    throw std::runtime_error("enqueue on stopped ThreadPool");
                }

                m_tasks.emplace([task]() { (*task)(); });
            }
            m_condition.notify_one();
            return res;
        }

        size_t getThreadCount() const { return m_workers.size(); }
        size_t getQueueSize() const {
            std::unique_lock<std::mutex> lock(m_queueMutex);
            return m_tasks.size();
        }

    private:
        std::vector<std::thread> m_workers;
        std::queue<std::function<void()>> m_tasks;
        mutable std::mutex m_queueMutex;
        std::condition_variable m_condition;
        bool m_stop;
    };
}