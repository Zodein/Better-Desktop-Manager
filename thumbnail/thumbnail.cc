#include "thumbnail.h"

#include "iostream"

Thumbnail::Thumbnail(HWND self_hwnd, HWND destination_hwnd, int order) {
    this->self_hwnd = self_hwnd;
    this->destination_hwnd = destination_hwnd;
    this->order = order;
}

Thumbnail::~Thumbnail() {
    if (this->registered) this->unregister_thumbnail();
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

void Thumbnail::unregister_thumbnail() {
    this->registered = false;
    DwmUnregisterThumbnail(this->thumbnail);
    return;
}

void Thumbnail::update_window_position() {
    RECT rect;
    WINDOWPLACEMENT placement;
    BOOL result = GetWindowPlacement(this->self_hwnd, &placement);
    GetWindowRect(this->self_hwnd, &rect);
    if (IsIconic(this->self_hwnd)) {
        if (placement.flags == 2) {
            rect.bottom = 1448;
            rect.left = -8;
            rect.right = 3448;
            rect.top = -8;
        } else {
            rect.bottom = placement.rcNormalPosition.bottom;
            rect.left = placement.rcNormalPosition.left;
            rect.right = placement.rcNormalPosition.right;
            rect.top = placement.rcNormalPosition.top;
        }
    }
    this->window_position.width = rect.right - rect.left;
    this->window_position.height = rect.bottom - rect.top;
    this->ratio = (double)this->window_position.width / (double)this->window_position.height;
    return;
}