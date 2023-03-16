#ifndef THUMBNAIL_H
#define THUMBNAIL_H
#include <dwmapi.h>

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
    HMONITOR on_monitor = (HMONITOR)-1;

    Thumbnail(HWND self_hwnd, HWND destination_hwnd, int order, HMONITOR on_monitor);
    ~Thumbnail();
    void register_thumbnail();
    void unregister_thumbnail();
    void update_window_position();
};
#endif