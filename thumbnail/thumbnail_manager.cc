#include "thumbnail_manager.h"

#include <windows.h>

#include <algorithm>
#include <iostream>
#include <vector>

ThumbnailManager::ThumbnailManager(WindowSwitcher *window_switcher) {
    this->w_s = window_switcher;
    this->thumbnails = *(new std::vector<Thumbnail *>());
    this->thumbnails_comparing = *(new std::vector<Thumbnail *>());
}

bool ThumbnailManager::check_if_new_thumbnails_added() {
    std::cout << "checking thumbnails\n";
    this->destroy_all_comparing_thumbnails();
    EnumWindows(ThumbnailManager::comparing_collector_callback, reinterpret_cast<LPARAM>(w_s));
    if (this->thumbnails.size() != this->thumbnails_comparing.size()) {
        std::cout << "sizes not equal\n";
        return true;
    }
    for (int i = 0; i < this->thumbnails.size(); i++) {
        if (this->thumbnails[i]->self_hwnd != this->thumbnails_comparing[i]->self_hwnd) {
            std::cout << "hwnds not equal\n";
            return true;
        }
    }
    return false;
}

void ThumbnailManager::collect_all_thumbnails() {
    this->destroy_all_thumbnails();
    EnumWindows(ThumbnailManager::collector_callback, reinterpret_cast<LPARAM>(w_s));
    return;
}

void ThumbnailManager::calculate_all_thumbnails_positions() {
    this->window_width = w_s->monitor->get_width();
    int i = 0;
    int u = 0;
    this->widths.clear();
    while (i < this->thumbnails.size()) {
        int x = this->w_s->margin;
        while (1) {
            int add_to_x = (this->w_s->thumbnail_height * this->thumbnails[i]->ratio) + this->w_s->margin;
            if (x + add_to_x >= this->window_width) break;
            x += add_to_x;
            i++;
            if (i == this->thumbnails.size()) break;
        }
        this->widths.push_back(x);
    }

    i = 0;
    int x = (this->window_width - this->widths[u]) / 2;
    while (i < this->thumbnails.size()) {
        if (x + (this->w_s->thumbnail_height * this->thumbnails[i]->ratio) + this->w_s->margin > this->window_width) {
            u++;
            x = (this->window_width - this->widths[u]) / 2;
            continue;
        }
        this->thumbnails[i]->thumbnail_position.x = x + this->w_s->margin;
        this->thumbnails[i]->thumbnail_position.y = (this->w_s->thumbnail_height + this->w_s->margin + this->w_s->title_height) * u + this->w_s->margin;
        this->thumbnails[i]->thumbnail_position.width = this->w_s->thumbnail_height * this->thumbnails[i]->ratio;
        this->thumbnails[i]->thumbnail_position.height = this->w_s->thumbnail_height;

        x += (this->w_s->thumbnail_height * this->thumbnails[i]->ratio) + this->w_s->margin;
        i++;
    }
    this->window_height = (this->w_s->thumbnail_height + this->w_s->margin + this->w_s->title_height) * (u + 1) + this->w_s->margin;
    return;
}

void ThumbnailManager::update_all_windows_positions() {
    for (Thumbnail *i : this->thumbnails) {
        i->update_window_position();
    }
    return;
}

void ThumbnailManager::register_all_thumbnails() {
    for (Thumbnail *i : this->thumbnails) {
        i->register_thumbnail();
    }

    return;
}

void ThumbnailManager::destroy_all_thumbnails() {
    for (auto i : this->thumbnails) {
        delete i;
    }
    this->thumbnails.clear();
    return;
}

void ThumbnailManager::destroy_all_comparing_thumbnails() {
    for (auto i : this->thumbnails_comparing) {
        delete i;
    }
    this->thumbnails_comparing.clear();
    return;
}

