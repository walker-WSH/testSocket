#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_HOST "127.0.0.1"
//#define SERVER_HOST "39.105.145.19"
//#define SERVER_HOST "116.204.78.5"
#define SERVER_PORT 8080

#define DEFAULT_BUFLEN      512

int __cdecl main(int argc, char** argv)
{
        SOCKET ConnectSocket = INVALID_SOCKET;

        WSADATA wsaData;
        int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != NO_ERROR)
        {
                printf("WSAStartup failed with error: %d\n", iResult);
                return 1;
        }

        ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (ConnectSocket == INVALID_SOCKET)
        {
                printf("socket failed with error: %ld \n", WSAGetLastError());
                WSACleanup();
                return 1;
        }

        sockaddr_in service;
        service.sin_family = AF_INET;
        service.sin_addr.s_addr = inet_addr(SERVER_HOST);
        service.sin_port = htons(SERVER_PORT);

        iResult = connect(ConnectSocket, (SOCKADDR*)&service, sizeof(service));
        if (iResult == SOCKET_ERROR)
        {
                printf("failed to connect!\n");
                closesocket(ConnectSocket);
                WSACleanup();
                return 1;
        }

        // Send an initial buffer
        char sendbuf[] = "this is a test";
        iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
        if (iResult == SOCKET_ERROR)  // 此处send不一定发送完毕了 必须检查其返回值 判断发送成功了多少 循环发送
        {
                printf("send failed with error: %d\n", WSAGetLastError());
                closesocket(ConnectSocket);
                WSACleanup();
                return 1;
        }

        printf("Bytes Sent: %ld\n", iResult);

        // shutdown the connection since no more data will be sent
        //iResult = shutdown(ConnectSocket, SD_SEND);
        //if (iResult == SOCKET_ERROR) 
        //{
        //    printf("shutdown failed with error: %d\n", WSAGetLastError());
        //    closesocket(ConnectSocket);
        //    WSACleanup();
        //    return 1;
        //}

        // Receive until the peer closes the connection
        do
        {
                char recvbuf[DEFAULT_BUFLEN + 1];
                int recvbuflen = DEFAULT_BUFLEN;

                iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
                if (iResult > 0)
                {
                        recvbuf[iResult] = 0;
                        printf("Bytes received: %d msg:[%s]\n", iResult, recvbuf);
                }
                else if (iResult == 0)
                        printf("Connection closed\n");
                else
                        printf("recv failed with error: %d\n", WSAGetLastError());
        } while (iResult > 0);

        // cleanup
        closesocket(ConnectSocket);
        WSACleanup();

        getchar();
        return 0;
}