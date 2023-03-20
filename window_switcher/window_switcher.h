#pragma once
#include <wrl.h>
using namespace Microsoft::WRL;
#include <d2d1.h>
#include <d2d1_2.h>
#include <d2d1_2helper.h>
#include <d2d1effects.h>
#include <d2d1helper.h>
#include <d3d11_2.h>
#include <dcomp.h>
#include <dwrite.h>
#include <dxgi1_3.h>
#include <windows.h>
#include <winuser.h>

#include <mutex>

#include "../thumbnail/thumbnail_manager.h"

class ThumbnailManager;

class WindowSwitcher {
   public:
    static D2D1_COLOR_F const background_color;
    static D2D1_COLOR_F const on_mouse_color;
    static D2D1_COLOR_F const selected_color;
    static D2D1_COLOR_F const title_bg_color;
    static D2D1_COLOR_F const active_vt_bg_color;

    std::vector<WindowSwitcher*>* window_switchers;
    ThumbnailManager* thumbnail_manager;
    HWND hwnd;
    static WNDCLASSEX wc;
    int mouse_on = -1;
    int mouse_down = false;
    int mouse_down_on[2] = {0, 0};
    int catched_thumbnail_ref_coord[2] = {0, 0};
    int selected_window = -1;
    int virtual_desktop_scroll = 0;
    Monitor* monitor;
    DWORD MAIN_THREAD_ID;
    std::mutex render_lock;
    std::mutex thumbnail_destroyer_lock;

    int title_height = 24;
    int margin = 24;
    int thumbnail_height = 384;

    ComPtr<ID3D11Device> direct3dDevice;
    ComPtr<ID2D1DeviceContext> dc;
    ComPtr<IDCompositionDevice> dcompDevice;
    ComPtr<IDXGISwapChain1> swapChain;
    ComPtr<IDCompositionVisual> visual;
    ComPtr<IDCompositionTarget> target;
    ComPtr<IDXGISurface2> surface;
    ComPtr<ID2D1Device1> d2Device;
    ComPtr<ID2D1Factory2> d2Factory;
    ComPtr<IDXGIDevice> dxgiDevice;
    ComPtr<IDXGIFactory2> dxFactory;
    IDWriteFactory* writeFactory;
    IDWriteTextFormat* writeTextFormat;
    IDWriteTextFormat* virtual_desktop_label_format;

    ComPtr<ID2D1SolidColorBrush> background_brush;
    ComPtr<ID2D1SolidColorBrush> on_mouse_brush;
    ComPtr<ID2D1SolidColorBrush> selected_brush;
    ComPtr<ID2D1SolidColorBrush> title_bg_brush;
    ComPtr<ID2D1SolidColorBrush> active_vt_bg_brush;
    RECT rect = {0, 0, 0, 0};

    void CreateRoundRect(int x, int y, int width, int height, int leftTop, int rightTop, int rightBottom, int leftBottom, ID2D1Brush* brush);
    LRESULT CALLBACK window_proc(HWND handle_window, UINT message, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK window_proc_static(HWND handle_window, UINT message, WPARAM wParam, LPARAM lParam);
    WindowSwitcher(Monitor* monitor, std::vector<WindowSwitcher*>* window_switchers, DWORD MAIN_THREAD_ID);

    int create_window();

    void activate_this_window(HWND hwnd);
    void reset_selected();
    void show_window();
    void hide_window();
    void render();
    void render_vdesktops();

    void on_hotkey_event();
    void on_print_event();
    void on_mousewheel_event(WPARAM w_param, LPARAM l_param);
    void on_mousemdown_event(WPARAM w_param, LPARAM l_param);
    void on_mouseldown_event(WPARAM w_param, LPARAM l_param);
    void on_mouselup_event(WPARAM w_param, LPARAM l_param);
    void on_mousemove_event(LPARAM l_param);
};

struct WINDOWCOMPOSITIONATTRIBDATA {
    int Attrib = 19;
    void* pvData;
    DWORD cbData;
};

struct AccentPolicy {
    int AccentState = 3;
    int AccentFlags = 0;
    int GradientColor = 0;
    int AnimationId = 0;
};

typedef BOOL(WINAPI* SetWindowCompositionAttribute)(IN HWND hwnd, IN WINDOWCOMPOSITIONATTRIBDATA* pwcad);

struct ComException {
    HRESULT result;
    ComException(HRESULT const value) : result(value) {}
};
