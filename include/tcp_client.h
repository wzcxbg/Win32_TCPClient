#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <windows.h>
#include <string>
#include <functional>
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>

class TCPClient {
public:
    TCPClient();
    ~TCPClient();

    bool Connect(const std::string& ip, int port);
    bool Disconnect();
    bool Send(const std::string& message);
    bool IsConnected() const { return connected; }

    // Callback for receiving messages
    using MessageCallback = std::function<void(const std::string&)>;
    void SetMessageCallback(MessageCallback callback) { messageCallback = callback; }

private:
    SOCKET sock;
    bool connected;
    HANDLE receiveThread;
    bool shouldStop;
    MessageCallback messageCallback;

    static DWORD WINAPI ReceiveThreadProc(LPVOID lpParam);
    void ReceiveLoop();

    // Prevent copying
    TCPClient(const TCPClient&) = delete;
    TCPClient& operator=(const TCPClient&) = delete;
};

#endif // TCP_CLIENT_H