#include "window_switcher.h"

#include <windowsx.h>

#include <thread>

#include "../monitor_resolver/monitor_resolver.h"
#include "../thumbnail/thumbnail.h"
#include "gdiplus.h"
#include "iostream"
#include "wingdi.h"
#include "winuser.h"

WNDCLASSEX WindowSwitcher::wc;
WNDCLASSEX WindowSwitcher::wc_child;
HBRUSH WindowSwitcher::background_brush = CreateSolidBrush(RGB(0, 0, 0));
HBRUSH WindowSwitcher::on_mouse_brush = CreateSolidBrush(RGB(255, 255, 255));
HBRUSH WindowSwitcher::selected_brush = CreateSolidBrush(RGB(255, 255, 255));
HBRUSH WindowSwitcher::null_brush = (HBRUSH)GetStockObject(NULL_BRUSH);

WindowSwitcher::WindowSwitcher(Monitor* monitor, std::vector<WindowSwitcher*>* window_switchers) {
    this->window_switchers = window_switchers;
    this->monitor = monitor;
    this->thumbnail_manager = new ThumbnailManager(this);
}

LRESULT CALLBACK WindowSwitcher::window_proc_static(HWND handle_window, UINT message, WPARAM wParam, LPARAM lParam) {
    WindowSwitcher* window_switcher = reinterpret_cast<WindowSwitcher*>(GetWindowLongPtr(handle_window, GWLP_USERDATA));
    if (window_switcher) return window_switcher->window_proc(handle_window, message, wParam, lParam);
    return DefWindowProc(handle_window, message, wParam, lParam);
}

LRESULT CALLBACK WindowSwitcher::window_proc_child_static(HWND handle_window, UINT message, WPARAM wParam, LPARAM lParam) {
    WindowSwitcher* window_switcher = reinterpret_cast<WindowSwitcher*>(GetWindowLongPtr(handle_window, GWLP_USERDATA));
    if (window_switcher) return window_switcher->window_proc_child(handle_window, message, wParam, lParam);
    return DefWindowProc(handle_window, message, wParam, lParam);
}

LRESULT CALLBACK WindowSwitcher::window_proc(HWND handle_window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_HOTKEY: {
            if (!IsWindowVisible(this->hwnd)) {
                this->show_window();
                this->selected_window = this->thumbnail_manager->thumbnails.size() > 1;
                std::thread t1([=]() {
                    int last_checked = 0;
                    while (GetAsyncKeyState(0x42)) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        if (!IsWindowVisible(this->hwnd)) return;
                        if (last_checked < GetTickCount()) {
                            this->thumbnail_manager->update_thumbnails_if_needed();
                            last_checked = GetTickCount() + 100;
                        }
                    }
                    if (IsWindowVisible(this->hwnd)) {
                        POINT p;
                        if (GetCursorPos(&p)) {
                            if (p.x > this->monitor->get_x()) {
                                if (p.x < this->monitor->get_x2()) {
                                    if (p.y > this->monitor->get_y()) {
                                        if (p.y < this->monitor->get_y2()) {
                                            this->activate_window(this->thumbnail_manager->thumbnails[this->selected_window]->self_hwnd);
                                        }
                                    }
                                }
                            }
                        }
                        this->hide_window();
                    }
                    return;
                });
                t1.detach();
                std::cout << "HOTKEY\n";
            }
            break;
        }
        case WM_PAINT: {
            InvalidateRect(this->hwnd_child, NULL, FALSE);
            PAINTSTRUCT ps;
            HDC hdca = BeginPaint(this->hwnd, &ps);

            EndPaint(hwnd, &ps);
            std::cout << "paint\n";
            break;
        }
        case WM_ERASEBKGND: {
            return 1;
            break;
        }
        case WM_CLOSE:
            DestroyWindow(handle_window);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(handle_window, message, wParam, lParam);
    }
    return 0;
}

