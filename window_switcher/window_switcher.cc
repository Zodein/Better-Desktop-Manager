#include "window_switcher.h"

#include <windowsx.h>

#include <thread>

#include "../thumbnail/thumbnail.h"
#include "gdiplus.h"
#include "iostream"
#include "wingdi.h"
#include "winuser.h"

ThumbnailManager *WindowSwitcher::thumbnail_manager = new ThumbnailManager(32, 384);
HWND WindowSwitcher::hwnd;
HWND WindowSwitcher::hwnd_child;
WNDCLASSEX WindowSwitcher::wc;
WNDCLASSEX WindowSwitcher::wc_child;
int WindowSwitcher::mouse_on = -1;

HBRUSH WindowSwitcher::background_brush = CreateSolidBrush(RGB(0, 0, 0));
HBRUSH WindowSwitcher::border_brush = CreateSolidBrush(RGB(96, 96, 96));
HBRUSH WindowSwitcher::null_brush = (HBRUSH)GetStockObject(NULL_BRUSH);
RECT WindowSwitcher::rect = {0, 0, 0, 0};

void WindowSwitcher::on_mouse_message(LPARAM lParam) {
    int x = GET_X_LPARAM(lParam);
    int y = GET_Y_LPARAM(lParam);
    for (auto i : WindowSwitcher::thumbnail_manager->thumbnails) {
        if (x > i->thumbnail_position.x) {
            if (y > i->thumbnail_position.y) {
                if (x < i->thumbnail_position.x + i->thumbnail_position.width) {
                    if (y < i->thumbnail_position.y + i->thumbnail_position.height) {
                        if (WindowSwitcher::mouse_on != i->order) {
                            WindowSwitcher::mouse_on = i->order;
                            InvalidateRect(WindowSwitcher::hwnd_child, NULL, FALSE);
                        }
                        return;
                    }
                }
            }
        }
    }
    if (WindowSwitcher::mouse_on != -1) {
        WindowSwitcher::mouse_on = -1;
        InvalidateRect(WindowSwitcher::hwnd_child, NULL, FALSE);
    }
    // std::cout << "mousemove\n";
    return;
}

