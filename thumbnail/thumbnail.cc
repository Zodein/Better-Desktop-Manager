#include "thumbnail.h"

Thumbnail::Thumbnail(HWND self_hwnd, HWND destination_hwnd) {
    this->self_hwnd = self_hwnd;
    this->destination_hwnd = destination_hwnd;
}

Thumbnail::Thumbnail(HWND self_hwnd, HWND destination_hwnd, int order) {
    this->self_hwnd = self_hwnd;
    this->destination_hwnd = destination_hwnd;
    this->order = order;
}

Thumbnail::Thumbnail(HWND self_hwnd, HWND destination_hwnd, int order, int x, int y, int width, int height) {
    this->self_hwnd = self_hwnd;
    this->destination_hwnd = destination_hwnd;
    this->order = order;
    this->window_position.x = x;
    this->window_position.y = y;
    this->window_position.width = width;
    this->window_position.height = height;
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
    }
    return;
}

int Thumbnail::update_n_get_x() {
    this->update_window_position();
    return this->window_position.x;
}

int Thumbnail::update_n_get_y() {
    this->update_window_position();
    return this->window_position.y;
}

int Thumbnail::update_n_get_width() {
    this->update_window_position();
    return this->window_position.width;
}

int Thumbnail::update_n_get_height() {
    this->update_window_position();
    return this->window_position.height;
}

double Thumbnail::update_n_get_ratio() {
    this->update_window_position();
    return (double)this->window_position.width / (double)this->window_position.height;
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