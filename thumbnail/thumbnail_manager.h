#ifndef THUMBNAIL_MANAGER_H
#define THUMBNAIL_MANAGER_H
#include <vector>

#include "../monitor_resolver/monitor_resolver.h"
#include "../window_switcher/window_switcher.h"
#include "thumbnail.h"

class WindowSwitcher;

class ThumbnailManager {
   public:
    std::vector<Thumbnail *> thumbnails;
    std::vector<Thumbnail *> thumbnails_comparing;
    WindowSwitcher *w_s;
    int window_width;
    int window_height;
    int margin;
    int thumbnail_height;
    std::vector<int> widths;

    ThumbnailManager(WindowSwitcher *window_switcher, int window_width, int window_height);
    bool check_if_new_thumbnails_added();
    void calculate_all_thumbnails_positions(double extra_ratio = 1.0);
    void update_all_windows_positions();
    void collect_all_thumbnails();
    void register_all_thumbnails();
    void destroy_all_thumbnails();
    void destroy_all_comparing_thumbnails();
    void update_thumbnails_if_needed();
    static BOOL CALLBACK collector_callback(HWND hwnd, LPARAM lParam);
    static BOOL CALLBACK comparing_collector_callback(HWND hwnd, LPARAM lParam);
};

#endif