#include "window_switcher.h"

#include <thread>

#include "window_switcher_gui.h"

ThumbnailManager *WindowSwitcher::thumbnail_manager = new ThumbnailManager(32, 384);
void WindowSwitcher::create_gui() {
    std::thread t1(WindowSwitcherGUI::create_window);

    while (WindowSwitcherGUI::hwnd == 0) {
    }

    WindowSwitcher::thumbnail_manager->collect_all_thumbnails();
    WindowSwitcher::thumbnail_manager->update_all_windows_positions();
    WindowSwitcher::thumbnail_manager->calculate_all_thumbnails_positions();
    WindowSwitcher::thumbnail_manager->register_all_thumbnails();
    // WindowSwitcherGUI::resize(0, 0, WindowSwitcher::thumbnail_manager->window_width, WindowSwitcher::thumbnail_manager->window_height);
    WindowSwitcherGUI::show_window();

    while (1) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    return;
}
