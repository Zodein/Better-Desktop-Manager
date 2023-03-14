#include "window_switcher_gui.h"

#include <windowsx.h>

#include "../thumbnail/thumbnail.h"
#include "gdiplus.h"
#include "iostream"
#include "window_switcher.h"
#include "wingdi.h"
#include "winuser.h"

HWND WindowSwitcherGUI::hwnd;
WNDCLASSEX WindowSwitcherGUI::wc;
MSG WindowSwitcherGUI::msg;
int WindowSwitcherGUI::max_fps = 144;
int WindowSwitcherGUI::time_between_frames = 1000 / WindowSwitcherGUI::max_fps;
int WindowSwitcherGUI::last_draw = 0;
int WindowSwitcherGUI::mouse_on = -1;

HBRUSH WindowSwitcherGUI::background_brush = CreateSolidBrush(RGB(48, 48, 48));
HBRUSH WindowSwitcherGUI::border_brush = CreateSolidBrush(RGB(96, 96, 96));
HDC WindowSwitcherGUI::hdc = NULL;

void WindowSwitcherGUI::on_mouse_message(LPARAM lParam) {
    int x = GET_X_LPARAM(lParam);
    int y = GET_Y_LPARAM(lParam);
    for (auto i : WindowSwitcher::thumbnail_manager->thumbnails) {
        if (x > i->thumbnail_position.x) {
            if (y > i->thumbnail_position.y) {
                if (x < i->thumbnail_position.x + i->thumbnail_position.width) {
                    if (y < i->thumbnail_position.y + i->thumbnail_position.height) {
                        if (last_draw + time_between_frames < GetTickCount()) {
                            if (WindowSwitcherGUI::hdc == NULL) WindowSwitcherGUI::update_hdc();
                            if (WindowSwitcherGUI::mouse_on != i->order) {
                                WindowSwitcherGUI::mouse_on = i->order;
                                RECT rect;
                                GetClipBox(WindowSwitcherGUI::hdc, &rect);
                                InvalidateRect(WindowSwitcherGUI::hwnd, &rect, FALSE);
                            }
                            return;
                        }
                    }
                }
            }
        }
    }
    if (WindowSwitcherGUI::mouse_on != -1) {
        WindowSwitcherGUI::mouse_on = -1;
        RECT rect;
        GetClipBox(WindowSwitcherGUI::hdc, &rect);
        InvalidateRect(WindowSwitcherGUI::hwnd, &rect, FALSE);
    }
    std::cout << x << " - " << y << "\n";
    // WindowSwitcherGUI::clear_background();
    return;
}

void WindowSwitcherGUI::clear_background() {
    if (WindowSwitcherGUI::hdc == NULL) WindowSwitcherGUI::update_hdc();
    RECT rect;
    GetClipBox(WindowSwitcherGUI::hdc, &rect);
    FillRect(WindowSwitcherGUI::hdc, &rect, WindowSwitcherGUI::background_brush);
}

void WindowSwitcherGUI::update_hdc() { WindowSwitcherGUI::hdc = GetDC(WindowSwitcherGUI::hwnd); }

LRESULT CALLBACK WindowSwitcherGUI::window_proc(HWND handle_window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_MOUSEMOVE:
            WindowSwitcherGUI::on_mouse_message(lParam);
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdca = BeginPaint(WindowSwitcherGUI::hwnd, &ps);

            RECT rect;
            GetClipBox(hdca, &rect);
            FillRect(hdca, &rect, WindowSwitcherGUI::background_brush);

            if (mouse_on >= 0) {
                int border_width = 8;
                rect.left = WindowSwitcher::thumbnail_manager->thumbnails[WindowSwitcherGUI::mouse_on]->thumbnail_position.x - border_width;
                rect.top = WindowSwitcher::thumbnail_manager->thumbnails[WindowSwitcherGUI::mouse_on]->thumbnail_position.y - border_width;
                rect.right = WindowSwitcher::thumbnail_manager->thumbnails[WindowSwitcherGUI::mouse_on]->thumbnail_position.x + WindowSwitcher::thumbnail_manager->thumbnails[WindowSwitcherGUI::mouse_on]->thumbnail_position.width + border_width;
                rect.bottom = WindowSwitcher::thumbnail_manager->thumbnails[WindowSwitcherGUI::mouse_on]->thumbnail_position.y + WindowSwitcher::thumbnail_manager->thumbnails[WindowSwitcherGUI::mouse_on]->thumbnail_position.height + border_width;
                FillRect(hdca, &rect, WindowSwitcherGUI::border_brush);
            }

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
            // if (message != 512 && message != 132 && message != 32)
            // std::cout << message << "\n";
            return DefWindowProc(handle_window, message, wParam, lParam);
    }
    return 0;
}

int WindowSwitcherGUI::create_window() {
    WindowSwitcherGUI::wc.cbSize = sizeof(WNDCLASSEX);
    WindowSwitcherGUI::wc.style = 0;
    WindowSwitcherGUI::wc.lpfnWndProc = WindowSwitcherGUI::window_proc;
    WindowSwitcherGUI::wc.cbClsExtra = 0;
    WindowSwitcherGUI::wc.cbWndExtra = 0;
    WindowSwitcherGUI::wc.hInstance = GetModuleHandle(0);
    WindowSwitcherGUI::wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WindowSwitcherGUI::wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    WindowSwitcherGUI::wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    WindowSwitcherGUI::wc.lpszMenuName = NULL;
    WindowSwitcherGUI::wc.lpszClassName = L"class";
    WindowSwitcherGUI::wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&WindowSwitcherGUI::wc)) {
        MessageBox(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    WindowSwitcherGUI::hwnd = CreateWindowEx(0, WindowSwitcherGUI::wc.lpszClassName, L"Better Desktop Manager", WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, 3440, 1080, NULL, NULL, WindowSwitcherGUI::wc.hInstance, NULL);

    if (WindowSwitcherGUI::hwnd == NULL) {
        DWORD x = GetLastError();
        MessageBox(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }
    // ShowWindow(WindowSwitcherGUI::hwnd, SW_SHOWNORMAL);
    // UpdateWindow(WindowSwitcherGUI::hwnd);

    while (GetMessage(&WindowSwitcherGUI::msg, NULL, 0, 0) > 0) {
        TranslateMessage(&WindowSwitcherGUI::msg);
        DispatchMessage(&WindowSwitcherGUI::msg);
    }
    return WindowSwitcherGUI::msg.wParam;
}

void WindowSwitcherGUI::resize(int x, int y, int width, int height) {
    SetWindowPos(WindowSwitcherGUI::hwnd, 0, x, y, width, height, SWP_NOACTIVATE);
    return;
}

void WindowSwitcherGUI::show_window() {
    int x = (3440 - WindowSwitcher::thumbnail_manager->window_width) / 2;
    int y = (1440 - WindowSwitcher::thumbnail_manager->window_height) / 2;
    int width = WindowSwitcher::thumbnail_manager->window_width;
    int height = WindowSwitcher::thumbnail_manager->window_height;
    SetWindowPos(WindowSwitcherGUI::hwnd, 0, x, y, width, height, SWP_SHOWWINDOW);
    // ShowWindow(WindowSwitcherGUI::hwnd, SW_SHOWNORMAL);
    return;
}