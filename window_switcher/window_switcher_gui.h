#ifndef WINDOW_SWITCHER_GUI_H
#define WINDOW_SWITCHER_GUI_H
#include <dwmapi.h>
#include <windows.h>
#include <winuser.h>

class WindowSwitcherGUI {
   public:
    static HWND hwnd;
    static WNDCLASSEX wc;
    static MSG msg;
    static int max_fps;
    static int time_between_frames;
    static int last_draw;
    static HBRUSH background_brush;
    static HBRUSH border_brush;
    static HDC hdc;
    static int mouse_on;

    static LRESULT CALLBACK window_proc(HWND handle_window, UINT message, WPARAM wParam, LPARAM lParam);
    static int create_window();
    static void show_window();
    static void resize(int x, int y, int width, int height);
    static void on_mouse_message(LPARAM lParam);
    static void clear_background();
    static void update_hdc();
};
#endif