#pragma once


#include <ObjectArray.h>
#include <hstring.h>
#include <inspectable.h>
#include <objbase.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "v_desktop_api.h"
#include "../thumbnail/thumbnail.h"

class VirtualDesktop {
   public:
    std::wstring guid = L"";
    std::vector<Thumbnail *> windows;
    int index = 0;
    int render_left = 0;
    int render_top = 0;
    int render_right = 0;
    int render_bottom = 0;
    IVirtualDesktop *i_vt = nullptr;
    bool is_active = false;
    ~VirtualDesktop();
    VirtualDesktop(std::wstring guid = L"", int index = 0, IVirtualDesktop *i_vt = nullptr, bool is_active = false, int render_left = 0, int render_top = 0, int render_right = 0, int render_bottom = 0);

    int get_width() { return this->render_right - this->render_left; }
    int get_height() { return this->render_bottom - this->render_top; }
    int get_x() { return this->render_left; }
    int get_y() { return this->render_top; }
    int get_x2() { return this->render_right; }
    int get_y2() { return this->render_bottom; }
};