#include "v_desktop.h"

VirtualDesktop::~VirtualDesktop() {
    for (auto i : windows) {
        delete i;
    }
    windows.clear();
    if (i_vt != nullptr)
        i_vt->Release();
}

VirtualDesktop::VirtualDesktop(std::wstring guid, int index, IVirtualDesktop *i_vt, bool is_active, int render_left, int render_top, int render_right, int render_bottom) {
    this->guid = guid;
    this->index = index;
    this->i_vt = i_vt;
    this->is_active = is_active;
    this->render_left = render_left;
    this->render_top = render_top;
    this->render_right = render_right;
    this->render_bottom = render_bottom;
    this->windows;
}