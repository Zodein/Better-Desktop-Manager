#include "v_desktop_manager.h"

#include <windows.h>
#include <winuser.h>

#include <algorithm>
#include <iostream>
#include <numeric>
#include <vector>

#include "v_desktop.h"

VDesktopManager::VDesktopManager(CommandCenter *command_center, int window_width, int window_height) {
    this->c_s = command_center;
    this->window_width = window_width;
    this->window_height = window_height;
    *this->active_desktop = NULL;
}

void VDesktopManager::update_current_desktop() {
    auto temp = this->virtual_desktops.find(VDesktopAPI::get_current_desktop_guid_as_string());
    if (temp != this->virtual_desktops.end()) {
        if (*this->active_desktop == temp->second) return;
        *this->active_desktop = temp->second;
        {
            std::lock_guard<std::mutex> lock1(this->c_s->thumbnail_destroyer_lock);
            std::lock_guard<std::mutex> lock(this->c_s->render_lock);
            this->unregister_thumbnails();
            this->calculate_thumbnails_pose();
            this->register_thumbnails();
        }
        return;
    } else {
        *this->active_desktop = NULL;
        return;
    }
}

void VDesktopManager::refresh_thumbnails() {
    for (auto i : this->virtual_desktops) {
        if (i.second->stale_data) {
            for (auto u : i.second->windows) {
                delete u;
            }
            i.second->windows.clear();
        }
    }
    EnumWindows(VDesktopManager::collector_callback, reinterpret_cast<LPARAM>(c_s));
    this->calculate_thumbnails_pose();
    this->register_thumbnails();
    for (auto i : this->virtual_desktops) {
        if (i.second->stale_data) {
            i.second->stale_data = false;
        }
    }
}

void VDesktopManager::refresh_v_desktops() {
    IObjectArray *desktops;
    UINT size = 0;
    IVirtualDesktop *desktop;
    GUID guid;
    WCHAR id[256];
    bool is_active = false;
    if (SUCCEEDED(VDesktopAPI::desktop_manager_internal->GetDesktops(0, &desktops))) {
        if (SUCCEEDED(desktops->GetCount(&size))) {
            *this->active_desktop = NULL;
            for (auto i : this->virtual_desktops) {
                delete i.second;
            }
            this->virtual_desktops.clear();
            this->virtual_desktops_index.clear();
            for (int i = 0; i < size; i++) {
                desktops->GetAt(i, IID_IVirtualDesktop, (void **)&desktop);
                desktop->GetID(&guid);
                if (SUCCEEDED(StringFromGUID2(guid, id, _countof(id)))) {
                    is_active = std::wstring(id).compare(VDesktopAPI::get_current_desktop_guid_as_string()) == 0;
                    VirtualDesktop *virtual_desktop = new VirtualDesktop(std::wstring(id), i, desktop, is_active);
                    this->virtual_desktops.insert({std::wstring(id), virtual_desktop});
                    this->virtual_desktops_index.insert({i, std::wstring(id)});
                }
            }
            this->virtual_desktops.insert({std::wstring(L"add_desktop"), new VirtualDesktop(std::wstring(L"add_desktop"), virtual_desktops.size(), nullptr, false)});
            this->virtual_desktops_index.insert({virtual_desktops_index.size(), std::wstring(L"add_desktop")});
            this->calculate_vdesktops_pose();
            desktops->Release();
        }
    }
    return;
}

void VDesktopManager::calculate_vdesktops_pose() {
    for (auto i : this->virtual_desktops_index) {
        auto virtual_desktop = this->virtual_desktops.find(i.second);
        if (virtual_desktop != this->virtual_desktops.end()) {
            auto _sizes = this->c_s->monitor->vt_size;

            virtual_desktop->second->render_left = _sizes->reference_x + (i.first * (_sizes->width + _sizes->h_margin)) - (this->c_s->virtual_desktop_scroll);
            virtual_desktop->second->render_top = _sizes->reference_y;
            virtual_desktop->second->render_right = virtual_desktop->second->render_left + _sizes->width;
            virtual_desktop->second->render_bottom = virtual_desktop->second->render_top + _sizes->height;
        }
    }
}