LRESULT CALLBACK WindowSwitcher::window_proc_child(HWND handle_window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_MOUSEWHEEL: {
            if (GET_WHEEL_DELTA_WPARAM(wParam) < 0 && this->selected_window + 1 < this->thumbnail_manager->thumbnails.size())
                this->selected_window++;
            else if (GET_WHEEL_DELTA_WPARAM(wParam) > 0 && this->selected_window > 0)
                this->selected_window--;
            else
                break;
            InvalidateRect(this->hwnd_child, NULL, FALSE);
            break;
        }
        case WM_LBUTTONDOWN: {
            if (this->mouse_on != -1) {
                this->activate_window(this->thumbnail_manager->thumbnails[this->mouse_on]->self_hwnd);
                this->hide_window();
                std::cout << "clicked on child\n";
            }
            break;
        }
        case WM_MBUTTONDOWN: {
            if (this->mouse_on != -1) {
                SendMessage(this->thumbnail_manager->thumbnails[this->mouse_on]->self_hwnd, WM_CLOSE, 0, 0);
                std::cout << "clicked on child\n";
            }
            break;
        }
        case WM_MOUSEMOVE:
            this->on_mouse_message(lParam);
            std::cout << "mousemove on child\n";
            break;
        case WM_ERASEBKGND: {
            return 0;
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdca = BeginPaint(this->hwnd_child, &ps);

            RECT rect;
            GetClipBox(hdca, &rect);
            FillRect(hdca, &rect, WindowSwitcher::background_brush);

            if (this->mouse_on >= 0) {
                int border_width = 4;
                rect.left = this->thumbnail_manager->thumbnails[this->mouse_on]->thumbnail_position.x - border_width;
                rect.top = this->thumbnail_manager->thumbnails[this->mouse_on]->thumbnail_position.y - border_width;
                rect.right = this->thumbnail_manager->thumbnails[this->mouse_on]->thumbnail_position.x + this->thumbnail_manager->thumbnails[this->mouse_on]->thumbnail_position.width + border_width;
                rect.bottom = this->thumbnail_manager->thumbnails[this->mouse_on]->thumbnail_position.y + this->thumbnail_manager->thumbnails[this->mouse_on]->thumbnail_position.height + border_width;
                FillRect(hdca, &rect, WindowSwitcher::on_mouse_brush);
            }
            if (this->selected_window >= 0) {
                {
                    int border_width = 2;
                    rect.left = this->thumbnail_manager->thumbnails[this->selected_window]->thumbnail_position.x - border_width;
                    rect.top = this->thumbnail_manager->thumbnails[this->selected_window]->thumbnail_position.y - border_width;
                    rect.right = this->thumbnail_manager->thumbnails[this->selected_window]->thumbnail_position.x + this->thumbnail_manager->thumbnails[this->selected_window]->thumbnail_position.width + border_width;
                    rect.bottom = this->thumbnail_manager->thumbnails[this->selected_window]->thumbnail_position.y + this->thumbnail_manager->thumbnails[this->selected_window]->thumbnail_position.height + border_width;
                    FillRect(hdca, &rect, WindowSwitcher::selected_brush);
                }
                {
                    HFONT font = CreateFont(16, 0, 0, 0, 300, false, false, false, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Courier New");
                    SelectObject(hdca, font);
                    SetBkColor(hdca, 0);
                    SetTextColor(hdca, 0x00ffffff);
                    SetBkMode(hdca, TRANSPARENT);
                    for (auto i : this->thumbnail_manager->thumbnails) {
                        wchar_t title[128];
                        GetWindowText(i->self_hwnd, title, 128);
                        rect.left = i->thumbnail_position.x;
                        rect.top = i->thumbnail_position.y - 20;
                        rect.right = i->thumbnail_position.x + i->thumbnail_position.width;
                        rect.bottom = i->thumbnail_position.y;
                        DrawText(hdca, title, -1, &rect, DT_BOTTOM | DT_SINGLELINE | DT_CENTER | DT_VCENTER);
                    }
                    DeleteObject(font);
                }
            }

            EndPaint(this->hwnd_child, &ps);

            std::cout << "paint_child\n";
            break;
        }
        case WM_CLOSE:
            DestroyWindow(handle_window);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(handle_window, message, wParam, lParam);
    }
    return 0;
}

int WindowSwitcher::create_window() {
    this->hwnd = CreateWindowEx(WS_EX_TOPMOST, WindowSwitcher::wc.lpszClassName, L"Better Desktop Manager", WS_POPUP, this->monitor->get_x(), this->monitor->get_y(), this->monitor->get_width(), this->monitor->get_height(), NULL, NULL, NULL, NULL);
    this->hwnd_child = CreateWindowEx(0, WindowSwitcher::wc_child.lpszClassName, L"", WS_CHILD | WS_VISIBLE, 0, 0, this->monitor->get_width(), this->monitor->get_height(), this->hwnd, NULL, NULL, NULL);

    if (this->hwnd == NULL || this->hwnd_child == NULL) {
        DWORD x = GetLastError();
        MessageBox(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
    SetWindowLongPtr(hwnd_child, GWLP_USERDATA, (LONG_PTR)this);

    SetWindowLong(this->hwnd_child, GWL_EXSTYLE, GetWindowLong(this->hwnd_child, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(this->hwnd_child, RGB(48, 48, 48), 192, LWA_ALPHA);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

void WindowSwitcher::activate_window(HWND hwnd_window) {
    if (IsIconic(hwnd_window)) ShowWindow(hwnd_window, SW_RESTORE);
    SetForegroundWindow(hwnd_window);
}

void WindowSwitcher::show_window() {
    this->thumbnail_manager->update_thumbnails_if_needed();
    SetWindowPos(this->hwnd, HWND_TOPMOST, this->monitor->get_x(), this->monitor->get_y(), this->monitor->get_width(), this->monitor->get_height(), SWP_SHOWWINDOW);
    return;
}

void WindowSwitcher::hide_window() {
    ShowWindow(this->hwnd, 0);
    this->thumbnail_manager->destroy_all_thumbnails();
    return;
}

void WindowSwitcher::resize_window() {
    SetWindowPos(this->hwnd, HWND_TOPMOST, this->monitor->get_x(), this->monitor->get_y(), this->monitor->get_width(), this->monitor->get_height(), SWP_SHOWWINDOW);
    return;
}

void WindowSwitcher::on_mouse_message(LPARAM lParam) {
    int x = GET_X_LPARAM(lParam);
    int y = GET_Y_LPARAM(lParam);
    for (auto i : this->thumbnail_manager->thumbnails) {
        if (x > i->thumbnail_position.x) {
            if (y > i->thumbnail_position.y) {
                if (x < i->thumbnail_position.x + i->thumbnail_position.width) {
                    if (y < i->thumbnail_position.y + i->thumbnail_position.height) {
                        if (this->mouse_on != i->order) {
                            this->mouse_on = i->order;
                            InvalidateRect(this->hwnd_child, NULL, FALSE);
                        }
                        return;
                    }
                }
            }
        }
    }
    if (this->mouse_on != -1) {
        this->mouse_on = -1;
        InvalidateRect(this->hwnd_child, NULL, FALSE);
    }
    return;
}
