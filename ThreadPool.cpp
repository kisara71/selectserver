#include "ThreadPool.h"
ThreadPool::ThreadPool(int min , int max):
m_maxWorkers(max),
m_minWorkers(min),
m_idleWorkers(min),
m_destroyWorkers(0),
m_stop(false),
m_totalWorkers(min)
{
    m_monitor = std::thread(&ThreadPool::monitor, this);
    for (int i=0; i<m_minWorkers; ++i){
        std::thread t(&ThreadPool::worker, this);
        m_workers[t.get_id()] = std::move(t);
    }
    std::cout<<"ThreadPool created with"<<m_minWorkers<<"workers"<<std::endl;
}

void ThreadPool::worker(){
    while (true){
        std::function<void(void)> task;
        {
            std::unique_lock<std::mutex> lock(m_mtx);
            m_condition.wait(lock, [this](){
                return !m_tasks.empty()||m_destroyWorkers!=0||m_stop;
            });
            if(m_stop&&m_tasks.empty()){
                return;
            }
           {
                std::unique_lock<std::mutex> delLock(m_delMtx); 
                if(m_destroyWorkers!=0){
                    m_idleWorkers--;
                    m_totalWorkers--;
                    m_destroyWorkers--;
                    m_idleWorkerIds.push_back(std::this_thread::get_id());
                    return;
                }
            }
            if(!m_tasks.empty()){
                task = m_tasks.front();
                m_tasks.pop();
            }
        }
        m_idleWorkers--;
        task();
        m_idleWorkers++;
    }
}

void ThreadPool::monitor(){
    while(true){
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        if(m_stop){
            return;
        }
        {
            std::unique_lock<std::mutex> delLock(m_delMtx);
            if(!m_idleWorkerIds.empty()){
                for(auto it = m_idleWorkerIds.begin();it!=m_idleWorkerIds.end();){
                        if(m_workers[*it].joinable())
                        {
                            m_workers[*it].join();
                        }
                        m_workers.erase(*it);
                        it = m_idleWorkerIds.erase(it);
                        std::cout<<"destroyed a thread"<<std::endl;
                } 
        }
        }
        if(m_idleWorkers > m_totalWorkers/2&&m_totalWorkers>m_minWorkers)
        {
            m_destroyWorkers.store(m_totalWorkers/3);
            m_condition.notify_all();
        }else if(m_idleWorkers==0&&m_totalWorkers<m_maxWorkers)
        {

            std::thread t(&ThreadPool::worker, this);
            std::unique_lock<std::mutex> lock(m_mtx);
            m_workers[t.get_id()] = std::move(t);
            m_idleWorkers++;
            m_totalWorkers++;
            std::cout<<"lack of threads, appending one"<<std::endl;
        }
    }
}

ThreadPool::~ThreadPool()
{
    m_stop = true;
    m_monitor.join();
    m_condition.notify_all();
    for(auto& worker:m_workers)
    {
        if(std::get<1>(worker).joinable())
        {
            std::get<1>(worker).join();
        }
    }
    std::cout<<"all threads destroyed"<<std::endl;
}