void VDesktopManager::calculate_thumbnails_pose(double extra_ratio) {
    if (!(*this->active_desktop)) return;
    this->update_windows_pose();
    int i = 0;
    int u = 0;
    this->widths.clear();
    if ((*this->active_desktop)->windows.size() == 0) return;
    {
        while (i < (*this->active_desktop)->windows.size()) {
            if ((this->c_s->thumbnail_height * (*this->active_desktop)->windows[i]->ratio * extra_ratio) > this->window_width) {
                this->widths.push_back(this->window_width);
                i++;
            } else {
                int x = this->c_s->margin;
                while (1) {
                    int reference_x = (this->c_s->thumbnail_height + this->c_s->margin) * (*this->active_desktop)->windows[i]->ratio * extra_ratio;
                    if (x + reference_x >= this->window_width) break;
                    x += reference_x;
                    i++;
                    if (i == (*this->active_desktop)->windows.size()) break;
                }
                this->widths.push_back(x);
            }
        }

        if (((this->c_s->thumbnail_height + this->c_s->margin + this->c_s->title_height) * this->widths.size() * extra_ratio) > (this->window_height * 0.75)) {
            return this->calculate_thumbnails_pose(extra_ratio * (double)(0.9));
        }
    }

    i = 0;
    int reference_x = (this->c_s->monitor->get_width() - this->widths[u]) / 2;
    int reference_y = 0;
    {
        reference_y = this->window_height * 0.75;
        reference_y -= (this->c_s->thumbnail_height + this->c_s->margin + this->c_s->title_height) * (this->widths.size()) * extra_ratio;
        reference_y /= 2;
    }
    int y = 0;
    while (i < (*this->active_desktop)->windows.size()) {
        if (reference_x + (this->c_s->thumbnail_height * (*this->active_desktop)->windows[i]->ratio * extra_ratio) + this->c_s->margin > ((this->c_s->monitor->get_width() / 2) + (this->window_width / 2)) && y != 0) {
            u++;
            y = 0;
            reference_x = (this->c_s->monitor->get_width() - this->widths[u]) / 2;
            continue;
        }
        {
            (*this->active_desktop)->windows[i]->thumbnail_position.x = 0;
            (*this->active_desktop)->windows[i]->thumbnail_position.y = (this->c_s->thumbnail_height + this->c_s->margin) * u * extra_ratio + (this->c_s->title_height * u);
            (*this->active_desktop)->windows[i]->thumbnail_position.width = this->c_s->thumbnail_height * (*this->active_desktop)->windows[i]->ratio * extra_ratio;
            (*this->active_desktop)->windows[i]->thumbnail_position.height = this->c_s->thumbnail_height * extra_ratio;
        }
        {
            (*this->active_desktop)->windows[i]->thumbnail_position.x += this->c_s->margin;
            (*this->active_desktop)->windows[i]->thumbnail_position.x += reference_x;

            (*this->active_desktop)->windows[i]->thumbnail_position.y += this->c_s->margin;
            (*this->active_desktop)->windows[i]->thumbnail_position.y += reference_y;
        }

        if (((*this->active_desktop)->windows[i]->window_position.width > this->window_width || (*this->active_desktop)->windows[i]->window_position.height > (this->window_height * 0.75)) && (*this->active_desktop)->windows[i]->thumbnail_position.width >= this->window_width) {  // if thumbnail is too large to fit on screen
            double new_ratio = (double)(this->window_width - ((this->c_s->margin * 2))) / (double)(*this->active_desktop)->windows[i]->thumbnail_position.width;
            (*this->active_desktop)->windows[i]->thumbnail_position.width *= new_ratio;
            (*this->active_desktop)->windows[i]->thumbnail_position.height *= new_ratio;
            (*this->active_desktop)->windows[i]->thumbnail_position.y += ((this->c_s->thumbnail_height * extra_ratio) - (this->c_s->thumbnail_height * new_ratio)) / 2;
        } else if ((*this->active_desktop)->windows[i]->thumbnail_position.width > (*this->active_desktop)->windows[i]->window_position.width && (*this->active_desktop)->windows[i]->thumbnail_position.height > (*this->active_desktop)->windows[i]->window_position.height) {  // if thumbnail is larger than its window
            (*this->active_desktop)->windows[i]->thumbnail_position.width = (*this->active_desktop)->windows[i]->window_position.width;
            (*this->active_desktop)->windows[i]->thumbnail_position.height = (*this->active_desktop)->windows[i]->window_position.height;
            (*this->active_desktop)->windows[i]->thumbnail_position.y += ((this->c_s->thumbnail_height * extra_ratio) - (*this->active_desktop)->windows[i]->window_position.height) / 2;
        }
        (*this->active_desktop)->windows[i]->thumbnail_position.y += this->c_s->title_height;

        reference_x += (*this->active_desktop)->windows[i]->thumbnail_position.width + this->c_s->margin;
        i++;
        y++;
    }

    return;
}

void VDesktopManager::update_thumbnails_pose() {
    if (!(*this->active_desktop)) return;
    for (auto i : (*this->active_desktop)->windows) {
        i->update_thumbnail_position();
    }
    return;
}

void VDesktopManager::update_windows_pose() {
    if (!(*this->active_desktop)) return;
    for (auto i : (*this->active_desktop)->windows) {
        i->update_window_position();
    }
    return;
}

void VDesktopManager::register_thumbnails() {
    if (!(*this->active_desktop)) return;
    for (auto i : (*this->active_desktop)->windows) {
        i->register_thumbnail();
    }

    return;
}

