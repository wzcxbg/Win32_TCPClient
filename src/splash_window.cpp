#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "splash_window.h"

SplashWindow::SplashWindow() : hwnd(NULL) {}

SplashWindow::~SplashWindow() {
    if (hwnd) {
        DestroyWindow(hwnd);
    }
}

bool SplashWindow::Create() {
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = L"SplashWindowClass";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClassEx(&wc)) {
        return false;
    }

    // Get screen dimensions for centering
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int xPos = (screenWidth - WINDOW_WIDTH) / 2;
    int yPos = (screenHeight - WINDOW_HEIGHT) / 2;

    // Create popup window without border
    hwnd = CreateWindowEx(
        WS_EX_TOPMOST,           // Always on top
        L"SplashWindowClass",    // Window class
        L"Splash",              // Window text
        WS_POPUP,               // Popup window without border
        xPos, yPos,             // Position
        WINDOW_WIDTH,           // Width
        WINDOW_HEIGHT,          // Height
        NULL,                   // Parent window
        NULL,                   // Menu
        GetModuleHandle(NULL),  // Instance handle
        this                    // Additional application data
    );

    if (hwnd == NULL) {
        return false;
    }

    return true;
}

void SplashWindow::Show() {
    if (hwnd) {
        ShowWindow(hwnd, SW_SHOW);
        UpdateWindow(hwnd);
    }
}

void SplashWindow::Close() {
    if (hwnd) {
        DestroyWindow(hwnd);
        hwnd = NULL;
    }
}

LRESULT CALLBACK SplashWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    SplashWindow* pThis = NULL;

    if (uMsg == WM_NCCREATE) {
        CREATESTRUCT* pCreate = (CREATESTRUCT*)lParam;
        pThis = (SplashWindow*)pCreate->lpCreateParams;
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)pThis);
    } else {
        pThis = (SplashWindow*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            // Set up the text
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(0, 0, 0));
            SelectObject(hdc, GetStockObject(DEFAULT_GUI_FONT));

            // Draw the text
            RECT rect;
            GetClientRect(hwnd, &rect);
            const wchar_t* text = L"TCP Client";
            DrawText(hdc, text, -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DESTROY:
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}