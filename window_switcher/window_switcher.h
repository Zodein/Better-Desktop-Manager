#ifndef WINDOW_SWITCHER_GUI_H
#define WINDOW_SWITCHER_GUI_H
#include <wrl.h>
using namespace Microsoft::WRL;
#include <d2d1_2.h>
#include <d2d1_2helper.h>
#include <d3d11_2.h>
#include <dcomp.h>
#include <dwrite.h>
#include <dxgi1_3.h>
#include <windows.h>
#include <winuser.h>

#include "../thumbnail/thumbnail_manager.h"
class ThumbnailManager;

class WindowSwitcher {
   public:
    std::vector<WindowSwitcher*>* window_switchers;
    ThumbnailManager* thumbnail_manager;
    HWND hwnd;
    static WNDCLASSEX wc;
    int mouse_on = -1;
    int selected_window = -1;
    int title_height = 20;
    int margin = 20;
    int thumbnail_height = 384;
    Monitor* monitor;

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

    static D2D1_COLOR_F const background_color;
    static D2D1_COLOR_F const on_mouse_color;
    static D2D1_COLOR_F const selected_color;
    static D2D1_COLOR_F const title_bg_color;
    ComPtr<ID2D1SolidColorBrush> background_brush;
    ComPtr<ID2D1SolidColorBrush> on_mouse_brush;
    ComPtr<ID2D1SolidColorBrush> selected_brush;
    ComPtr<ID2D1SolidColorBrush> title_bg_brush;
    RECT rect = {0, 0, 0, 0};

    static LRESULT CALLBACK window_proc_static(HWND handle_window, UINT message, WPARAM wParam, LPARAM lParam);

    WindowSwitcher(Monitor* monitor, std::vector<WindowSwitcher*>* window_switchers);
    LRESULT CALLBACK window_proc(HWND handle_window, UINT message, WPARAM wParam, LPARAM lParam);
    int create_window();
    void show_window();
    void hide_window();
    void resize_window();
    void on_mouse_message(LPARAM lParam);
    void activate_window(HWND hwnd);
};
#endif