void VDesktopManager::unregister_thumbnails() {
    if (!(*this->active_desktop)) return;
    for (auto i : this->virtual_desktops) {
        for (auto i : i.second->windows) {
            i->unregister_thumbnail();
        }
    }
    return;
}

void VDesktopManager::check_thumbnail_data() {
    IObjectArray *desktops;
    UINT size = 0;
    IVirtualDesktop *desktop;
    GUID guid;
    WCHAR id[256];
    bool is_active = false;
    if (SUCCEEDED(VDesktopAPI::desktop_manager_internal->GetDesktops(0, &desktops))) {
        if (SUCCEEDED(desktops->GetCount(&size))) {
            for (auto i : this->virtual_desktops_comparing) {
                delete i.second;
            }
            this->virtual_desktops_comparing.clear();
            for (int i = 0; i < size; i++) {
                desktops->GetAt(i, IID_IVirtualDesktop, (void **)&desktop);
                desktop->GetID(&guid);
                if (SUCCEEDED(StringFromGUID2(guid, id, _countof(id)))) {
                    is_active = std::wstring(id).compare(VDesktopAPI::get_current_desktop_guid_as_string()) == 0;
                    VirtualDesktop *virtual_desktop = new VirtualDesktop(std::wstring(id), i, desktop, is_active);
                    this->virtual_desktops_comparing.insert({std::wstring(id), virtual_desktop});
                }
            }
            desktops->Release();
        }
    }
    EnumWindows(VDesktopManager::comparing_collector_callback, reinterpret_cast<LPARAM>(c_s));
    for (auto u : this->virtual_desktops) {
        auto temp = this->virtual_desktops_comparing.find(u.first);
        if (temp != this->virtual_desktops_comparing.end()) {
            if (u.second->windows.size() == temp->second->windows.size()) {
                for (int i = 0; i < u.second->windows.size() && i < temp->second->windows.size(); i++) {
                    if (u.second->windows[i]->self_hwnd != temp->second->windows[i]->self_hwnd) {
                        u.second->stale_data = true;
                        this->stale_thumbnails = true;
                        std::cout << "hwnds not equal\n";
                        // return true;
                    }
                }
            } else {
                u.second->stale_data = true;
                this->stale_thumbnails = true;
                std::cout << "windows sizes not equal\n";
            }
        }
    }
    return;
}

void VDesktopManager::check_vdesktop_data() {
    IObjectArray *desktops;
    UINT size = 0;
    IVirtualDesktop *desktop;
    GUID guid;
    WCHAR id[256];
    bool is_active = false;
    if (SUCCEEDED(VDesktopAPI::desktop_manager_internal->GetDesktops(0, &desktops))) {
        if (SUCCEEDED(desktops->GetCount(&size))) {
            this->virtual_desktops_index_comparing.clear();
            for (int i = 0; i < size; i++) {  // refresh index
                desktops->GetAt(i, IID_IVirtualDesktop, (void **)&desktop);
                desktop->GetID(&guid);
                if (SUCCEEDED(StringFromGUID2(guid, id, _countof(id)))) {
                    this->virtual_desktops_index_comparing.insert({i, std::wstring(id)});
                }
            }
            bool any_stale = this->virtual_desktops.size() - 1 != this->virtual_desktops_index_comparing.size();  // check phase 1
            if (!any_stale) {                                                                                 // check phase 2
                for (auto i : this->virtual_desktops_index_comparing) {
                    auto temp = this->virtual_desktops.find(i.second);
                    if (temp == this->virtual_desktops.end()) {
                        any_stale = true;
                        break;
                    }
                }
            }
            this->stale_vdesktops = any_stale;
            desktops->Release();
        }
    }
}

void VDesktopManager::refresh_data() {
    std::lock_guard<std::mutex> lock2(this->refresh_lock);
    this->update_current_desktop();

    this->check_vdesktop_data();
    this->check_thumbnail_data();

    if (this->stale_thumbnails || this->stale_vdesktops) {
        std::unique_lock<std::mutex> lock1(this->c_s->thumbnail_destroyer_lock);
        std::unique_lock<std::mutex> lock(this->c_s->render_lock);
        std::thread t1([=]() {  // using thread because virtual desktop api does not allow winproc thread
            if (this->stale_vdesktops) {
                std::cout << "Virtual Desktops Updating\n";
                this->refresh_v_desktops();
            }
            if (this->stale_thumbnails) {
                this->refresh_thumbnails();
            }
            this->stale_thumbnails = false;
            this->stale_vdesktops = false;
        });
        t1.join();
        lock.unlock();
        lock1.unlock();
        this->update_current_desktop();
        this->c_s->render_n_detach();
    }
}

