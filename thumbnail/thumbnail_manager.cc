#include "thumbnail_manager.h"

#include <windows.h>

#include <algorithm>
#include <iostream>
#include <numeric>
#include <vector>

ThumbnailManager::ThumbnailManager(WindowSwitcher *window_switcher, int window_width, int window_height) {
    this->w_s = window_switcher;
    this->window_width = window_width;
    this->window_height = window_height;
    this->vt_height = (this->w_s->monitor->get_height() * 0.25) - (this->w_s->virtual_desktop_vertical_margin * 2);

    this->thumbnails = *(new std::vector<Thumbnail *>());
    this->thumbnails_comparing = *(new std::vector<Thumbnail *>());
}

bool ThumbnailManager::check_if_new_thumbnails_added() {
    // std::cout << "checking thumbnails\n";
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
    this->clear_virtualdesktop_windowcount();
    this->update_virtualdesktops_if_needed();
    EnumWindows(ThumbnailManager::collector_callback, reinterpret_cast<LPARAM>(w_s));
    return;
}

void ThumbnailManager::clear_virtualdesktop_windowcount() {
    for (auto i : this->virtual_desktops) {
        i.second->window_count = 0;
    }
}

bool ThumbnailManager::update_virtualdesktops_if_needed(bool force) {
    IObjectArray *desktops;
    UINT size = 0;
    IVirtualDesktop *desktop;
    GUID guid;
    WCHAR id[256];
    bool is_active = false;
    auto hr = VirtualDesktopManager::desktop_manager_internal->GetDesktops(0, &desktops);
    if (SUCCEEDED(hr)) {
        hr = desktops->GetCount(&size);
        if (SUCCEEDED(hr) && (force || this->virtual_desktops.size() != size)) {
            this->virtual_desktops.clear();
            this->virtual_desktops_index.clear();
            for (int i = 0; i < size; i++) {
                desktops->GetAt(i, IID_IVirtualDesktop, (void **)&desktop);
                desktop->GetID(&guid);
                hr = StringFromGUID2(guid, id, _countof(id));
                if (SUCCEEDED(hr)) {
                    is_active = std::wstring(id).compare(VirtualDesktopManager::get_current_desktop_guid_as_string()) == 0;
                    VirtualDesktop *virtual_desktop = new VirtualDesktop(std::wstring(id), i, 0, desktop, is_active);
                    this->virtual_desktops.insert({std::wstring(id), virtual_desktop});
                    this->virtual_desktops_index.insert({i, std::wstring(id)});
                }
            }
            desktops->Release();
            this->calculate_all_virtualdesktops_positions();
        }
    }
    return false;
}

void ThumbnailManager::calculate_all_virtualdesktops_positions() {
    for (auto i : this->virtual_desktops_index) {
        auto virtual_desktop = this->virtual_desktops.find(i.second);
        if (virtual_desktop != this->virtual_desktops.end()) {
            auto _sizes = this->w_s->monitor->vt_size;

            virtual_desktop->second->render_left = _sizes->reference_x + (i.first * (_sizes->width + _sizes->h_margin)) - (this->w_s->virtual_desktop_scroll);
            virtual_desktop->second->render_top = _sizes->reference_y;
            virtual_desktop->second->render_right = virtual_desktop->second->render_left + _sizes->width;
            virtual_desktop->second->render_bottom = virtual_desktop->second->render_top + _sizes->height;
        }
    }
}

