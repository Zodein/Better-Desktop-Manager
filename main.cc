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

    if (!RegisterHotKey(NULL, 1, MOD_NOREPEAT, VK_OEM_102)) {
        auto y = GetLastError();
        MessageBox(NULL, L"Hotkey Registration Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    MonitorResolver::update_monitors_data();
    std::vector<CommandCenter *> command_centers;

    for (auto i : MonitorResolver::monitors) {
        CommandCenter *command_center = new CommandCenter(i.second, &command_centers, MAIN_THREAD_ID);
        std::thread t1([command_center]() { command_center->create_window(); });
        t1.detach();
        command_centers.push_back(command_center);
    }

    MSG message;
    while (GetMessage(&message, NULL, 0, 0) != 0) {
        switch (message.message) {
            case WM_HOTKEY: {
                std::cout << "thread hotkey pressed\n";
                SetFocus(command_centers[0]->hwnd);
                for (auto i : command_centers) {
                    PostMessage(i->hwnd, WM_HOTKEY, 0, 0);
                }
                SetForegroundWindow(command_centers[0]->hwnd);  // can't control mouse if foreground window was fullscreen without this line
                break;
            }
        }
    }
    return 0;
}