LRESULT CALLBACK WindowSwitcher::window_proc(HWND handle_window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_HOTKEY: {
            if (!IsWindowVisible(WindowSwitcher::hwnd)) {
                WindowSwitcher::show_window();
                std::thread t1([]() {
                    int last_checked = 0;
                    while (GetAsyncKeyState(0x42)) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        if (!IsWindowVisible(WindowSwitcher::hwnd)) return;
                        if (last_checked < GetTickCount()) {
                            WindowSwitcher::thumbnail_manager->update_thumbnails_if_needed();
                            last_checked = GetTickCount() + 100;
                        }
                    }
                    WindowSwitcher::hide_window();
                    return;
                });
                t1.detach();

                std::cout << "HOTKEY\n";
            }
            break;
        }
        // case WM_LBUTTONDOWN: {
        //     if (WindowSwitcher::mouse_on != -1) {
        //         SetForegroundWindow(WindowSwitcher::thumbnail_manager->thumbnails[WindowSwitcher::mouse_on]->self_hwnd);
        //         WindowSwitcher::hide_window();
        //     }
        //     std::cout << "clicked on parent\n";
        //     break;
        // }
        // case WM_MBUTTONDOWN: {
        //     if (WindowSwitcher::mouse_on != -1) {
        //         SendMessage(WindowSwitcher::thumbnail_manager->thumbnails[WindowSwitcher::mouse_on]->self_hwnd, WM_CLOSE, 0, 0);
        //     }
        //     std::cout << "clicked on parent\n";
        //     break;
        // }
        // case WM_MOUSEMOVE:
        //     WindowSwitcher::on_mouse_message(lParam);
        //     std::cout << "mousemove on parent\n";
        //     break;
        case WM_PAINT: {
            InvalidateRect(WindowSwitcher::hwnd_child, NULL, FALSE);
            PAINTSTRUCT ps;
            HDC hdca = BeginPaint(WindowSwitcher::hwnd, &ps);

            EndPaint(hwnd, &ps);
            std::cout << "paint\n";
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
        case WM_LBUTTONDOWN: {
            if (WindowSwitcher::mouse_on != -1) {
                SetForegroundWindow(WindowSwitcher::thumbnail_manager->thumbnails[WindowSwitcher::mouse_on]->self_hwnd);
                WindowSwitcher::hide_window();
                std::cout << "clicked on child\n";
            }
        }
        case WM_MBUTTONDOWN: {
            if (WindowSwitcher::mouse_on != -1) {
                SendMessage(WindowSwitcher::thumbnail_manager->thumbnails[WindowSwitcher::mouse_on]->self_hwnd, WM_CLOSE, 0, 0);
                std::cout << "clicked on child\n";
            }
        }
        case WM_MOUSEMOVE:
            WindowSwitcher::on_mouse_message(lParam);
            std::cout << "mousemove on child\n";
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdca = BeginPaint(WindowSwitcher::hwnd_child, &ps);

            RECT rect;
            GetClipBox(hdca, &rect);
            FillRect(hdca, &rect, WindowSwitcher::background_brush);

            if (mouse_on >= 0) {
                int border_width = 8;
                rect.left = WindowSwitcher::thumbnail_manager->thumbnails[WindowSwitcher::mouse_on]->thumbnail_position.x - border_width;
                rect.top = WindowSwitcher::thumbnail_manager->thumbnails[WindowSwitcher::mouse_on]->thumbnail_position.y - border_width;
                rect.right = WindowSwitcher::thumbnail_manager->thumbnails[WindowSwitcher::mouse_on]->thumbnail_position.x + WindowSwitcher::thumbnail_manager->thumbnails[WindowSwitcher::mouse_on]->thumbnail_position.width + border_width;
                rect.bottom = WindowSwitcher::thumbnail_manager->thumbnails[WindowSwitcher::mouse_on]->thumbnail_position.y + WindowSwitcher::thumbnail_manager->thumbnails[WindowSwitcher::mouse_on]->thumbnail_position.height + border_width;
                FillRect(hdca, &rect, WindowSwitcher::border_brush);
            }

            EndPaint(WindowSwitcher::hwnd_child, &ps);

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
    WindowSwitcher::wc.cbSize = sizeof(WNDCLASSEX);
    WindowSwitcher::wc.style = 0;
    WindowSwitcher::wc.lpfnWndProc = WindowSwitcher::window_proc;
    WindowSwitcher::wc.cbClsExtra = 0;
    WindowSwitcher::wc.cbWndExtra = 0;
    WindowSwitcher::wc.hInstance = GetModuleHandle(0);
    WindowSwitcher::wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WindowSwitcher::wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    WindowSwitcher::wc.hbrBackground = WindowSwitcher::null_brush;
    WindowSwitcher::wc.lpszMenuName = NULL;
    WindowSwitcher::wc.lpszClassName = L"class";
    WindowSwitcher::wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    WindowSwitcher::wc_child = WindowSwitcher::wc;
    WindowSwitcher::wc_child.lpfnWndProc = WindowSwitcher::window_proc_child;
    WindowSwitcher::wc_child.hbrBackground = WindowSwitcher::background_brush;
    WindowSwitcher::wc_child.lpszClassName = L"class_child";

    if (!RegisterClassEx(&WindowSwitcher::wc) || !RegisterClassEx(&WindowSwitcher::wc_child)) {
        MessageBox(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    WindowSwitcher::hwnd = CreateWindowEx(0, WindowSwitcher::wc.lpszClassName, L"Better Desktop Manager", WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, 3440, 1440, NULL, NULL, WindowSwitcher::wc.hInstance, NULL);
    WindowSwitcher::hwnd_child = CreateWindowEx(0, WindowSwitcher::wc_child.lpszClassName, L"Better Desktop Manager", WS_CHILD | WS_VISIBLE, 0, 0, 3440, 1440, WindowSwitcher::hwnd, NULL, NULL, NULL);

    if (WindowSwitcher::hwnd == NULL || WindowSwitcher::hwnd_child == NULL) {
        DWORD x = GetLastError();
        MessageBox(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    SetWindowLong(WindowSwitcher::hwnd_child, GWL_EXSTYLE, GetWindowLong(WindowSwitcher::hwnd_child, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(WindowSwitcher::hwnd_child, RGB(48, 48, 48), 192, LWA_ALPHA);

    RegisterHotKey(WindowSwitcher::hwnd, 1, MOD_NOREPEAT, 0x42);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

void WindowSwitcher::show_window() {
    WindowSwitcher::thumbnail_manager->update_thumbnails_if_needed();
    // int x = (3440 - WindowSwitcher::thumbnail_manager->window_width) / 2;
    // int y = (1440 - WindowSwitcher::thumbnail_manager->window_height) / 2;
    // int width = WindowSwitcher::thumbnail_manager->window_width;
    // int height = WindowSwitcher::thumbnail_manager->window_height;
    // SetWindowPos(WindowSwitcher::hwnd, HWND_TOPMOST, x, y, width, height, SWP_SHOWWINDOW);
    SetWindowPos(WindowSwitcher::hwnd, HWND_TOPMOST, 0, 0, 3440, 1440, SWP_SHOWWINDOW);
    return;
}

void WindowSwitcher::hide_window() {
    ShowWindow(WindowSwitcher::hwnd, 0);
    WindowSwitcher::thumbnail_manager->destroy_all_thumbnails();
    return;
}

void WindowSwitcher::resize_window() {
    // int x = (3440 - WindowSwitcher::thumbnail_manager->window_width) / 2;
    // int y = (1440 - WindowSwitcher::thumbnail_manager->window_height) / 2;
    // int width = WindowSwitcher::thumbnail_manager->window_width;
    // int height = WindowSwitcher::thumbnail_manager->window_height;
    // SetWindowPos(WindowSwitcher::hwnd, HWND_TOPMOST, x, y, width, height, SWP_SHOWWINDOW);
    SetWindowPos(WindowSwitcher::hwnd, HWND_TOPMOST, 0, 0, 3440, 1440, SWP_SHOWWINDOW);
    return;
}
