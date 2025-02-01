#include "server.h"

Server::Server(){
    close.store(false);
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        std::cerr << "WSAStartup failed: " << WSAGetLastError() << std::endl;
    }

    m_server = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(55655);

    if((bind(m_server, (struct sockaddr*)& addr, sizeof(addr)))==SOCKET_ERROR){
        std::perror("bind failed");
    }
    if(listen(m_server, 128)==SOCKET_ERROR){
        std::perror("listen failed");
    }else{
        std::cout<<"listening on 55655"<<std::endl;
    }

    FD_ZERO(&m_readfd);
    FD_SET(m_server, &m_readfd);

    m_maxFD.store(m_server);
    pool.addTask(&Server::selectThread, this);
}

void Server::selectThread(){
    fd_set temp;
    struct timeval time;
    time.tv_sec = 5;
    time.tv_usec = 0;
    while(true){
        if(close)
            return;
        {
           std::unique_lock<std::mutex> lock(m_mtx);
           temp = m_readfd; 
        }
        int res = select(m_maxFD+1, &temp, nullptr, nullptr, nullptr);
        if(FD_ISSET(m_server, &temp)){
                struct sockaddr_in addr;
                int len = sizeof(addr);
                SOCKET client = accept(m_server, (struct sockaddr*)&addr, &len);
                pool.addTask(&Server::handleClient, this, client, addr);
                   
            }
        }
    }

Server::~Server(){
    close.store(true);
    WSACleanup();
    closesocket(m_server);
}

void Server::handleClient(SOCKET& client, sockaddr_in& addr){
     if(client==INVALID_SOCKET){
            std::perror("invalid client");
    }else{
            std::unique_lock<std::mutex> lock(m_mtx);
            m_clients[addr.sin_addr.s_addr] = client;
            char* ip = inet_ntoa(addr.sin_addr);
            std::cout<<"client:  "<<ip<<"  connected"<<std::endl;
    }
}