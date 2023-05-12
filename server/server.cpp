#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <process.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <assert.h>

#pragma comment (lib, "Ws2_32.lib") 

#define SERVER_HOST "127.0.0.1"
#define SERVER_PORT 10096


#define DEFAULT_BUFLEN      512 

int __cdecl main(void)
{
        SOCKET ListenSocket = INVALID_SOCKET;
        SOCKET ClientSocket = INVALID_SOCKET;

        WSADATA wsaData;
        int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (iResult != NO_ERROR)
        {
                printf("WSAStartup failed with error: %d\n", iResult);
                return 1;
        }

        ListenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (ListenSocket == INVALID_SOCKET)
        {
                printf("socket failed with error: %ld\n", WSAGetLastError());

                WSACleanup();
                return 1;
        }

        sockaddr_in service;
        service.sin_family = AF_INET;
        service.sin_addr.s_addr = inet_addr(SERVER_HOST);

        int port = SERVER_PORT;
        while (1)
        {
                service.sin_port = htons(port);

                iResult = bind(ListenSocket, (SOCKADDR*)&service, sizeof(service));
                if (iResult == SOCKET_ERROR)
                {
                        printf("[%d] bind failed with error: %d\n", port, WSAGetLastError());
                        ++port;
                }
                else
                {
                        printf("bind port: %d\n", port);
                        break;
                }
        }

        iResult = listen(ListenSocket, SOMAXCONN);
        if (iResult == SOCKET_ERROR)
        {
                printf("listen failed with error: %d\n", WSAGetLastError());

                closesocket(ListenSocket);
                WSACleanup();
                return 1;
        }

        // Accept a client socket
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET)
        {
                printf("accept failed with error: %d\n", WSAGetLastError());
                closesocket(ListenSocket);
                WSACleanup();
                return 1;
        }

        // No longer need server socket
        closesocket(ListenSocket);

        // Receive until the peer shuts down the connection
        do
        {
                char recvbuf[DEFAULT_BUFLEN + 1];
                int recvbuflen = DEFAULT_BUFLEN;

                iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
                if (iResult > 0)
                {
                        recvbuf[iResult] = 0;
                        printf("Bytes received: %d msg:[%s] \n", iResult, recvbuf);

                        std::string resp = recvbuf;
                        resp += "(resp)";

                        int iSendResult = send(ClientSocket, resp.c_str(), resp.length(), 0);
                        if (iSendResult == SOCKET_ERROR)   // 此处send不一定发送完毕了 必须检查其返回值 判断发送成功了多少 循环发送
                        {
                                printf("send failed with error: %d\n", WSAGetLastError());

                                closesocket(ClientSocket);
                                WSACleanup();
                                return 1;
                        }
                        printf("Bytes sent: %d\n", iSendResult);
                }
                else if (iResult == 0)
                {
                        printf("Connection closing...\n");
                }
                else
                {
                        printf("recv failed with error: %d\n", WSAGetLastError());

                        closesocket(ClientSocket);
                        WSACleanup();
                        return 1;
                }
        } while (iResult > 0);

        iResult = shutdown(ClientSocket, SD_SEND);
        if (iResult == SOCKET_ERROR)
        {
                printf("shutdown failed with error: %d\n", WSAGetLastError());

                closesocket(ClientSocket);
                WSACleanup();
                return 1;
        }

        // cleanup
        closesocket(ClientSocket);
        WSACleanup();

        getchar();
        return 0;
}