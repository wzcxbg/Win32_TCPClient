#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include "tcp_client.h"
#include <stdexcept>

TCPClient::TCPClient() : sock(INVALID_SOCKET), connected(false),
    receiveThread(NULL), shouldStop(false) {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        throw std::runtime_error("WSAStartup failed");
    }
}

TCPClient::~TCPClient() {
    Disconnect();
    WSACleanup();
}

bool TCPClient::Connect(const std::string& ip, int port) {
    if (connected) {
        return false;
    }

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        return false;
    }

    // Set up server address
    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &serverAddr.sin_addr);

    // Connect to server
    if (connect(sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(sock);
        sock = INVALID_SOCKET;
        return false;
    }

    connected = true;
    shouldStop = false;

    // Start receive thread
    receiveThread = CreateThread(
        NULL,
        0,
        ReceiveThreadProc,
        this,
        0,
        NULL
    );

    return true;
}

bool TCPClient::Disconnect() {
    if (!connected) {
        return false;
    }

    shouldStop = true;
    connected = false;

    // Close socket
    if (sock != INVALID_SOCKET) {
        closesocket(sock);
        sock = INVALID_SOCKET;
    }

    // Wait for receive thread to finish
    if (receiveThread != NULL) {
        WaitForSingleObject(receiveThread, INFINITE);
        CloseHandle(receiveThread);
        receiveThread = NULL;
    }

    return true;
}

bool TCPClient::Send(const std::string& message) {
    if (!connected) {
        return false;
    }

    int result = send(sock, message.c_str(), message.length(), 0);
    return (result != SOCKET_ERROR);
}

DWORD WINAPI TCPClient::ReceiveThreadProc(LPVOID lpParam) {
    TCPClient* client = static_cast<TCPClient*>(lpParam);
    client->ReceiveLoop();
    return 0;
}

void TCPClient::ReceiveLoop() {
    char buffer[1024];
    while (!shouldStop) {
        int bytesReceived = recv(sock, buffer, sizeof(buffer) - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            if (messageCallback) {
                messageCallback(std::string(buffer));
            }
        } else if (bytesReceived == 0 || bytesReceived == SOCKET_ERROR) {
            break;
        }
    }
}