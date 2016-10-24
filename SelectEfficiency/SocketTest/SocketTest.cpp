// SocketTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <vector>

#define PORT 15150
#define DATA_BUFSIZE 8192

typedef struct _SOCKET_INFORMATION {
    CHAR Buffer[DATA_BUFSIZE];
    WSABUF DataBuf;
    SOCKET Socket;
    OVERLAPPED Overlapped;
    DWORD BytesSEND;
    DWORD BytesRECV;
} SOCKET_INFORMATION, *LPSOCKET_INFORMATION;

// Prototypes
BOOL CreateSocketInformation(SOCKET s);
void FreeSocketInformation(DWORD Index);

// Global var
DWORD TotalSockets = 0;
LPSOCKET_INFORMATION SocketArray[FD_SETSIZE];

double get_time()
{
    // http://stackoverflow.com/questions/2349776/how-can-i-benchmark-c-code-easily
    LARGE_INTEGER t, f;
    QueryPerformanceCounter(&t);
    QueryPerformanceFrequency(&f);
    return (double)t.QuadPart / (double)f.QuadPart;
}

int SelectCallsPerSecond(int socketCount) {
    SOCKET AcceptSocket;
    SOCKADDR_IN InternetAddr;
    WSADATA wsaData;
    INT Ret;
    FD_SET WriteSet;
    FD_SET ReadSet;
    DWORD i;
    DWORD readyHandles;
    ULONG NonBlock;
    DWORD Flags;
    DWORD SendBytes;
    DWORD RecvBytes;

    std::vector<SOCKET> sockets;

    if ((Ret = WSAStartup(0x0202, &wsaData)) != 0)
    {
        printf("WSAStartup() failed with error %d\n", Ret);
        WSACleanup();
        return 1;
    }
    else
        printf("WSAStartup() is fine!\n");

    // Prepare a socket to listen for connections
    for (int index = 0; index < socketCount; index++) {
        SOCKET ListenSocket;
        if ((ListenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
        {
            printf("WSASocket() failed with error %d\n", WSAGetLastError());
            return 1;
        }
        else {
            //printf("WSASocket() is OK!\n");
        }

        InternetAddr.sin_family = AF_INET;
        InternetAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        InternetAddr.sin_port = htons(PORT + index);

        if (bind(ListenSocket, (PSOCKADDR)&InternetAddr, sizeof(InternetAddr)) == SOCKET_ERROR)
        {
            printf("bind() failed with error %d\n", WSAGetLastError());
            return 1;
        }
        else {
            //printf("bind() is OK!\n");
        }

        if (listen(ListenSocket, 5))
        {
            printf("listen() failed with error %d\n", WSAGetLastError());
            return 1;
        }
        else {
            //printf("listen() is OK!\n");
        }

        // Change the socket mode on the listening socket from blocking to
        // non-block so the application will not block waiting for requests
        NonBlock = 1;
        if (ioctlsocket(ListenSocket, FIONBIO, &NonBlock) == SOCKET_ERROR)
        {
            printf("ioctlsocket() failed with error %d\n", WSAGetLastError());
            return 1;
        }
        else
            printf("ioctlsocket() is OK!\n");

        sockets.push_back(ListenSocket);
    }

    int selectCallCount = 0;
    double start = get_time();
    int cycle = 100;
    while (TRUE)
    {
        selectCallCount++;
        if (selectCallCount > cycle) {
            selectCallCount = 0;
            double duration = get_time() - start;
            printf("Duration: %g\nFor count in cycle: %d\n", duration, cycle);

            // slow down or speed up measuring cycle to reach 1 second duration
            if (duration < 0.95) {
                cycle = static_cast<int>(cycle * 1.1);
            } else if (duration > 1.05) {
                cycle = static_cast<int>(cycle / 1.1);
            } else {
                // the duration is about 1 second. return the cycle count
                return cycle;
            }
            start = get_time();
        }

        // Prepare the Read and Write socket sets for network I/O notification
        FD_ZERO(&ReadSet);
        FD_ZERO(&WriteSet);

        // Always look for connection attempts
        for (const auto& sock : sockets) {
            FD_SET(sock, &ReadSet);
        }

        // Set Read and Write notification for each socket based on the
        // current state the buffer.  If there is data remaining in the
        // buffer then set the Write set otherwise the Read set
        for (i = 0; i < TotalSockets; i++) {
            if (SocketArray[i]->BytesRECV > SocketArray[i]->BytesSEND)
                FD_SET(SocketArray[i]->Socket, &WriteSet);
            else
                FD_SET(SocketArray[i]->Socket, &ReadSet);
        }

        //printf("calling `select`\n");
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 0;

        if ((readyHandles = select(0, &ReadSet, &WriteSet, NULL, &tv)) == SOCKET_ERROR)
        {
            //printf("select() returned with error %d\n", WSAGetLastError());
            return 1;
        }
        else {
            //printf("select() is OK!\n");
        }

        // Check for arriving connections on the listening socket.
        for (const auto& sockToAccept : sockets) {
            if (FD_ISSET(sockToAccept, &ReadSet))
            {
                readyHandles--;
                if ((AcceptSocket = accept(sockToAccept, NULL, NULL)) != INVALID_SOCKET)
                {
                    // Set the accepted socket to non-blocking mode so the server will
                    // not get caught in a blocked condition on WSASends
                    NonBlock = 1;
                    if (ioctlsocket(AcceptSocket, FIONBIO, &NonBlock) == SOCKET_ERROR)
                    {
                        printf("ioctlsocket(FIONBIO) failed with error %d\n", WSAGetLastError());
                        return 1;
                    }
                    else {
                        printf("ioctlsocket(FIONBIO) is OK!\n");
                    }

                    if (CreateSocketInformation(AcceptSocket) == FALSE)
                    {
                        printf("CreateSocketInformation(AcceptSocket) failed!\n");
                        return 1;
                    }
                    else
                        printf("CreateSocketInformation() is OK!\n");

                }
                else
                {
                    if (WSAGetLastError() != WSAEWOULDBLOCK)
                    {
                        printf("accept() failed with error %d\n", WSAGetLastError());
                        return 1;
                    }
                    else
                        printf("accept() is fine!\n");
                }
            }
        }

        // Check each socket for Read and Write notification until the number
        // of sockets in readyHandles is satisfied
        for (i = 0; readyHandles > 0 && i < TotalSockets; i++)
        {
            LPSOCKET_INFORMATION SocketInfo = SocketArray[i];

            // If the ReadSet is marked for this socket then this means data
            // is available to be read on the socket
            if (FD_ISSET(SocketInfo->Socket, &ReadSet))
            {
                readyHandles--;

                SocketInfo->DataBuf.buf = SocketInfo->Buffer;
                SocketInfo->DataBuf.len = DATA_BUFSIZE;

                Flags = 0;
                if (WSARecv(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &RecvBytes, &Flags, NULL, NULL) == SOCKET_ERROR)
                {
                    if (WSAGetLastError() != WSAEWOULDBLOCK)
                    {
                        printf("WSARecv() failed with error %d\n", WSAGetLastError());
                        FreeSocketInformation(i);
                    }
                    else
                        printf("WSARecv() is OK!\n");
                    continue;
                }
                else
                {
                    SocketInfo->BytesRECV = RecvBytes;

                    // If zero bytes are received, this indicates the peer closed the connection.
                    if (RecvBytes == 0)
                    {
                        FreeSocketInformation(i);
                        continue;
                    }
                }
            }

            // If the WriteSet is marked on this socket then this means the internal
            // data buffers are available for more data
            if (FD_ISSET(SocketInfo->Socket, &WriteSet))
            {
                readyHandles--;

                SocketInfo->DataBuf.buf = SocketInfo->Buffer + SocketInfo->BytesSEND;
                SocketInfo->DataBuf.len = SocketInfo->BytesRECV - SocketInfo->BytesSEND;

                if (WSASend(SocketInfo->Socket, &(SocketInfo->DataBuf), 1, &SendBytes, 0, NULL, NULL) == SOCKET_ERROR)
                {
                    if (WSAGetLastError() != WSAEWOULDBLOCK)
                    {
                        printf("WSASend() failed with error %d\n", WSAGetLastError());
                        FreeSocketInformation(i);
                    }
                    else
                        printf("WSASend() is OK!\n");

                    continue;
                }
                else
                {
                    SocketInfo->BytesSEND += SendBytes;

                    if (SocketInfo->BytesSEND == SocketInfo->BytesRECV)
                    {
                        SocketInfo->BytesSEND = 0;
                        SocketInfo->BytesRECV = 0;
                    }
                }
            }
        }
    }
}

BOOL CreateSocketInformation(SOCKET s)
{
    LPSOCKET_INFORMATION SI;

    printf("Accepted socket number %d\n", s);

    if ((SI = (LPSOCKET_INFORMATION)GlobalAlloc(GPTR, sizeof(SOCKET_INFORMATION))) == NULL)
    {
        printf("GlobalAlloc() failed with error %d\n", GetLastError());
        return FALSE;
    }
    else
        printf("GlobalAlloc() for SOCKET_INFORMATION is OK!\n");

    // Prepare SocketInfo structure for use
    SI->Socket = s;
    SI->BytesSEND = 0;
    SI->BytesRECV = 0;

    SocketArray[TotalSockets] = SI;
    TotalSockets++;
    return(TRUE);
}

void FreeSocketInformation(DWORD Index)
{
    LPSOCKET_INFORMATION SI = SocketArray[Index];
    DWORD i;

    closesocket(SI->Socket);
    printf("Closing socket number %d\n", SI->Socket);
    GlobalFree(SI);

    // Squash the socket array
    for (i = Index; i < TotalSockets; i++)
    {
        SocketArray[i] = SocketArray[i + 1];
    }

    TotalSockets--;
}


int main(int argc, char **argv)
{
    printf("Usage: SocketTest.exe [socket count]\n");
    int socketCount = atoi(argv[1]);
    printf("Measuring with this many sockets: %d", socketCount);
    int cycles = SelectCallsPerSecond(socketCount);
    printf("\nCycles for 1 second duration: %d\n", cycles);
    return cycles;
}