// SocketServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdint.h>

#pragma comment(lib, "ws2_32.lib")

// 客户端数据回声
void EchoServer()
{
    /*
    typedef struct WSAData {
        WORD           wVersion;  //ws2_32.dll 建议我们使用的版本号
        WORD           wHighVersion;  //ws2_32.dll 支持的最高版本号
        //一个以 null 结尾的字符串，用来说明 ws2_32.dll 的实现以及厂商信息
        char           szDescription[WSADESCRIPTION_LEN + 1];
        //一个以 null 结尾的字符串，用来说明 ws2_32.dll 的状态以及配置信息
        char           szSystemStatus[WSASYS_STATUS_LEN + 1];
        unsigned short iMaxSockets;  //2.0以后不再使用
        unsigned short iMaxUdpDg;  //2.0以后不再使用
        char FAR       *lpVendorInfo;  //2.0以后不再使用
    } WSADATA, *LPWSADATA;
    */
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);

    // AF_INET - ipv4 ,  AF_INET6 - ipv6
    // SOCK_STREAM - 流格式套接字/面向连接的套接字 ,  SOCK_DGRAM - 数据报套接字/无连接的套接字
    // IPPROTO_TCP - TCP 传输协议 , IPPTOTO_UDP - UDP 传输协议  (填0也可以，win可以根据前面两个进行推导)
    SOCKET server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    /*
    struct sockaddr_in {
        sa_family_t     sin_family;   //地址族（Address Family），也就是地址类型
        uint16_t        sin_port;     //16位的端口号, 0~1023 - 系统分配服务端口， 1024~65536 用户分配服务断开
        struct in_addr  sin_addr;     //32位IP地址
        char            sin_zero[8];  //不使用，一般用0填充
    };

    struct sockaddr{
        sa_family_t  sin_family;   //地址族（Address Family），也就是地址类型
        char         sa_data[14];  //IP地址和端口号
    };

    将 sockaddr_in 转成 sockaddr 的原因是， sockaddr 可能是64IP，是一个通用的结构，大小和 ipv4的 sockaddr_in 相同。
    */
    sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = PF_INET;
    InetPtonA(PF_INET, "127.0.0.1", &sock_addr.sin_addr.s_addr);
    sock_addr.sin_port = htons(1234);

    // int bind(SOCKET sock, const struct sockaddr *addr, int addrlen);
    bind(server_socket, (sockaddr*)&sock_addr, sizeof(sock_addr));

    // int listen(SOCKET sock, int backlog);
    // sock 为需要进入监听状态的套接字， backlog 为请求队列的最大长度
    // listen() 只是让套接字进入监听状态，并没有真正接收客户端请求，
    // listen() 后面的代码会继续执行，直到遇到 accept()。accept() 会阻塞程序执行（后面代码不能被执行），直到有新的请求到来。
    // 可以把包括listen到处理数据的逻辑放到线程中
    listen(server_socket, 20);

    // 输入输出缓冲区的默认大小 64k, 8k
    uint32_t opt_val = 0;
    int opt_len = sizeof(uint32_t);
    getsockopt(server_socket, SOL_SOCKET, SO_SNDBUF, (char*)&opt_val, &opt_len);
    printf("Buffer length: %d\n", opt_val);

    while (true) {
        sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        int size = sizeof(client_addr);

        // SOCKET accept(SOCKET sock, struct sockaddr *addr, int *addrlen);
        // accept() 返回一个新的套接字来和客户端通信，addr 保存了客户端的IP地址和端口号，而 sock 是服务器端的套接字，大家注意区分。
        // 后面和客户端通信时，要使用这个新生成的套接字，而不是原来服务器端的套接字。
        // 如果没有客户端调用connect请求，会在这里一直阻塞
        SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &size);

        char sz_content[MAX_PATH] = { 0 };
        // 如果客户端没有send()数据过来，缓冲区此时为空，会在这里一直阻塞
        int recv_num = recv(client_socket, sz_content, MAX_PATH, 0);

        send(client_socket, sz_content, recv_num, 0);
        //printf("receive: %s", sz_content);

        closesocket(client_socket);
    }

    closesocket(server_socket);

    WSACleanup();
}

void FileTransServer() {
    WSADATA ws_data;
    memset(&ws_data, 0, sizeof(ws_data));
    WSAStartup(MAKEWORD(2, 2), &ws_data);

    SOCKET server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = PF_INET;
    InetPtonA(PF_INET, "127.0.0.1", &sock_addr.sin_addr.s_addr);
    sock_addr.sin_port = htons(1234);

    bind(server_socket, (sockaddr*)&sock_addr, sizeof(sockaddr));

    listen(server_socket, 20);

    sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(client_addr));
    int size = sizeof(client_addr);
    SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &size);

    char buff[1024] = { 0 };
    int nCount = 0;
    FILE* file = NULL;
    file = fopen("D:\\1.avi", "rb");
    if (file) {
        nCount = fread(buff, 1, 1024, file);
        while (nCount > 0) {
            send(client_socket, buff, nCount, 0);
            nCount = fread(buff, 1, 1024, file);
        }
        fclose(file);
    }
    shutdown(client_socket, SD_SEND);

    recv(client_socket, buff, 1024, 0);
    closesocket(client_socket);
    closesocket(server_socket);

    WSACleanup();
}


void UdpEchoServer() {
    WSADATA ws_data;
    WSAStartup(MAKEWORD(2, 2), &ws_data);

    SOCKET server_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

    sockaddr_in server_addr;
    server_addr.sin_family = PF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(1234);
    bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr));

    sockaddr_in client_addr;
    int size = sizeof(client_addr);
    char buff[1024] = { 0 };
    while (true) {
        int recv_len = recvfrom(server_socket, buff, 1024, 0, (sockaddr*)&client_addr, &size);
        sendto(server_socket, buff, recv_len, 0, (sockaddr*)&client_addr, size);
    }

    closesocket(server_socket);
    WSACleanup();
}

int main() {
    UdpEchoServer();
    return 0;
}