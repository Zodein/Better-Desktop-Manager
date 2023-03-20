#ifndef MONITOR_RESOLVER_H
#define MONITOR_RESOLVER_H
#include <windows.h>

#include <map>
#include <vector>

struct VT_Size {
    int reference_x = 0;
    int reference_y = 0;
    int h_margin = 0;
    int v_margin = 0;
    int width = 0;
    int height = 0;
    double ratio = 0;
};

class Monitor {
   public:
    int left = 0;
    int top = 0;
    int right = 0;
    int bottom = 0;
    double ratio = 0;
    bool is_primary = false;
    HMONITOR handle = 0;
    VT_Size *vt_size = new VT_Size;

    Monitor(int left, int top, int right, int bottom, bool is_primary, HMONITOR handle) {
        this->left = left;
        this->top = top;
        this->right = right;
        this->bottom = bottom;
        this->is_primary = is_primary;
        this->handle = handle;
        this->ratio = (double)this->get_width() / (double)this->get_height();

        this->vt_size->h_margin = 24;
        this->vt_size->v_margin = 24;
        this->vt_size->reference_x = this->vt_size->h_margin;
        this->vt_size->reference_y = (this->get_height() * 0.75) + this->vt_size->v_margin;
        this->vt_size->height = (this->get_height() * 0.25) - (this->vt_size->v_margin * 2);
        this->vt_size->width = (this->vt_size->height * this->ratio);
        this->vt_size->ratio = this->ratio;
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
    static std::map<HMONITOR, Monitor *> monitors;
    static BOOL CALLBACK monitor_enum(HMONITOR hMon, HDC hdc, LPRECT rect, LPARAM pData);
    static void update_monitors_data();
};

#endif