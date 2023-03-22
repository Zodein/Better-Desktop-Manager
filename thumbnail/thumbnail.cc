#include "thumbnail.h"

#include "iostream"

Thumbnail::Thumbnail(HWND self_hwnd, HWND destination_hwnd, int order, Monitor* on_monitor, std::wstring title) {
    this->self_hwnd = self_hwnd;
    this->destination_hwnd = destination_hwnd;
    this->order = order;
    this->on_monitor = on_monitor;
    this->title = title;
}

Thumbnail::~Thumbnail() {
    // std::cout << "thumb destroyed\n";
    if (this->registered) this->unregister_thumbnail();
    if (this->bmp != nullptr) this->bmp->Release();
}

void Thumbnail::register_thumbnail() {
    HRESULT hr = DwmRegisterThumbnail(this->destination_hwnd, this->self_hwnd, &thumbnail);
    if (SUCCEEDED(hr)) {
        RECT dest = {this->thumbnail_position.x, this->thumbnail_position.y, this->thumbnail_position.x + this->thumbnail_position.width, this->thumbnail_position.y + this->thumbnail_position.height};
        this->thumbnail_properties.dwFlags = DWM_TNP_VISIBLE | DWM_TNP_RECTDESTINATION;
        this->thumbnail_properties.fSourceClientAreaOnly = FALSE;
        this->thumbnail_properties.fVisible = TRUE;
        this->thumbnail_properties.opacity = 255;
        this->thumbnail_properties.rcDestination = dest;
        hr = DwmUpdateThumbnailProperties(thumbnail, &this->thumbnail_properties);
        this->registered = true;
    }
    return;
}

void Thumbnail::update_thumbnail_position() {
    this->thumbnail_properties.rcDestination = {this->thumbnail_position.x, this->thumbnail_position.y, this->thumbnail_position.x + this->thumbnail_position.width, this->thumbnail_position.y + this->thumbnail_position.height};
    DwmUpdateThumbnailProperties(thumbnail, &thumbnail_properties);
    return;
}

void Thumbnail::repose_thumbnail(int x, int y) {
    RECT dest = {x, y, x + this->thumbnail_position.width, y + this->thumbnail_position.height};
    this->thumbnail_properties.rcDestination = dest;
    this->thumbnail_position.x = x;
    this->thumbnail_position.y = y;
    DwmUpdateThumbnailProperties(thumbnail, &this->thumbnail_properties);
    return;
}

void Thumbnail::unregister_thumbnail() {
    this->registered = false;
    DwmUnregisterThumbnail(this->thumbnail);
    return;
}

void Thumbnail::update_window_position() {
    RECT rect;
    if (IsIconic(this->self_hwnd)) {
        WINDOWPLACEMENT placement;
        GetWindowPlacement(this->self_hwnd, &placement);
        if (placement.flags == 2) {
            rect.left = this->on_monitor->get_x() - 8;
            rect.top = this->on_monitor->get_y() - 8;
            rect.right = this->on_monitor->get_width() + 8;
            rect.bottom = this->on_monitor->get_height() + 8;
        } else {
            rect.left = placement.rcNormalPosition.left;
            rect.top = placement.rcNormalPosition.top;
            rect.right = placement.rcNormalPosition.right;
            rect.bottom = placement.rcNormalPosition.bottom;
        }
    } else {
        GetWindowRect(this->self_hwnd, &rect);
    }
    this->window_position.x = rect.left;
    this->window_position.y = rect.top;
    this->window_position.width = rect.right - rect.left;
    this->window_position.height = rect.bottom - rect.top;
    this->ratio = (double)this->window_position.width / (double)this->window_position.height;
    return;
}