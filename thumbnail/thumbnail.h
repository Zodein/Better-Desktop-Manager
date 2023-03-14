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

    Thumbnail(HWND self_hwnd, HWND destination_hwnd);
    Thumbnail(HWND self_hwnd, HWND destination_hwnd, int order);
    Thumbnail(HWND self_hwnd, HWND destination_hwnd, int order, int x, int y, int width, int height);
    void register_thumbnail();
    int update_n_get_x();
    int update_n_get_y();
    int update_n_get_width();
    int update_n_get_height();
    double update_n_get_ratio();
    void update_window_position();
};
#endif