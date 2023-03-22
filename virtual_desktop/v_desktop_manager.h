#pragma once
#include <map>
#include <vector>

#include "../monitor_resolver/monitor_resolver.h"
#include "v_desktop.h"
#include "../command_center/command_center.h"
#include "../thumbnail/thumbnail.h"

class CommandCenter;

class VDesktopManager {
   public:
    CommandCenter *c_s;
    int window_width;
    int window_height;
    int thumbnail_height;

    VirtualDesktop **active_desktop = (VirtualDesktop **)new int;

    std::vector<int> widths;

    std::map<int, std::wstring> virtual_desktops_index;
    std::map<std::wstring, VirtualDesktop *> virtual_desktops;
    std::map<std::wstring, VirtualDesktop *> virtual_desktops_comparing;

    VDesktopManager(CommandCenter *command_center, int window_width, int window_height);
    bool update_current_desktop();
    bool check_if_new_thumbnails_added();
    void calculate_all_thumbnails_positions(double extra_ratio = 1.0);
    void calculate_all_virtualdesktops_positions();
    void update_all_thumbnails_positions();
    void update_all_windows_positions();
    void register_all_thumbnails();
    void destroy_all_thumbnails();
    void destroy_all_comparing_thumbnails();
    bool update_thumbnails_if_needed(bool force = false);
    bool update_virtualdesktops_if_needed(bool force = false);
    static BOOL CALLBACK collector_callback(HWND hwnd, LPARAM lParam);
    static BOOL CALLBACK comparing_collector_callback(HWND hwnd, LPARAM lParam);
};
