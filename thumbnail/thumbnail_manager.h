#pragma once
#include <map>
#include <vector>

#include "../monitor_resolver/monitor_resolver.h"
#include "../virtual_desktop/virtual_desktop.h"
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
    int thumbnail_height;
    int vt_height;
    std::vector<int> widths;
    std::map<std::wstring, VirtualDesktop *> virtual_desktops;
    std::map<int, std::wstring> virtual_desktops_index;

    ThumbnailManager(WindowSwitcher *window_switcher, int window_width, int window_height);
    bool check_if_new_thumbnails_added();
    void calculate_all_thumbnails_positions(double extra_ratio = 1.0);
    void calculate_all_virtualdesktops_positions();
    void clear_virtualdesktop_windowcount();
    void update_all_thumbnails_positions();
    void update_all_windows_positions();
    void collect_all_thumbnails();
    void register_all_thumbnails();
    void destroy_all_thumbnails();
    void destroy_all_comparing_thumbnails();
    bool update_thumbnails_if_needed(bool force = false);
    bool update_virtualdesktops_if_needed(bool force = false);
    static BOOL CALLBACK collector_callback(HWND hwnd, LPARAM lParam);
    static BOOL CALLBACK comparing_collector_callback(HWND hwnd, LPARAM lParam);
};
