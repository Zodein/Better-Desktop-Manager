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
    bool stale_thumbnails = false;
    bool stale_vdesktops = false;

    VirtualDesktop **active_desktop = (VirtualDesktop **)new int;

    std::vector<int> widths;

    std::map<int, std::wstring> virtual_desktops_index;
    std::map<int, std::wstring> virtual_desktops_index_comparing;
    std::map<std::wstring, VirtualDesktop *> virtual_desktops;
    std::map<std::wstring, VirtualDesktop *> virtual_desktops_comparing;

    std::mutex refresh_lock;

    VDesktopManager(CommandCenter *command_center, int window_width, int window_height);
    void update_current_desktop();
    void refresh_thumbnails();
    void refresh_v_desktops();
    void calculate_vdesktops_pose();
    void calculate_thumbnails_pose(double extra_ratio = 1.0);
    void update_thumbnails_pose();
    void update_windows_pose();
    void register_thumbnails();
    void unregister_thumbnails();
    void check_thumbnail_data();
    void check_vdesktop_data();
    void refresh_data();
    VirtualDesktop *get_vdesktop_by_index(int index);
    VirtualDesktop *get_vdesktop_by_guid(std::wstring guid);
    VirtualDesktop *get_vdesktop_by_guid(GUID guid);
    static BOOL CALLBACK collector_callback(HWND hwnd, LPARAM lParam);
    static BOOL CALLBACK comparing_collector_callback(HWND hwnd, LPARAM lParam);
};
