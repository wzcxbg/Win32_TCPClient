#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <windowsx.h>
#include <richedit.h>
#include "window.h"
#include "resource.h"
#include "tcp_client.h"

Window::Window() : hwnd(NULL), hwndIpEdit(NULL), hwndPortEdit(NULL),
    hwndConnectBtn(NULL), hwndMessageEdit(NULL), hwndSendBtn(NULL),
    hwndLogEdit(NULL), isConnected(false) {}

Window::~Window() {}

bool Window::Create() {
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = WINDOW_CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClassEx(&wc)) {
        return false;
    }

    // Create the window
    // Get screen dimensions
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    
    // Calculate window position
    int xPos = (screenWidth - WINDOW_WIDTH) / 2;
    int yPos = (screenHeight - WINDOW_HEIGHT) / 2;
    
    hwnd = CreateWindowEx(
        0,                              // Optional window styles
        WINDOW_CLASS_NAME,               // Window class
        L"TCP Client",                  // Window text
        WS_OVERLAPPEDWINDOW,            // Window style
        xPos, yPos,                     // Position
        WINDOW_WIDTH, WINDOW_HEIGHT,     // Size
        NULL,                           // Parent window
        NULL,                           // Menu
        GetModuleHandle(NULL),          // Instance handle
        this                            // Additional application data
    );

    if (hwnd == NULL) {
        return false;
    }

    // Create all child controls
    CreateControls();
    
    return true;
}

void Window::Show(int nCmdShow) {
    ShowWindow(hwnd, nCmdShow);
}

