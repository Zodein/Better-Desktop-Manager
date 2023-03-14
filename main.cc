#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "window_switcher/window_switcher.h"

int main() {
    WindowSwitcher::create_gui();
    while (1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}