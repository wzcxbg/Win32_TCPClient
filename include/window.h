#ifndef WINDOW_H
#define WINDOW_H

#include <windows.h>
#include <string>
#include "tcp_client.h"

class Window {
public:
    Window();
    ~Window();

    bool Create();
    void Show(int nCmdShow);
    HWND GetHandle() const { return hwnd; }

    // Message handling
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HWND hwnd;               // Main window handle
    HWND hwndIpEdit;         // IP address edit control
    HWND hwndPortEdit;       // Port edit control
    HWND hwndConnectBtn;     // Connect button
    HWND hwndMessageEdit;    // Message edit control
    HWND hwndSendBtn;        // Send button
    HWND hwndLogEdit;        // Log text area

    TCPClient tcpClient;     // TCP client instance
    bool isConnected;        // Connection status

    // Window dimensions
    static constexpr int WINDOW_WIDTH = 600;
    static constexpr int WINDOW_HEIGHT = 400;

    void CreateControls();
    void HandleCommand(WPARAM wParam, LPARAM lParam);
    void HandleCreate();
    void HandleDestroy();

    // Prevent copying
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
};

#endif // WINDOW_H