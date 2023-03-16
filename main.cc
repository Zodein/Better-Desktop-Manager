#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "window_switcher/window_switcher.h"

int main() {
    WindowSwitcher::wc.cbSize = sizeof(WNDCLASSEX);
    WindowSwitcher::wc.style = 0;
    WindowSwitcher::wc.lpfnWndProc = WindowSwitcher::window_proc_static;
    WindowSwitcher::wc.cbClsExtra = 0;
    WindowSwitcher::wc.cbWndExtra = 0;
    WindowSwitcher::wc.hInstance = NULL;
    WindowSwitcher::wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WindowSwitcher::wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    WindowSwitcher::wc.hbrBackground = WindowSwitcher::null_brush;
    WindowSwitcher::wc.lpszMenuName = NULL;
    WindowSwitcher::wc.lpszClassName = L"class";
    WindowSwitcher::wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    WindowSwitcher::wc_child = WindowSwitcher::wc;
    WindowSwitcher::wc_child.lpfnWndProc = WindowSwitcher::window_proc_child_static;
    WindowSwitcher::wc_child.hbrBackground = WindowSwitcher::background_brush;
    WindowSwitcher::wc_child.lpszClassName = L"class_child";
    if (!RegisterClassEx(&WindowSwitcher::wc) || !RegisterClassEx(&WindowSwitcher::wc_child)) {
        MessageBox(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    MonitorResolver::update_monitors_data();
    std::vector<WindowSwitcher *> window_switchers;

    for (auto i : MonitorResolver::monitors) {
        WindowSwitcher *window_switcher = new WindowSwitcher(i.second, &window_switchers);
        std::thread t1([window_switcher]() { window_switcher->create_window(); });
        t1.detach();
        window_switchers.push_back(window_switcher);
    }

    auto x = RegisterHotKey(NULL, 1, MOD_NOREPEAT, 0x42);
    auto y = GetLastError();

    MSG message;
    while (GetMessage(&message, NULL, 0, 0) != 0) {
        switch (message.message) {
            case WM_HOTKEY:
                std::cout << "thread hotkey pressed\n";
                for (auto i : window_switchers) {
                    SendMessage(i->hwnd, WM_HOTKEY, 0, 0);
                }
        }
    }
    return 0;
}