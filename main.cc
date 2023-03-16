#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "window_switcher/window_switcher.h"

int main() {
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

    while (1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return 0;
}