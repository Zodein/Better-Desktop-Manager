#pragma once
#include <d2d1.h>
#include <dwmapi.h>

#include <string>

#include "../monitor_resolver/monitor_resolver.h"

class Position {
   public:
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

class Thumbnail {
   public:
    HWND self_hwnd;
    HWND destination_hwnd;
    HRESULT hr = S_OK;
    HTHUMBNAIL thumbnail = NULL;
    DWM_THUMBNAIL_PROPERTIES thumbnail_properties;
    Position window_position;
    Position thumbnail_position;
    int order;
    double ratio;
    bool registered = false;
    Monitor* on_monitor;
    std::wstring title;
    ID2D1Bitmap* bmp = nullptr;

    Thumbnail(HWND self_hwnd, HWND destination_hwnd, int order, Monitor* on_monitor, std::wstring title = L"");
    ~Thumbnail();
    void register_thumbnail();
    void unregister_thumbnail();
    void update_window_position();
    void update_thumbnail_position();
    void repose_thumbnail(int x, int y);
};