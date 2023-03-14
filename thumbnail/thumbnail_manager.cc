#include "thumbnail_manager.h"

#include <windows.h>

#include <algorithm>
#include <iostream>
#include <vector>

#include "../window_switcher/window_switcher_gui.h"

ThumbnailManager::ThumbnailManager(int margin, int thumbnail_height) {
    this->margin = margin;
    this->thumbnail_height = thumbnail_height;
    this->max_width = (3440 / 4 * 3);
}

void ThumbnailManager::collect_all_thumbnails() {
    EnumWindows(ThumbnailManager::collector_callback, reinterpret_cast<LPARAM>(this));
    return;
}

void ThumbnailManager::calculate_all_thumbnails_positions() {
    int i = 0;
    int u = 0;
    this->widths.clear();
    while (i < this->thumbnails.size()) {
        int x = this->margin;
        while (1) {
            int add_to_x = (this->thumbnail_height * this->thumbnails[i]->ratio) + this->margin;
            if (x + add_to_x >= max_width) break;
            x += add_to_x;
            i++;
            if (i == this->thumbnails.size()) break;
        }
        this->widths.push_back(x);
        x = this->margin / 2;
    }

    i = 0;
    this->window_width = (*max_element(std::begin(this->widths), std::end(this->widths)));
    int x = (this->window_width - this->widths[u]) / 2;
    while (i < this->thumbnails.size()) {
        if (x + this->margin + this->thumbnails[i]->thumbnail_position.width > this->widths[u]) {
            u++;
            x = (this->window_width - this->widths[u]) / 2;
            continue;
        }
        this->thumbnails[i]->thumbnail_position.x = x + this->margin;
        this->thumbnails[i]->thumbnail_position.y = (this->thumbnail_height + this->margin) * u + this->margin;
        this->thumbnails[i]->thumbnail_position.width = this->thumbnail_height * this->thumbnails[i]->ratio;
        this->thumbnails[i]->thumbnail_position.height = this->thumbnail_height;

        x += (this->thumbnail_height * this->thumbnails[i]->ratio) + this->margin;
        i++;
    }
    this->window_height = (this->thumbnail_height + this->margin) * (u + 1) + this->margin;
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

BOOL CALLBACK ThumbnailManager::collector_callback(HWND hwnd, LPARAM lParam) {
    // if (IsIconic(hwnd)) return TRUE;

    if (!IsWindowVisible(hwnd)) return TRUE;

    RECT rect;
    GetWindowRect(hwnd, &rect);
    if (((rect.right - rect.left == 0) || rect.bottom - rect.top == 0) || ((rect.right - rect.left < 0) || rect.bottom - rect.top < 0)) {
        return TRUE;
    }

    if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_TOOLWINDOW && !(GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_APPWINDOW)) return TRUE;

    // TITLEBARINFO ti;
    // ti.cbSize = sizeof(ti);
    // GetTitleBarInfo(hwnd, &ti);
    // if (ti.rgstate[0] & STATE_SYSTEM_INVISIBLE) return TRUE;

    // HWND hwndTry, hwndWalk = NULL;
    // hwndTry = GetAncestor(hwnd, GA_ROOTOWNER);
    // while (hwndTry != hwndWalk) {
    //     hwndWalk = hwndTry;
    //     hwndTry = GetLastActivePopup(hwndWalk);
    //     if (IsWindowVisible(hwndTry)) break;
    // }
    // if (hwndWalk != hwnd) return TRUE;

    if (hwnd == WindowSwitcherGUI::hwnd) return TRUE;

    int is_cloaked;
    DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &is_cloaked, sizeof(int));
    if (is_cloaked) return TRUE;

    wchar_t wnd_title[256];
    GetWindowText(hwnd, wnd_title, 256);
    std::wcout << wnd_title << L" - MINIMIZED: " << hwnd << L"\n";

    ThumbnailManager *thumbnail_manager = reinterpret_cast<ThumbnailManager *>(lParam);
    thumbnail_manager->thumbnails.push_back(new Thumbnail(hwnd, WindowSwitcherGUI::hwnd, thumbnail_manager->thumbnails.size()));
    return TRUE;
}