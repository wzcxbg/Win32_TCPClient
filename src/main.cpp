#define UNICODE
#define _UNICODE
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "window.h"
#include "splash_window.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    // Create and show splash window
    SplashWindow splash;
    if (!splash.Create()) {
        MessageBox(NULL, L"Failed to create splash window", L"Error", MB_ICONERROR);
        return 1;
    }
    splash.Show();

    // Wait for 2 seconds
    Sleep(2000);

    // Close splash window
    splash.Close();

    // Create and show main window
    Window window;
    if (!window.Create()) {
        MessageBox(NULL, L"Failed to create window", L"Error", MB_ICONERROR);
        return 1;
    }

    window.Show(nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}