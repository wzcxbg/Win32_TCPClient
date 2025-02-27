#ifndef SPLASH_WINDOW_H
#define SPLASH_WINDOW_H

#include <windows.h>

class SplashWindow {
public:
    SplashWindow();
    ~SplashWindow();

    bool Create();
    void Show();
    void Close();
    HWND GetHandle() const { return hwnd; }

    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    HWND hwnd;

    // Window dimensions
    static constexpr int WINDOW_WIDTH = 400;
    static constexpr int WINDOW_HEIGHT = 250;

    // Prevent copying
    SplashWindow(const SplashWindow&) = delete;
    SplashWindow& operator=(const SplashWindow&) = delete;
};

#endif // SPLASH_WINDOW_H