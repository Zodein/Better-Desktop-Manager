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

#include "command_center/command_center.h"
#include "virtual_desktop/v_desktop.h"

std::vector<CommandCenter *> command_centers;
bool showing = false;

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    KBDLLHOOKSTRUCT *kbdStruct = (KBDLLHOOKSTRUCT *)lParam;
    if (kbdStruct->vkCode == VK_F24) {
        if (wParam == WM_KEYDOWN && !showing) {
            showing = true;
            std::thread t1([=]() {
                std::cout << "showing = true\n";
                if (command_centers.size() > 0) {
                    SetFocus(command_centers[0]->hwnd);
                    for (auto i : command_centers) {
                        PostMessage(i->hwnd, WM_HOTKEY, 0, 0);
                    }
                    SetForegroundWindow(command_centers[0]->hwnd);  // can't control mouse if foreground window was fullscreen without this line
                }
            });
            t1.detach();
        } else if (wParam == WM_KEYUP && showing) {
            showing = false;
            std::cout << "showing = false\n";
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main() {
    DWORD MAIN_THREAD_ID = GetCurrentThreadId();

    if (!VDesktopAPI::init()) {
        MessageBox(NULL, L"Virtual Desktop API Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    CommandCenter::wc.cbSize = sizeof(WNDCLASSEX);
    CommandCenter::wc.style = 0;
    CommandCenter::wc.lpfnWndProc = CommandCenter::window_proc_static;
    CommandCenter::wc.cbClsExtra = 0;
    CommandCenter::wc.cbWndExtra = 0;
    CommandCenter::wc.hInstance = NULL;
    CommandCenter::wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    CommandCenter::wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    CommandCenter::wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    CommandCenter::wc.lpszMenuName = NULL;
    CommandCenter::wc.lpszClassName = L"class";
    CommandCenter::wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    if (!RegisterClassEx(&CommandCenter::wc)) {
        MessageBox(NULL, L"Window Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    if (!RegisterHotKey(NULL, 1, MOD_NOREPEAT, VK_F24)) {
        MessageBox(NULL, L"Hotkey Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    HHOOK ll_keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, 0, 0);

    if (ll_keyboard_hook == NULL) {
        MessageBox(NULL, L"Keyboard Hook Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    MonitorResolver::update_monitors_data();

    for (auto i : MonitorResolver::monitors) {
        CommandCenter *command_center = new CommandCenter(i.second, &command_centers, MAIN_THREAD_ID);
        std::thread t1([command_center]() { command_center->create_window(); });
        t1.detach();
        command_centers.push_back(command_center);
    }

    MSG message;
    while (GetMessage(&message, NULL, 0, 0) != 0) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }
    return 0;
}