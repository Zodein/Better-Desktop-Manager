#include "monitor_resolver.h"

std::vector<Monitor *> MonitorResolver::monitors;
Monitor *MonitorResolver::selected_monitor;

BOOL CALLBACK MonitorResolver::monitor_enum(HMONITOR hMon, HDC hdc, LPRECT rect, LPARAM pData) {
    bool is_primary = (rect->left == 0 && rect->top == 0);
    Monitor *monitor = new Monitor(rect->left, rect->top, rect->right, rect->bottom, is_primary);
    if (is_primary) MonitorResolver::selected_monitor = monitor;
    MonitorResolver::monitors.push_back(monitor);
    return TRUE;
}

void MonitorResolver::update_monitors_data() {
    for (auto i : MonitorResolver::monitors) {
        delete i;
    }
    MonitorResolver::monitors.clear();
    EnumDisplayMonitors(0, 0, MonitorResolver::monitor_enum, 0);
}