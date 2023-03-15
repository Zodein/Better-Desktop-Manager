#ifndef MONITOR_RESOLVER_H
#define MONITOR_RESOLVER_H
#include <windows.h>

#include <vector>

class Monitor {
   public:
    int left = 0;
    int top = 0;
    int right = 0;
    int bottom = 0;
    bool is_primary = false;

    Monitor(int left, int top, int right, int bottom, bool is_primary) {
        this->left = left;
        this->top = top;
        this->right = right;
        this->bottom = bottom;
        this->is_primary = is_primary;
    }
    int get_width() { return this->right - this->left; }
    int get_height() { return this->bottom - this->top; }
    int get_x() { return this->left; }
    int get_y() { return this->top; }
    int get_x2() { return this->right; }
    int get_y2() { return this->bottom; }
};

class MonitorResolver {
   public:
    static Monitor *selected_monitor;
    static std::vector<Monitor *> monitors;
    static BOOL CALLBACK monitor_enum(HMONITOR hMon, HDC hdc, LPRECT rect, LPARAM pData);
    static void update_monitors_data();
};

#endif