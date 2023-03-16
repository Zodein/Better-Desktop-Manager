#ifndef WINDOW_SWITCHER_GUI_H
#define WINDOW_SWITCHER_GUI_H
#include <dwmapi.h>
#include <windows.h>
#include <winuser.h>

#include "../thumbnail/thumbnail_manager.h"
class ThumbnailManager;

class WindowSwitcher {
   public:
    // ThumbnailManager* thumbnail_manager;
    // HWND hwnd;
    // HWND hwnd_child;
    // WNDCLASSEX wc;
    // WNDCLASSEX wc_child;
    // HBRUSH background_brush;
    // HBRUSH on_mouse_brush;
    // HBRUSH selected_brush;
    // HBRUSH null_brush;
    // RECT rect;
    // int mouse_on;
    // int selected_window;
    // int title_height;
    std::vector<WindowSwitcher*>* window_switchers;
    ThumbnailManager* thumbnail_manager;
    HWND hwnd;
    HWND hwnd_child;
    static WNDCLASSEX wc;
    static WNDCLASSEX wc_child;
    int mouse_on = -1;
    int selected_window = -1;
    int title_height = 20;
    int margin = 20;
    int thumbnail_height = 384;
    Monitor* monitor;

    static HBRUSH background_brush;
    static HBRUSH on_mouse_brush;
    static HBRUSH selected_brush;
    static HBRUSH null_brush;
    RECT rect = {0, 0, 0, 0};

    static LRESULT CALLBACK window_proc_static(HWND handle_window, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK window_proc_child_static(HWND handle_window, UINT message, WPARAM wParam, LPARAM lParam);

    WindowSwitcher(Monitor* monitor, std::vector<WindowSwitcher*>* window_switchers);
    LRESULT CALLBACK window_proc(HWND handle_window, UINT message, WPARAM wParam, LPARAM lParam);
    LRESULT CALLBACK window_proc_child(HWND handle_window, UINT message, WPARAM wParam, LPARAM lParam);
    int create_window();
    void show_window();
    void hide_window();
    void resize_window();
    void on_mouse_message(LPARAM lParam);
    void activate_window(HWND hwnd);
};
#endif