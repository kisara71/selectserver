#ifndef THREADPOOL_HPP
#define THREADPOOL_HPP
#include <thread>
#include <atomic>
#include <queue>
#include <mutex>
#include <future>
#include <memory>
#include <map>
#include <functional>
#include <vector>
#include <iostream>
#include <condition_variable>
#include <chrono>   
class ThreadPool{
public:
    template<typename F, typename... Args>
    auto addTask(F&& f, Args&&... args)->std::future<typename std::invoke_result_t<F,Args...>>{
        using returnType = typename std::invoke_result_t<F, Args...>;
        auto taskPtr = std::make_shared<std::packaged_task<returnType(void)>>(
            std::bind(std::forward<F>(f), std::forward<Args> (args)...)
        );
        std::future<returnType> res = taskPtr->get_future();

        {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_tasks.emplace([taskPtr](){
                (*taskPtr)();
            });
            m_condition.notify_one();
        }
        return res;
    }
    ThreadPool(int min = 4, int max = std::thread::hardware_concurrency());
    ~ThreadPool();

private:
    void monitor();
    void worker();

private:
    std::thread m_monitor;

    std::atomic<int> m_maxWorkers;
    std::atomic<int> m_minWorkers;
    std::atomic<int> m_idleWorkers;
    std::atomic<int> m_destroyWorkers;
    std::atomic<bool> m_stop;
    std::atomic<int> m_totalWorkers;

    std::queue<std::function<void(void)>> m_tasks;
    std::map<std::thread::id, std::thread> m_workers;
    std::vector<std::thread::id> m_idleWorkerIds;

    std::condition_variable m_condition;
    std::mutex m_mtx;
    std::mutex m_delMtx;

};

#endif