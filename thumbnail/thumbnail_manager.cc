#include "thumbnail_manager.h"

#include <windows.h>

#include <algorithm>
#include <iostream>
#include <vector>

#include "../monitor_resolver/monitor_resolver.h"
#include "../window_switcher/window_switcher.h"

ThumbnailManager::ThumbnailManager(int margin, int thumbnail_height) {
    this->margin = margin;
    this->thumbnail_height = thumbnail_height;
}

bool ThumbnailManager::check_if_new_thumbnails_added() {
    std::cout << "checking thumbnails\n";
    this->destroy_all_comparing_thumbnails();
    EnumWindows(ThumbnailManager::collector_callback, reinterpret_cast<LPARAM>(&this->thumbnails_comparing));
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
    EnumWindows(ThumbnailManager::collector_callback, reinterpret_cast<LPARAM>(&this->thumbnails));
    return;
}

void ThumbnailManager::calculate_all_thumbnails_positions() {
    this->window_width = MonitorResolver::selected_monitor->get_width();
    int i = 0;
    int u = 0;
    this->widths.clear();
    while (i < this->thumbnails.size()) {
        int x = this->margin;
        while (1) {
            int add_to_x = (this->thumbnail_height * this->thumbnails[i]->ratio) + this->margin;
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
        if (x + (this->thumbnail_height * this->thumbnails[i]->ratio) + this->margin > this->window_width) {
            u++;
            x = (this->window_width - this->widths[u]) / 2;
            continue;
        }
        this->thumbnails[i]->thumbnail_position.x = x + this->margin;
        this->thumbnails[i]->thumbnail_position.y = (this->thumbnail_height + this->margin + WindowSwitcher::title_height) * u + this->margin;
        this->thumbnails[i]->thumbnail_position.width = this->thumbnail_height * this->thumbnails[i]->ratio;
        this->thumbnails[i]->thumbnail_position.height = this->thumbnail_height;

        x += (this->thumbnail_height * this->thumbnails[i]->ratio) + this->margin;
        i++;
    }
    this->window_height = (this->thumbnail_height + this->margin + WindowSwitcher::title_height) * (u + 1) + this->margin;
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
    if (WindowSwitcher::thumbnail_manager->check_if_new_thumbnails_added()) {
        WindowSwitcher::thumbnail_manager->collect_all_thumbnails();
        WindowSwitcher::thumbnail_manager->update_all_windows_positions();
        WindowSwitcher::thumbnail_manager->calculate_all_thumbnails_positions();
        WindowSwitcher::thumbnail_manager->register_all_thumbnails();
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
    if (hwnd == WindowSwitcher::hwnd) return TRUE;
    int is_cloaked;
    DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &is_cloaked, sizeof(int));
    if (is_cloaked) return TRUE;

    std::vector<Thumbnail *> *thumbnails = reinterpret_cast<std::vector<Thumbnail *> *>(lParam);
    thumbnails->push_back(new Thumbnail(hwnd, WindowSwitcher::hwnd, thumbnails->size()));
    return TRUE;
}