void ThumbnailManager::calculate_all_thumbnails_positions(double extra_ratio) {
    int i = 0;
    int u = 0;
    this->widths.clear();
    if (this->thumbnails.size() == 0) return;
    {  // precalculations
        while (i < this->thumbnails.size()) {
            if ((this->w_s->thumbnail_height * this->thumbnails[i]->ratio * extra_ratio) > this->window_width) {
                this->widths.push_back(this->window_width);
                i++;
            } else {
                int x = this->w_s->margin;
                while (1) {
                    int reference_x = (this->w_s->thumbnail_height + this->w_s->margin) * this->thumbnails[i]->ratio * extra_ratio;
                    if (x + reference_x >= this->window_width) break;
                    x += reference_x;
                    i++;
                    if (i == this->thumbnails.size()) break;
                }
                this->widths.push_back(x);
            }
        }

        if (((this->w_s->thumbnail_height + this->w_s->margin + this->w_s->title_height) * this->widths.size() * extra_ratio) > (this->window_height * 0.75)) {
            return this->calculate_all_thumbnails_positions(extra_ratio * (double)(0.9));
        }
    }

    i = 0;
    int reference_x = (this->w_s->monitor->get_width() - this->widths[u]) / 2;
    int reference_y = 0;
    {
        reference_y = this->window_height * 0.75;
        reference_y -= (this->w_s->thumbnail_height + this->w_s->margin + this->w_s->title_height) * (this->widths.size()) * extra_ratio;
        reference_y /= 2;
    }
    int y = 0;
    while (i < this->thumbnails.size()) {
        if (reference_x + (this->w_s->thumbnail_height * this->thumbnails[i]->ratio * extra_ratio) + this->w_s->margin > ((this->w_s->monitor->get_width() / 2) + (this->window_width / 2)) && y != 0) {
            u++;
            y = 0;
            reference_x = (this->w_s->monitor->get_width() - this->widths[u]) / 2;
            continue;
        }
        {
            this->thumbnails[i]->thumbnail_position.x = 0;
            this->thumbnails[i]->thumbnail_position.y = (this->w_s->thumbnail_height + this->w_s->margin) * u * extra_ratio + (this->w_s->title_height * u);
            this->thumbnails[i]->thumbnail_position.width = this->w_s->thumbnail_height * this->thumbnails[i]->ratio * extra_ratio;
            this->thumbnails[i]->thumbnail_position.height = this->w_s->thumbnail_height * extra_ratio;
        }
        {
            this->thumbnails[i]->thumbnail_position.x += this->w_s->margin;
            this->thumbnails[i]->thumbnail_position.x += reference_x;

            this->thumbnails[i]->thumbnail_position.y += this->w_s->margin;
            this->thumbnails[i]->thumbnail_position.y += reference_y;
        }

        if ((this->thumbnails[i]->window_position.width > this->window_width || this->thumbnails[i]->window_position.height > (this->window_height * 0.75)) && this->thumbnails[i]->thumbnail_position.width >= this->window_width) {  // if thumbnail is too large to fit on screen
            double new_ratio = (double)(this->window_width - ((this->w_s->margin * 2))) / (double)this->thumbnails[i]->thumbnail_position.width;
            this->thumbnails[i]->thumbnail_position.width *= new_ratio;
            this->thumbnails[i]->thumbnail_position.height *= new_ratio;
            this->thumbnails[i]->thumbnail_position.y += ((this->w_s->thumbnail_height * extra_ratio) - (this->w_s->thumbnail_height * new_ratio)) / 2;
        } else if (this->thumbnails[i]->thumbnail_position.width > this->thumbnails[i]->window_position.width && this->thumbnails[i]->thumbnail_position.height > this->thumbnails[i]->window_position.height) {  // if thumbnail is larger than its window
            this->thumbnails[i]->thumbnail_position.width = this->thumbnails[i]->window_position.width;
            this->thumbnails[i]->thumbnail_position.height = this->thumbnails[i]->window_position.height;
            this->thumbnails[i]->thumbnail_position.y += ((this->w_s->thumbnail_height * extra_ratio) - this->thumbnails[i]->window_position.height) / 2;
        }
        this->thumbnails[i]->thumbnail_position.y += this->w_s->title_height;

        reference_x += this->thumbnails[i]->thumbnail_position.width + this->w_s->margin;
        i++;
        y++;
    }

    return;
}

void ThumbnailManager::update_all_thumbnails_positions() {
    for (Thumbnail *i : this->thumbnails) {
        i->update_thumbnail_position();
    }
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
        i->unregister_thumbnail();
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

bool ThumbnailManager::update_thumbnails_if_needed(bool force) {
    if (force || this->check_if_new_thumbnails_added()) {
        this->collect_all_thumbnails();
        if (this->thumbnails.size() < 1) {
            return false;
        }
        this->update_all_windows_positions();
        this->calculate_all_thumbnails_positions();
        this->register_all_thumbnails();
        InvalidateRect(this->w_s->hwnd, NULL, FALSE);
        return true;
    }
    return false;
}

BOOL CALLBACK ThumbnailManager::collector_callback(HWND hwnd, LPARAM lParam) {
    if (!IsWindowVisible(hwnd)) return TRUE;
    WindowSwitcher *w_s = reinterpret_cast<WindowSwitcher *>(lParam);

    for (auto i : *(w_s->window_switchers)) {
        if (hwnd == i->hwnd) return TRUE;
    }

    auto on_monitor_pair = MonitorResolver::monitors.find(MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST));
    HMONITOR on_monitor = (HMONITOR)-1;
    if (on_monitor_pair != MonitorResolver::monitors.end()) on_monitor = on_monitor_pair->first;
    if (on_monitor != w_s->monitor->handle) return TRUE;

    UINT shown = 0;
    IApplicationView *app_view;
    GUID desktop_guid;
    auto hr = VirtualDesktopManager::application_view_collection->GetViewForHwnd(hwnd, &app_view);
    if (SUCCEEDED(hr)) {
        app_view->GetShowInSwitchers(&shown);
        if (shown != 0) {
            app_view->GetVirtualDesktopId(&desktop_guid);
            WCHAR id[256];
            hr = StringFromGUID2(desktop_guid, id, _countof(id));
            auto temp = w_s->thumbnail_manager->virtual_desktops.find(std::wstring(id));
            if (temp != w_s->thumbnail_manager->virtual_desktops.end()) {
                temp->second->window_count++;
            }
        }
        app_view->Release();
    }

    int is_cloaked;
    DwmGetWindowAttribute(hwnd, DWMWA_CLOAKED, &is_cloaked, sizeof(int));
    if (is_cloaked) {
        return TRUE;
    }

    if (shown != 0) {
        w_s->thumbnail_manager->thumbnails.push_back(new Thumbnail(hwnd, w_s->hwnd, w_s->thumbnail_manager->thumbnails.size(), w_s->monitor));
    }

    return TRUE;
}

BOOL CALLBACK ThumbnailManager::comparing_collector_callback(HWND hwnd, LPARAM lParam) {
    if (!IsWindowVisible(hwnd)) return TRUE;
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

    UINT shown = 0;
    IApplicationView *app_view;
    auto hr = VirtualDesktopManager::application_view_collection->GetViewForHwnd(hwnd, &app_view);
    if (SUCCEEDED(hr)) {
        app_view->GetShowInSwitchers(&shown);
        if (shown != 0) {
            w_s->thumbnail_manager->thumbnails_comparing.push_back(new Thumbnail(hwnd, w_s->hwnd, w_s->thumbnail_manager->thumbnails_comparing.size(), w_s->monitor));
        }
        app_view->Release();
    }
    return TRUE;
}