BOOL CALLBACK VDesktopManager::collector_callback(HWND hwnd, LPARAM lParam) {
    if (!IsWindowVisible(hwnd)) return TRUE;
    CommandCenter *c_s = reinterpret_cast<CommandCenter *>(lParam);

    for (auto i : *(c_s->command_centers)) {
        if (hwnd == i->hwnd) return TRUE;
    }

    if (MonitorResolver::monitors.find(MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST))->first != c_s->monitor->handle) return TRUE;  // check if window on correct monitor

    UINT shown = 0;
    IApplicationView *app_view;
    GUID desktop_guid;
    if (SUCCEEDED(VDesktopAPI::application_view_collection->GetViewForHwnd(hwnd, &app_view))) {
        app_view->GetShowInSwitchers(&shown);
        if (shown != 0) {
            app_view->GetVirtualDesktopId(&desktop_guid);
            auto temp = c_s->desktop_manager->virtual_desktops.find(VDesktopAPI::guid_to_string(desktop_guid));
            if (temp != c_s->desktop_manager->virtual_desktops.end()) {
                if (!temp->second->stale_data) return TRUE;
                std::wstring s;
                int len = GetWindowTextLength(hwnd) + 1;
                s.resize(len);
                GetWindowText(hwnd, LPWSTR(s.c_str()), len);
                // }
                temp->second->windows.push_back(new Thumbnail(hwnd, c_s->hwnd, temp->second->windows.size(), c_s->monitor, s));
                {
                    HICON iconHandle = nullptr;
                    (HICON)SendMessageTimeout(hwnd, WM_GETICON, ICON_SMALL2, 0, SMTO_ABORTIFHUNG | SMTO_NORMAL, 10, (PDWORD_PTR)&iconHandle);
                    if (iconHandle == nullptr) (HICON)SendMessageTimeout(hwnd, WM_GETICON, ICON_BIG, 0, SMTO_ABORTIFHUNG | SMTO_NORMAL, 10, (PDWORD_PTR)&iconHandle);
                    if (iconHandle == nullptr) (HICON)SendMessageTimeout(hwnd, WM_GETICON, ICON_SMALL, 0, SMTO_ABORTIFHUNG | SMTO_NORMAL, 10, (PDWORD_PTR)&iconHandle);
                    if (iconHandle == nullptr) iconHandle = (HICON)GetClassLongPtr(hwnd, GCLP_HICON);
                    if (iconHandle == nullptr) iconHandle = (HICON)GetClassLongPtr(hwnd, GCLP_HICONSM);
                    if (iconHandle == nullptr) iconHandle = (HICON)LoadIcon(NULL, IDI_APPLICATION);
                    if (iconHandle != nullptr) {
                        ICONINFO iconinfo;
                        BITMAP bm;

                        GetIconInfo(iconHandle, &iconinfo);
                        GetObject(iconinfo.hbmColor, sizeof(BITMAP), &bm);

                        if (bm.bmWidth > 0 && bm.bmHeight > 0 && bm.bmWidth <= 1024 && bm.bmHeight <= 1024) {
                            DWORD dwSize = bm.bmWidth * bm.bmHeight * 4;
                            char *pData = new char[dwSize];
                            auto y = GetBitmapBits(iconinfo.hbmColor, dwSize, pData);
                            c_s->dc->CreateBitmap(D2D1::SizeU(bm.bmWidth, bm.bmHeight), pData, bm.bmWidth * 4, D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)), &temp->second->windows.back()->bmp);
                            delete[] pData;
                        }
                    }
                }
            }
        }
        app_view->Release();
    }
    return TRUE;
}

BOOL CALLBACK VDesktopManager::comparing_collector_callback(HWND hwnd, LPARAM lParam) {
    if (!IsWindowVisible(hwnd)) return TRUE;
    CommandCenter *c_s = reinterpret_cast<CommandCenter *>(lParam);

    for (auto i : *(c_s->command_centers)) {
        if (hwnd == i->hwnd) return TRUE;
    }

    if (MonitorResolver::monitors.find(MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST))->first != c_s->monitor->handle) return TRUE;

    UINT shown = 0;
    IApplicationView *app_view;
    GUID desktop_guid;
    if (SUCCEEDED(VDesktopAPI::application_view_collection->GetViewForHwnd(hwnd, &app_view))) {
        app_view->GetShowInSwitchers(&shown);
        if (shown != 0) {
            app_view->GetVirtualDesktopId(&desktop_guid);
            auto temp = c_s->desktop_manager->virtual_desktops_comparing.find(VDesktopAPI::guid_to_string(desktop_guid));
            if (temp != c_s->desktop_manager->virtual_desktops_comparing.end()) {
                int index = temp->second->windows.size();
                temp->second->windows.push_back(new Thumbnail(hwnd, c_s->hwnd, index, c_s->monitor));
            }
        }
        app_view->Release();
    }
    return TRUE;
}