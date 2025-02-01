#include <WinSock2.h>
#include <WS2tcpip.h>
#include <iostream>
int main(){
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    SOCKET ser = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr.s_addr);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(55655);

    connect(ser, (struct sockaddr*)& addr, sizeof(addr));
    getchar();
    return 0;
}