void ThumbnailManager::update_thumbnails_if_needed() {
    if (this->check_if_new_thumbnails_added()) {
        this->collect_all_thumbnails();
        this->update_all_windows_positions();
        this->calculate_all_thumbnails_positions();
        this->register_all_thumbnails();
        // WindowSwitcher::resize_window();
    }
    return;
}

BOOL CALLBACK ThumbnailManager::collector_callback(HWND hwnd, LPARAM lParam) {
    if (!IsWindowVisible(hwnd)) return TRUE;
    RECT rect;
    GetWindowRect(hwnd, &rect);
    if (((rect.right - rect.left == 0) || rect.bottom - rect.top == 0) || ((rect.right - rect.left < 0) || rect.bottom - rect.top < 0)) {
        return TRUE;
    }
    if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW && !(GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_APPWINDOW)) return TRUE;

    if (GetWindow(hwnd, GW_OWNER) != NULL) return TRUE;

    HWND hwndTry, hwndWalk = NULL;
    hwndTry = GetAncestor(hwnd, GA_ROOTOWNER);
    while (hwndTry != hwndWalk) {
        hwndWalk = hwndTry;
        hwndTry = GetLastActivePopup(hwndWalk);
        if (IsWindowVisible(hwndTry)) break;
    }
    if (hwndWalk != hwnd) return TRUE;

    WindowSwitcher *w_s = reinterpret_cast<WindowSwitcher *>(lParam);

    for (auto i : *(w_s->window_switchers)) {
        if (hwnd == i->hwnd) return TRUE;
    }

    int is_cloaked;
    DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &is_cloaked, sizeof(int));
    if (is_cloaked) return TRUE;
    auto on_monitor_pair = MonitorResolver::monitors.find(MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST));
    HMONITOR on_monitor = (HMONITOR)-1;
    if (on_monitor_pair != MonitorResolver::monitors.end()) on_monitor = on_monitor_pair->first;
    if (on_monitor != w_s->monitor->handle) return TRUE;

    w_s->thumbnail_manager->thumbnails.push_back(new Thumbnail(hwnd, w_s->hwnd, w_s->thumbnail_manager->thumbnails.size(), on_monitor));
    return TRUE;
}

BOOL CALLBACK ThumbnailManager::comparing_collector_callback(HWND hwnd, LPARAM lParam) {
    if (!IsWindowVisible(hwnd)) return TRUE;
    RECT rect;
    GetWindowRect(hwnd, &rect);
    if (((rect.right - rect.left == 0) || rect.bottom - rect.top == 0) || ((rect.right - rect.left < 0) || rect.bottom - rect.top < 0)) {
        return TRUE;
    }
    if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW && !(GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_APPWINDOW)) return TRUE;

    if (GetWindow(hwnd, GW_OWNER) != NULL) return TRUE;

    HWND hwndTry, hwndWalk = NULL;
    hwndTry = GetAncestor(hwnd, GA_ROOTOWNER);
    while (hwndTry != hwndWalk) {
        hwndWalk = hwndTry;
        hwndTry = GetLastActivePopup(hwndWalk);
        if (IsWindowVisible(hwndTry)) break;
    }

    if (hwndWalk != hwnd) return TRUE;

    WindowSwitcher *w_s = reinterpret_cast<WindowSwitcher *>(lParam);

    for (auto i : *(w_s->window_switchers)) {
        if (hwnd == i->hwnd) return TRUE;
    }

    int is_cloaked;
    DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &is_cloaked, sizeof(int));
    if (is_cloaked) return TRUE;

    auto on_monitor_pair = MonitorResolver::monitors.find(MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST));
    HMONITOR on_monitor = (HMONITOR)-1;
    if (on_monitor_pair != MonitorResolver::monitors.end()) on_monitor = on_monitor_pair->first;
    if (on_monitor != w_s->monitor->handle) return TRUE;

    w_s->thumbnail_manager->thumbnails_comparing.push_back(new Thumbnail(hwnd, w_s->hwnd, w_s->thumbnail_manager->thumbnails_comparing.size(), on_monitor));
    return TRUE;
}