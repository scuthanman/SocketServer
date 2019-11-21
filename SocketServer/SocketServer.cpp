// SocketServer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <stdio.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <stdint.h>

#pragma comment(lib, "ws2_32.lib")

// �ͻ������ݻ���
void EchoServer()
{
    /*
    typedef struct WSAData {
        WORD           wVersion;  //ws2_32.dll ��������ʹ�õİ汾��
        WORD           wHighVersion;  //ws2_32.dll ֧�ֵ���߰汾��
        //һ���� null ��β���ַ���������˵�� ws2_32.dll ��ʵ���Լ�������Ϣ
        char           szDescription[WSADESCRIPTION_LEN + 1];
        //һ���� null ��β���ַ���������˵�� ws2_32.dll ��״̬�Լ�������Ϣ
        char           szSystemStatus[WSASYS_STATUS_LEN + 1];
        unsigned short iMaxSockets;  //2.0�Ժ���ʹ��
        unsigned short iMaxUdpDg;  //2.0�Ժ���ʹ��
        char FAR       *lpVendorInfo;  //2.0�Ժ���ʹ��
    } WSADATA, *LPWSADATA;
    */
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);

    // AF_INET - ipv4 ,  AF_INET6 - ipv6
    // SOCK_STREAM - ����ʽ�׽���/�������ӵ��׽��� ,  SOCK_DGRAM - ���ݱ��׽���/�����ӵ��׽���
    // IPPROTO_TCP - TCP ����Э�� , IPPTOTO_UDP - UDP ����Э��  (��0Ҳ���ԣ�win���Ը���ǰ�����������Ƶ�)
    SOCKET server_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    
    /*
    struct sockaddr_in {
        sa_family_t     sin_family;   //��ַ�壨Address Family����Ҳ���ǵ�ַ����
        uint16_t        sin_port;     //16λ�Ķ˿ں�, 0~1023 - ϵͳ�������˿ڣ� 1024~65536 �û��������Ͽ�
        struct in_addr  sin_addr;     //32λIP��ַ
        char            sin_zero[8];  //��ʹ�ã�һ����0���
    };

    struct sockaddr{
        sa_family_t  sin_family;   //��ַ�壨Address Family����Ҳ���ǵ�ַ����
        char         sa_data[14];  //IP��ַ�Ͷ˿ں�
    };

    �� sockaddr_in ת�� sockaddr ��ԭ���ǣ� sockaddr ������64IP����һ��ͨ�õĽṹ����С�� ipv4�� sockaddr_in ��ͬ��
    */
    sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));
    sock_addr.sin_family = PF_INET;
    InetPtonA(PF_INET, "127.0.0.1", &sock_addr.sin_addr.s_addr);
    sock_addr.sin_port = htons(1234);

    // int bind(SOCKET sock, const struct sockaddr *addr, int addrlen);
    bind(server_socket, (sockaddr*)&sock_addr, sizeof(sock_addr));

    // int listen(SOCKET sock, int backlog);
    // sock Ϊ��Ҫ�������״̬���׽��֣� backlog Ϊ������е���󳤶�
    // listen() ֻ�����׽��ֽ������״̬����û���������տͻ�������
    // listen() ����Ĵ�������ִ�У�ֱ������ accept()��accept() ����������ִ�У�������벻�ܱ�ִ�У���ֱ�����µ���������
    // ���԰Ѱ���listen���������ݵ��߼��ŵ��߳���
    listen(server_socket, 20);

    // ���������������Ĭ�ϴ�С 64k, 8k
    uint32_t opt_val = 0;
    int opt_len = sizeof(uint32_t);
    getsockopt(server_socket, SOL_SOCKET, SO_SNDBUF, (char*)&opt_val, &opt_len);
    printf("Buffer length: %d\n", opt_val);

    while (true) {
        sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        int size = sizeof(client_addr);

        // SOCKET accept(SOCKET sock, struct sockaddr *addr, int *addrlen);
        // accept() ����һ���µ��׽������Ϳͻ���ͨ�ţ�addr �����˿ͻ��˵�IP��ַ�Ͷ˿ںţ��� sock �Ƿ������˵��׽��֣����ע�����֡�
        // ����Ϳͻ���ͨ��ʱ��Ҫʹ����������ɵ��׽��֣�������ԭ���������˵��׽��֡�
        // ���û�пͻ��˵���connect���󣬻�������һֱ����
        SOCKET client_socket = accept(server_socket, (sockaddr*)&client_addr, &size);

        char sz_content[MAX_PATH] = { 0 };
        // ����ͻ���û��send()���ݹ�������������ʱΪ�գ���������һֱ����
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