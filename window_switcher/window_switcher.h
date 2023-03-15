#ifndef WINDOW_SWITCHER_GUI_H
#define WINDOW_SWITCHER_GUI_H
#include <dwmapi.h>
#include <windows.h>
#include <winuser.h>

#include "../thumbnail/thumbnail_manager.h"

class WindowSwitcher {
   public:
    static ThumbnailManager* thumbnail_manager;
    static HWND hwnd;
    static HWND hwnd_child;
    static WNDCLASSEX wc;
    static WNDCLASSEX wc_child;
    static HBRUSH background_brush;
    static HBRUSH border_brush;
    static HBRUSH null_brush;
    static RECT rect;
    static int mouse_on;

    static LRESULT CALLBACK window_proc(HWND handle_window, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK window_proc_child(HWND handle_window, UINT message, WPARAM wParam, LPARAM lParam);
    static int create_window();
    static void show_window();
    static void hide_window();
    static void resize_window();
    static void on_mouse_message(LPARAM lParam);
};
#endif