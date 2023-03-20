#pragma comment(lib, "dwmapi")
#pragma comment(lib, "user32")
#pragma comment(lib, "gdi32")
#pragma comment(lib, "ole32")
#pragma comment(lib, "dxgi")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dcomp")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "dxguid")

#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include "virtual_desktop/virtual_desktop.h"
#include "window_switcher/window_switcher.h"

int main() {
    // LPWSTR guidstr = L"{1841C6D7-4F9D-42C0-AF41-8747538F10E5}";
    // GUID guid;
    // HRESULT hr = CLSIDFromString(guidstr, (LPCLSID)&guid);

    // std::cout << std::hex << guid.Data1 << "\n";
    // std::cout << std::hex << guid.Data2 << "\n";
    // std::cout << std::hex << guid.Data3 << "\n";
    // for (auto i : guid.Data4) {
    //     std::cout << std::hex << (int)i << "\n";
    // }

    DWORD MAIN_THREAD_ID = GetCurrentThreadId();

    if (!VirtualDesktopManager::init()) {
        MessageBox(NULL, L"Virtual Desktop API Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    WindowSwitcher::wc.cbSize = sizeof(WNDCLASSEX);
    WindowSwitcher::wc.style = 0;
    WindowSwitcher::wc.lpfnWndProc = WindowSwitcher::window_proc_static;
    WindowSwitcher::wc.cbClsExtra = 0;
    WindowSwitcher::wc.cbWndExtra = 0;
    WindowSwitcher::wc.hInstance = NULL;
    WindowSwitcher::wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    WindowSwitcher::wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    WindowSwitcher::wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    WindowSwitcher::wc.lpszMenuName = NULL;
    WindowSwitcher::wc.lpszClassName = L"class";
    WindowSwitcher::wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&WindowSwitcher::wc)) {
        MessageBox(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    MonitorResolver::update_monitors_data();
    std::vector<WindowSwitcher *> window_switchers;

    for (auto i : MonitorResolver::monitors) {
        WindowSwitcher *window_switcher = new WindowSwitcher(i.second, &window_switchers, MAIN_THREAD_ID);
        std::thread t1([window_switcher]() { window_switcher->create_window(); });
        t1.detach();
        window_switchers.push_back(window_switcher);
    }

    auto x = RegisterHotKey(NULL, 1, MOD_NOREPEAT, 0x42);
    auto y = GetLastError();

    MSG message;
    while (GetMessage(&message, NULL, 0, 0) != 0) {
        switch (message.message) {
            case WM_HOTKEY: {
                std::cout << "thread hotkey pressed\n";
                for (auto i : window_switchers) {
                    std::thread t1([i]() {
                        i->thumbnail_manager->destroy_all_thumbnails();
                        i->thumbnail_manager->update_thumbnails_if_needed();
                        i->show_window();
                        InvalidateRect(i->hwnd, NULL, FALSE);
                    });
                    t1.join();
                    SendMessage(i->hwnd, WM_HOTKEY, 0, 0);
                }
                break;
            }
            case MSG_UPDATE_THUMBNAILS: {
                std::cout << "MSG_UPDATE_THUMBNAILS\n";
                for (auto i : window_switchers) {
                    std::thread t1([i]() {
                        i->thumbnail_manager->destroy_all_thumbnails();
                        i->thumbnail_manager->update_thumbnails_if_needed();
                        i->show_window();
                        InvalidateRect(i->hwnd, NULL, FALSE);
                    });
                    t1.join();
                }
                break;
            }
            case MSG_UPDATE_THUMBNAIL_POS: {
                std::cout << "MSG_UPDATE_THUMBNAIL_POS\n";
                for (auto i : window_switchers) {
                    std::thread t1([i]() {
                        i->thumbnail_manager->calculate_all_thumbnails_positions();
                        i->thumbnail_manager->update_all_thumbnails_positions();
                        InvalidateRect(i->hwnd, NULL, FALSE);
                    });
                    t1.join();
                }
                break;
            }
            case MSG_DESTROY_THUMBNAILS: {
                std::cout << "MSG_DESTROY_THUMBNAILS\n";
                for (auto i : window_switchers) {
                    std::thread t1([i]() {
                        i->thumbnail_manager->destroy_all_thumbnails();
                        InvalidateRect(i->hwnd, NULL, FALSE);
                    });
                    t1.join();
                }
                break;
            }
            case MSG_UPDATE_THUMBNAILS_IF_NEEDED: {
                std::cout << "MSG_UPDATE_THUMBNAILS_IF_NEEDED\n";
                for (auto i : window_switchers) {
                    std::thread t1([i]() {
                        if (i->thumbnail_manager->update_virtualdesktops_if_needed()) InvalidateRect(i->hwnd, NULL, FALSE);
                        if (i->thumbnail_manager->update_thumbnails_if_needed()) InvalidateRect(i->hwnd, NULL, FALSE);
                    });
                    t1.join();
                }
                break;
            }
            case MSG_UPDATE_THUMBNAILS_FORCE: {
                std::cout << "MSG_UPDATE_THUMBNAILS_FORCE\n";
                for (auto i : window_switchers) {
                    std::thread t1([i]() {
                        i->thumbnail_manager->update_virtualdesktops_if_needed(true);
                        i->thumbnail_manager->update_thumbnails_if_needed(true);
                        i->show_window();
                        InvalidateRect(i->hwnd, NULL, FALSE);
                    });
                    t1.join();
                }
                break;
            }
        }
    }
    return 0;
}