void Window::CreateControls() {
    // Create IP address edit control
    hwndIpEdit = CreateWindowEx(
        WS_EX_CLIENTEDGE, L"EDIT", L"127.0.0.1",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        10, 10, 350, 25,
        hwnd, (HMENU)IDC_IP_EDIT, GetModuleHandle(NULL), NULL
    );

    // Create port edit control
    hwndPortEdit = CreateWindowEx(
        WS_EX_CLIENTEDGE, L"EDIT", L"8080",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_NUMBER,
        370, 10, 120, 25,
        hwnd, (HMENU)IDC_PORT_EDIT, GetModuleHandle(NULL), NULL
    );

    // Create connect button
    hwndConnectBtn = CreateWindowEx(
        0, L"BUTTON", L"Connect",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        500, 10, 80, 25,
        hwnd, (HMENU)IDC_CONNECT_BTN, GetModuleHandle(NULL), NULL
    );

    // Create message edit control
    hwndMessageEdit = CreateWindowEx(
        WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        10, 45, 480, 25,
        hwnd, (HMENU)IDC_MESSAGE_EDIT, GetModuleHandle(NULL), NULL
    );

    // Create send button
    hwndSendBtn = CreateWindowEx(
        0, L"BUTTON", L"Send",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        500, 45, 80, 25,
        hwnd, (HMENU)IDC_SEND_BTN, GetModuleHandle(NULL), NULL
    );

    // Create message log window with flat style
    hwndLogEdit = CreateWindowEx(
        0, L"RICHEDIT50W", L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_BORDER,
        10, 80, 570, 270,
        hwnd, (HMENU)IDC_LOG_EDIT, GetModuleHandle(NULL), NULL
    );
    
    // Set flat style background color
    SendMessage(hwndLogEdit, EM_SETBKGNDCOLOR, 0, (LPARAM)RGB(255, 255, 255));
    
    // Remove 3D border effect
    LONG_PTR style = GetWindowLongPtr(hwndLogEdit, GWL_EXSTYLE);
    style &= ~WS_EX_CLIENTEDGE;
    SetWindowLongPtr(hwndLogEdit, GWL_EXSTYLE, style);
    
    // Force redraw
    SetWindowPos(hwndLogEdit, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

    // Set default font for all controls
    HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
    SendMessage(hwndIpEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hwndPortEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hwndConnectBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hwndMessageEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hwndSendBtn, WM_SETFONT, (WPARAM)hFont, TRUE);
    SendMessage(hwndLogEdit, WM_SETFONT, (WPARAM)hFont, TRUE);
}

LRESULT CALLBACK Window::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    Window* pThis = NULL;

    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pThis = (Window*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
        pThis->hwnd = hwnd;  // Store the window handle
    } else {
        pThis = (Window*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    if (pThis) {
        switch (uMsg) {
            case WM_CREATE:
                pThis->HandleCreate();
                return 0;

            case WM_COMMAND:
                pThis->HandleCommand(wParam, lParam);
                return 0;

            case WM_DESTROY:
                pThis->HandleDestroy();
                return 0;

            case WM_APP + 1: {
                // Handle received message
                std::wstring* pMessage = (std::wstring*)wParam;
                if (pMessage) {
                    wchar_t logBuffer[1024];
                    swprintf(logBuffer, 1024, L"%s\r\n", pMessage->c_str());
                    // Append message to log
                    int len = GetWindowTextLength(pThis->hwndLogEdit);
                    SendMessage(pThis->hwndLogEdit, EM_SETSEL, len, len);
                    
                    // Set red color for received messages
                    CHARFORMAT2 cf = {};
                    cf.cbSize = sizeof(CHARFORMAT2);
                    cf.dwMask = CFM_COLOR;
                    cf.crTextColor = RGB(255, 0, 0); // Red
                    SendMessage(pThis->hwndLogEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
                    
                    SendMessage(pThis->hwndLogEdit, EM_REPLACESEL, FALSE, (LPARAM)logBuffer);
                    delete pMessage;
                }
                return 0;
            }
            case WM_SIZE: {
                int clientWidth = LOWORD(lParam);
                int clientHeight = HIWORD(lParam);

                // Reposition and resize controls
                const int MARGIN = 10;
                const int CONTROL_HEIGHT = 25;
                const int BUTTON_WIDTH = 80;

                // Calculate widths with proportional scaling
                const float PORT_EDIT_WIDTH_RATIO = 0.2f; // Port edit takes 20% of available width
                int availableWidth = clientWidth - MARGIN * 4 - BUTTON_WIDTH;
                int portEditWidth = static_cast<int>(availableWidth * PORT_EDIT_WIDTH_RATIO);
                int ipEditWidth = availableWidth - portEditWidth;

                // IP edit and port edit controls
                MoveWindow(pThis->hwndIpEdit, MARGIN, MARGIN, ipEditWidth, CONTROL_HEIGHT, TRUE);
                MoveWindow(pThis->hwndPortEdit, MARGIN * 2 + ipEditWidth, MARGIN, portEditWidth, CONTROL_HEIGHT, TRUE);
                MoveWindow(pThis->hwndConnectBtn, clientWidth - MARGIN - BUTTON_WIDTH, MARGIN, BUTTON_WIDTH, CONTROL_HEIGHT, TRUE);

                // Message edit and send button
                int messageEditWidth = clientWidth - MARGIN * 3 - BUTTON_WIDTH;
                MoveWindow(pThis->hwndMessageEdit, MARGIN, MARGIN * 2 + CONTROL_HEIGHT, messageEditWidth, CONTROL_HEIGHT, TRUE);
                MoveWindow(pThis->hwndSendBtn, clientWidth - MARGIN - BUTTON_WIDTH, MARGIN * 2 + CONTROL_HEIGHT, BUTTON_WIDTH, CONTROL_HEIGHT, TRUE);

                // Log edit control
                int logEditHeight = clientHeight - (MARGIN * 4 + CONTROL_HEIGHT * 2);
                MoveWindow(pThis->hwndLogEdit, MARGIN, MARGIN * 3 + CONTROL_HEIGHT * 2, clientWidth - MARGIN * 2, logEditHeight, TRUE);
                return 0;
            }
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void Window::HandleCreate() {
    // Load Rich Edit library
    LoadLibrary(L"Msftedit.dll");

    // Controls are already created in Create()

    // Set up message callback
    tcpClient.SetMessageCallback([this](const std::string& message) {
        // Convert UTF-8 message to wide string
        int wideLength = MultiByteToWideChar(CP_UTF8, 0, message.c_str(), -1, NULL, 0);
        std::wstring wideMessage(wideLength, L'\0');
        MultiByteToWideChar(CP_UTF8, 0, message.c_str(), -1, &wideMessage[0], wideLength);

        // Format the log message
        std::wstring logMessage = L"Received: " + wideMessage;

        // Post message to main thread to update log
        PostMessage(hwnd, WM_APP + 1, (WPARAM)new std::wstring(logMessage), 0);
    });
}

void Window::HandleCommand(WPARAM wParam, LPARAM lParam) {
    int wmId = LOWORD(wParam);
    switch (wmId) {
        case IDC_CONNECT_BTN:
            if (!isConnected) {
                // Get IP and port from edit controls
                wchar_t ipBuffer[256];
                GetWindowText(hwndIpEdit, ipBuffer, 256);
                wchar_t portBuffer[32];
                GetWindowText(hwndPortEdit, portBuffer, 32);

                // Convert IP to string and port to integer
                char ipStr[256];
                WideCharToMultiByte(CP_UTF8, 0, ipBuffer, -1, ipStr, sizeof(ipStr), NULL, NULL);
                int port = _wtoi(portBuffer);

                // Try to connect
                if (tcpClient.Connect(ipStr, port)) {
                    isConnected = true;
                    SetWindowText(hwndConnectBtn, L"Disconnect");
                    
                    // Clear existing text
                    SetWindowText(hwndLogEdit, L"");
                    
                    // Set cursor to end and set blue color for connection status
                    int len = GetWindowTextLength(hwndLogEdit);
                    SendMessage(hwndLogEdit, EM_SETSEL, len, len);
                    
                    CHARFORMAT2 cf = {};
                    cf.cbSize = sizeof(CHARFORMAT2);
                    cf.dwMask = CFM_COLOR;
                    cf.crTextColor = RGB(0, 0, 255); // Blue
                    SendMessage(hwndLogEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
                    
                    // Add connection status message
                    SendMessage(hwndLogEdit, EM_REPLACESEL, FALSE, (LPARAM)L"Connected to server\r\n");
                    EnableWindow(hwndSendBtn, TRUE);
                } else {
                    // Clear existing text
                    SetWindowText(hwndLogEdit, L"");
                    
                    // Set cursor to end and set blue color for connection status
                    int len = GetWindowTextLength(hwndLogEdit);
                    SendMessage(hwndLogEdit, EM_SETSEL, len, len);
                    
                    CHARFORMAT2 cf = {};
                    cf.cbSize = sizeof(CHARFORMAT2);
                    cf.dwMask = CFM_COLOR;
                    cf.crTextColor = RGB(0, 0, 255); // Blue
                    SendMessage(hwndLogEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
                    
                    // Add connection status message
                    SendMessage(hwndLogEdit, EM_REPLACESEL, FALSE, (LPARAM)L"Failed to connect to server\r\n");
                }
            } else {
                // Disconnect
                if (tcpClient.Disconnect()) {
                    isConnected = false;
                    SetWindowText(hwndConnectBtn, L"Connect");
                    
                    // Clear existing text
                    SetWindowText(hwndLogEdit, L"");
                    
                    // Set cursor to end and set blue color for connection status
                    int len = GetWindowTextLength(hwndLogEdit);
                    SendMessage(hwndLogEdit, EM_SETSEL, len, len);
                    
                    CHARFORMAT2 cf = {};
                    cf.cbSize = sizeof(CHARFORMAT2);
                    cf.dwMask = CFM_COLOR;
                    cf.crTextColor = RGB(0, 0, 255); // Blue
                    SendMessage(hwndLogEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
                    
                    // Add connection status message
                    SendMessage(hwndLogEdit, EM_REPLACESEL, FALSE, (LPARAM)L"Disconnected from server\r\n");
                    EnableWindow(hwndSendBtn, FALSE);
                }
            }
            break;

        case IDC_SEND_BTN:
            if (isConnected) {
                // Get message from edit control
                wchar_t messageBuffer[1024];
                GetWindowText(hwndMessageEdit, messageBuffer, 1024);

                // Convert message to UTF-8
                char messageStr[1024];
                WideCharToMultiByte(CP_UTF8, 0, messageBuffer, -1, messageStr, sizeof(messageStr), NULL, NULL);

                // Send message
                if (tcpClient.Send(messageStr)) {
                    // Clear message edit control
                    SetWindowText(hwndMessageEdit, L"");

                    // Update log with sent message in green color
                    wchar_t logBuffer[1024];
                    swprintf(logBuffer, 1024, L"Sent: %s\r\n", messageBuffer);
                    int len = GetWindowTextLength(hwndLogEdit);
                    SendMessage(hwndLogEdit, EM_SETSEL, len, len);
                    
                    CHARFORMAT2 cf = {};
                    cf.cbSize = sizeof(CHARFORMAT2);
                    cf.dwMask = CFM_COLOR;
                    cf.crTextColor = RGB(0, 255, 0); // Green
                    SendMessage(hwndLogEdit, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
                    
                    SendMessage(hwndLogEdit, EM_REPLACESEL, FALSE, (LPARAM)logBuffer);
                } else {
                    MessageBox(hwnd, L"Failed to send message", L"Error", MB_ICONERROR);
                }
            }
            break;
    }
}

void Window::HandleDestroy() {
    PostQuitMessage(0);
}