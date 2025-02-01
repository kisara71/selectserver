#ifndef SERVER_H
#define SERVER_H
#include <iostream>
#include <winsock2.h>
#include "ThreadPool.h" 
#include <thread>
#include <mutex>
#include <map>
#include <string>
#include <condition_variable>
#include <atomic>

class Server{
public:
    Server();
    ~Server();

private:
    void selectThread();
    void handleClient(SOCKET& client, sockaddr_in& addr);
private:
    ThreadPool pool;
    SOCKET m_server;
    fd_set m_readfd;
    std::atomic<uint32_t> m_maxFD;
    std::unordered_map<uint32_t,SOCKET> m_clients;

    std::thread m_select;
    std::mutex m_mtx;
    std::condition_variable m_condition;
    std::atomic<bool> close;
};


#endif