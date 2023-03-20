#include "window_switcher.h"

#include <windowsx.h>

#include <thread>

#include "../monitor_resolver/monitor_resolver.h"
#include "../thumbnail/thumbnail.h"
#include "gdiplus.h"
#include "iostream"
#include "wingdi.h"
#include "winuser.h"

void HR(HRESULT const result) {
    if (S_OK != result) {
        throw ComException(result);
    }
}

template <class T>
void SafeRelease(T** ppT) {
    if (*ppT) {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

WNDCLASSEX WindowSwitcher::wc;
D2D1_COLOR_F const WindowSwitcher::background_color = D2D1::ColorF(0.0, 0.0, 0.0, 0.5f);
D2D1_COLOR_F const WindowSwitcher::on_mouse_color = D2D1::ColorF(0.5, 0.5, 0.5, 1.0f);
D2D1_COLOR_F const WindowSwitcher::selected_color = D2D1::ColorF(1.0, 1.0, 1.0, 1.0f);
D2D1_COLOR_F const WindowSwitcher::title_bg_color = D2D1::ColorF(0.05, 0.05, 0.05, 0.8f);
D2D1_COLOR_F const WindowSwitcher::active_vt_bg_color = D2D1::ColorF(0.22, 0.8, 1.0, 0.8f);

auto user32Lib = LoadLibrary(L"user32.dll");
auto lSetWindowCompositionAttribute = (SetWindowCompositionAttribute)GetProcAddress(user32Lib, "SetWindowCompositionAttribute");

void WindowSwitcher::CreateRoundRect(int x, int y, int x2, int y2, int leftTop, int rightTop, int rightBottom, int leftBottom, ID2D1Brush* brush) {
    ID2D1GeometrySink* sink = nullptr;
    ID2D1PathGeometry* path = nullptr;

    d2Factory->CreatePathGeometry(&path);
    path->Open(&sink);

    D2D1_POINT_2F p[2];

    p[0].x = x + leftTop;
    p[0].y = y;
    sink->BeginFigure(p[0], D2D1_FIGURE_BEGIN::D2D1_FIGURE_BEGIN_FILLED);
    p[1].x = x2 - rightTop;
    p[1].y = y;
    sink->AddLines(p, 2);

    p[0].x = x2;
    p[0].y = y + rightTop;

    if (rightTop) {
        D2D1_POINT_2F p2 = D2D1::Matrix3x2F::Rotation(0, p[1]).TransformPoint(p[0]);
        sink->AddArc(D2D1::ArcSegment(p2, D2D1::SizeF(rightTop, rightTop), 0, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
    }

    p[1].x = x2;
    p[1].y = y2 - rightBottom;
    sink->AddLines(p, 2);

    p[0].x = x2 - rightBottom;
    p[0].y = y2;

    if (rightBottom) {
        D2D1_POINT_2F p2 = D2D1::Matrix3x2F::Rotation(0, p[1]).TransformPoint(p[0]);
        sink->AddArc(D2D1::ArcSegment(p2, D2D1::SizeF(rightBottom, rightBottom), 0, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
    }

    p[1].x = x + leftBottom;
    p[1].y = y2;
    sink->AddLines(p, 2);

    p[0].x = x;
    p[0].y = y2 - leftBottom;
    if (leftBottom) {
        D2D1_POINT_2F p2 = D2D1::Matrix3x2F::Rotation(0, p[1]).TransformPoint(p[0]);
        sink->AddArc(D2D1::ArcSegment(p2, D2D1::SizeF(leftBottom, leftBottom), 0, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
    }

    p[1].x = x;
    p[1].y = y + leftTop;
    sink->AddLines(p, 2);
    p[0].x = x + leftTop;
    p[0].y = y;
    if (leftTop) {
        D2D1_POINT_2F p2 = D2D1::Matrix3x2F::Rotation(0, p[1]).TransformPoint(p[0]);
        sink->AddArc(D2D1::ArcSegment(p2, D2D1::SizeF(leftTop, leftTop), 0, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_SMALL));
    }

    sink->EndFigure(D2D1_FIGURE_END::D2D1_FIGURE_END_CLOSED);
    sink->Close();

    dc->FillGeometry(path, brush);

    SafeRelease(&sink);
    SafeRelease(&path);

    return;
}

LRESULT CALLBACK WindowSwitcher::window_proc_static(HWND handle_window, UINT message, WPARAM w_param, LPARAM l_param) {
    WindowSwitcher* window_switcher = reinterpret_cast<WindowSwitcher*>(GetWindowLongPtr(handle_window, GWLP_USERDATA));
    if (window_switcher) return window_switcher->window_proc(handle_window, message, w_param, l_param);
    return DefWindowProc(handle_window, message, w_param, l_param);
}

LRESULT CALLBACK WindowSwitcher::window_proc(HWND handle_window, UINT message, WPARAM w_param, LPARAM l_param) {
    switch (message) {
        case WM_MOUSEWHEEL: {
            this->on_mousewheel_event(w_param, l_param);
            break;
        }
        case WM_LBUTTONDOWN: {
            this->on_mouseldown_event(w_param, l_param);
            break;
        }
        case WM_LBUTTONUP: {
            this->on_mouselup_event(w_param, l_param);
            break;
        }
        case WM_MBUTTONDOWN: {
            this->on_mousemdown_event(w_param, l_param);
            break;
        }
        case WM_HOTKEY: {
            this->on_hotkey_event();
            break;
        }
        case WM_PAINT: {
            this->on_print_event();
            std::cout << "WM_PAINT\n";
            break;
        }
        case WM_ERASEBKGND: {
            this->on_print_event();
            std::cout << "WM_ERASEBKGND\n";
            break;
        }
        case WM_CLOSE:
            DestroyWindow(handle_window);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_MOUSEMOVE: {
            this->on_mousemove_event(l_param);
            break;
        }
        default:
            return DefWindowProc(handle_window, message, w_param, l_param);
    }
    return 0;
}

WindowSwitcher::WindowSwitcher(Monitor* monitor, std::vector<WindowSwitcher*>* window_switchers, DWORD MAIN_THREAD_ID) {
    this->MAIN_THREAD_ID = MAIN_THREAD_ID;
    this->window_switchers = window_switchers;
    this->monitor = monitor;
    this->thumbnail_manager = new ThumbnailManager(this, this->monitor->get_width() - 128, this->monitor->get_height() - 128);
}

int WindowSwitcher::create_window() {
    this->hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, WindowSwitcher::wc.lpszClassName, L"Better Desktop Manager", WS_POPUP, this->monitor->get_x(), this->monitor->get_y(), this->monitor->get_width(), this->monitor->get_height(), NULL, NULL, NULL, NULL);

    if (this->hwnd == NULL) {
        DWORD x = GetLastError();
        MessageBox(NULL, L"Window Creation Failed!", L"Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    HR(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, D3D11_CREATE_DEVICE_BGRA_SUPPORT, nullptr, 0, D3D11_SDK_VERSION, &direct3dDevice, nullptr, nullptr));
    HR(direct3dDevice.As(&dxgiDevice));
    HR(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, __uuidof(dxFactory), reinterpret_cast<void**>(dxFactory.GetAddressOf())));
    DXGI_SWAP_CHAIN_DESC1 description = {};
    description.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    description.BufferCount = 2;
    description.SampleDesc.Count = 1;
    description.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
    description.Width = monitor->get_width();
    description.Height = monitor->get_height();
    HR(dxFactory->CreateSwapChainForComposition(dxgiDevice.Get(), &description, nullptr, swapChain.GetAddressOf()));
    D2D1_FACTORY_OPTIONS const options = {D2D1_DEBUG_LEVEL_INFORMATION};
    HR(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, options, d2Factory.GetAddressOf()));
    HR(d2Factory->CreateDevice(dxgiDevice.Get(), d2Device.GetAddressOf()));
    HR(d2Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, dc.GetAddressOf()));
    HR(swapChain->GetBuffer(0, __uuidof(surface), reinterpret_cast<void**>(surface.GetAddressOf())));
    D2D1_BITMAP_PROPERTIES1 properties = {};
    properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
    properties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
    properties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
    ComPtr<ID2D1Bitmap1> bitmap;
    HR(dc->CreateBitmapFromDxgiSurface(surface.Get(), properties, bitmap.GetAddressOf()));
    dc->SetTarget(bitmap.Get());
    HR(DCompositionCreateDevice(dxgiDevice.Get(), __uuidof(dcompDevice), reinterpret_cast<void**>(dcompDevice.GetAddressOf())));
    HR(dcompDevice->CreateTargetForHwnd(hwnd, true, target.GetAddressOf()));
    HR(dcompDevice->CreateVisual(visual.GetAddressOf()));
    HR(visual->SetContent(swapChain.Get()));
    HR(target->SetRoot(visual.Get()));
    HR(dcompDevice->Commit());

    HR(dc->CreateSolidColorBrush(WindowSwitcher::background_color, this->background_brush.GetAddressOf()));
    HR(dc->CreateSolidColorBrush(WindowSwitcher::on_mouse_color, this->on_mouse_brush.GetAddressOf()));
    HR(dc->CreateSolidColorBrush(WindowSwitcher::selected_color, this->selected_brush.GetAddressOf()));
    HR(dc->CreateSolidColorBrush(WindowSwitcher::title_bg_color, this->title_bg_brush.GetAddressOf()));
    HR(dc->CreateSolidColorBrush(WindowSwitcher::active_vt_bg_color, this->active_vt_bg_brush.GetAddressOf()));

    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(writeFactory), reinterpret_cast<IUnknown**>(&writeFactory));
    writeFactory->CreateTextFormat(L"Segoe UI", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 14.0f, L"en-us", &writeTextFormat);
    writeTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    writeTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    writeFactory->CreateTextFormat(L"Segoe UI", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 64.0f, L"en-us", &virtual_desktop_label_format);
    virtual_desktop_label_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    virtual_desktop_label_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    if (user32Lib) {
        AccentPolicy accent;
        WINDOWCOMPOSITIONATTRIBDATA composition_data;
        composition_data.pvData = &accent;
        composition_data.cbData = sizeof(accent);
        lSetWindowCompositionAttribute(hwnd, &composition_data);
    }

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);
    InvalidateRect(this->hwnd, NULL, FALSE);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

void WindowSwitcher::activate_window(HWND hwnd_window) {
    if (IsIconic(hwnd_window)) ShowWindow(hwnd_window, SW_RESTORE);
    SetForegroundWindow(hwnd_window);
}

void WindowSwitcher::show_window() {
    if (this->thumbnail_manager->thumbnails.size() > 0)
        this->selected_window = this->thumbnail_manager->thumbnails.size() > 1;
    else
        this->selected_window = -1;
    SetWindowPos(this->hwnd, HWND_TOPMOST, this->monitor->get_x(), this->monitor->get_y(), this->monitor->get_width(), this->monitor->get_height(), SWP_SHOWWINDOW);
    return;
}

void WindowSwitcher::hide_window() {
    ShowWindow(this->hwnd, 0);
    PostThreadMessage(MAIN_THREAD_ID, MSG_DESTROY_THUMBNAILS, 0, 0);
    return;
}

void WindowSwitcher::render() {
    dc->BeginDraw();
    dc->Clear();

    auto _this = *this;

    dc->FillRectangle(D2D1::RectF(0, 0, monitor->get_width(), monitor->get_height()), _this.background_brush.Get());
    for (auto i : _this.thumbnail_manager->thumbnails) {
        int border_width = 1;
        rect.left = i->thumbnail_position.x - border_width;
        rect.top = i->thumbnail_position.y - border_width - _this.title_height;
        rect.right = i->thumbnail_position.x + i->thumbnail_position.width + border_width;
        rect.bottom = i->thumbnail_position.y + i->thumbnail_position.height + border_width;
        _this.CreateRoundRect(rect.left, rect.top, rect.right, rect.bottom, 16, 16, 0, 0, _this.on_mouse_brush.Get());
    }

    if (_this.selected_window >= 0) {
        int border_width = _this.mouse_on == _this.selected_window ? 6 : 2;
        rect.left = _this.thumbnail_manager->thumbnails[_this.selected_window]->thumbnail_position.x - border_width;
        rect.top = _this.thumbnail_manager->thumbnails[_this.selected_window]->thumbnail_position.y - border_width - _this.title_height;
        rect.right = _this.thumbnail_manager->thumbnails[_this.selected_window]->thumbnail_position.x + _this.thumbnail_manager->thumbnails[_this.selected_window]->thumbnail_position.width + border_width;
        rect.bottom = _this.thumbnail_manager->thumbnails[_this.selected_window]->thumbnail_position.y + _this.thumbnail_manager->thumbnails[_this.selected_window]->thumbnail_position.height + border_width;
        _this.CreateRoundRect(rect.left, rect.top, rect.right, rect.bottom, 16, 16, 0, 0, _this.selected_brush.Get());
    }

    if (_this.mouse_on >= 0) {
        if (_this.mouse_on < 1000000) {
            int border_width = 4;
            rect.left = _this.thumbnail_manager->thumbnails[_this.mouse_on]->thumbnail_position.x - border_width;
            rect.top = _this.thumbnail_manager->thumbnails[_this.mouse_on]->thumbnail_position.y - border_width - _this.title_height;
            rect.right = _this.thumbnail_manager->thumbnails[_this.mouse_on]->thumbnail_position.x + _this.thumbnail_manager->thumbnails[_this.mouse_on]->thumbnail_position.width + border_width;
            rect.bottom = _this.thumbnail_manager->thumbnails[_this.mouse_on]->thumbnail_position.y + _this.thumbnail_manager->thumbnails[_this.mouse_on]->thumbnail_position.height + border_width;
            _this.CreateRoundRect(rect.left, rect.top, rect.right, rect.bottom, 16, 16, 0, 0, _this.on_mouse_brush.Get());
        }
    }

    for (auto i : _this.thumbnail_manager->thumbnails) {
        std::wstring s;
        int len = GetWindowTextLength(i->self_hwnd) + 1;
        len = len < 100 ? len : 100;
        s.resize(len);
        GetWindowText(i->self_hwnd, LPWSTR(s.c_str()), len);
        if (len == 100) {
            s += +L"...";
            len = 103;
        }
        rect.left = i->thumbnail_position.x;
        rect.top = i->thumbnail_position.y - _this.title_height;
        rect.right = i->thumbnail_position.x + i->thumbnail_position.width;
        rect.bottom = i->thumbnail_position.y;
        _this.CreateRoundRect(rect.left, rect.top, rect.right, rect.bottom, 16, 16, 0, 0, _this.title_bg_brush.Get());
        dc->DrawText(s.c_str(), len, writeTextFormat, D2D1::RectF(rect.left, rect.top, rect.right, rect.bottom), _this.selected_brush.Get());
    }

    this->render_vdesktops();

    HR(dc->EndDraw());
    HR(swapChain->Present(1, 0));
    ValidateRect(hwnd, NULL);
}

void WindowSwitcher::render_vdesktops() {
    std::thread t1([this]() {
        auto _this = *this;
        int desktop_count = _this.thumbnail_manager->virtual_desktops.size();
        // int reference_y = (_this.monitor->get_height() * 0.75) + virtual_desktop_vertical_margin;
        // int height = (_this.monitor->get_height() * 0.25) - (virtual_desktop_vertical_margin * 2);
        // double monitor_ratio = (double)monitor->get_width() / (double)monitor->get_height();

        for (int i = 0; i < desktop_count; i++) {
            // rect.left = (i * ((height * monitor_ratio) + virtual_desktop_horizontal_margin)) + virtual_desktop_horizontal_margin;
            // rect.top = reference_y;
            // rect.right = rect.left + (height * monitor_ratio);
            // rect.bottom = rect.top + height;

            auto temp_obj = _this.thumbnail_manager->virtual_desktops_index.find(i);
            if (temp_obj != _this.thumbnail_manager->virtual_desktops_index.end()) {
                auto temp_obj2 = _this.thumbnail_manager->virtual_desktops.find(temp_obj->second);
                if (temp_obj2 != _this.thumbnail_manager->virtual_desktops.end()) {
                    // temp_obj2->second->render_left = rect.left;
                    // temp_obj2->second->render_top = rect.top;
                    // temp_obj2->second->render_right = rect.right;
                    // temp_obj2->second->render_bottom = rect.bottom;
                    if (temp_obj2->second->is_active) {
                        _this.CreateRoundRect(temp_obj2->second->render_left, temp_obj2->second->render_top, temp_obj2->second->render_right, temp_obj2->second->render_bottom, 8, 8, 8, 8, _this.active_vt_bg_brush.Get());
                    } else {
                        _this.CreateRoundRect(temp_obj2->second->render_left, temp_obj2->second->render_top, temp_obj2->second->render_right, temp_obj2->second->render_bottom, 8, 8, 8, 8, _this.selected_brush.Get());
                    }
                    dc->DrawText(std::to_wstring(temp_obj2->second->window_count).c_str(), temp_obj2->second->window_count > 0 ? (int)log10((double)temp_obj2->second->window_count) + 1 : 1, virtual_desktop_label_format, D2D1::RectF(temp_obj2->second->render_right - 80, temp_obj2->second->render_bottom - 80, temp_obj2->second->render_right, temp_obj2->second->render_bottom),
                                 _this.title_bg_brush.Get());
                    dc->DrawText(std::to_wstring(i + 1).c_str(), i + 1 > 0 ? (int)log10((double)i + 1) + 1 : 1, virtual_desktop_label_format, D2D1::RectF(temp_obj2->second->render_right - 80, temp_obj2->second->render_top, temp_obj2->second->render_right, temp_obj2->second->render_top + 80), _this.title_bg_brush.Get());
                }
            }

            // for (auto i : _this.thumbnail_manager->virtual_desktops) {
            //     dc->DrawText(std::to_wstring(i.second->index).c_str(), 1, writeTextFormat, D2D1::RectF(rect.left, rect.top, rect.right, rect.bottom), _this.title_bg_brush.Get());
            // }

            // IVirtualDesktop* virtual_desktop;
            // GUID guid;
            // auto hr = VirtualDesktopManager::desktop_manager_internal->GetCurrentDesktop(0, &virtual_desktop);
            // if (SUCCEEDED(hr)) {
            //     virtual_desktop->GetID(&guid);
            //     WCHAR id[256];
            //     hr = StringFromGUID2(guid, id, _countof(id));
            //     if (SUCCEEDED(hr)) {
            //         auto temp = _this.thumbnail_manager->virtual_desktops.find(std::wstring(id));
            //         if (temp != _this.thumbnail_manager->virtual_desktops.end()) {
            //             dc->DrawText(std::to_wstring(temp->second->index).c_str(), 1, writeTextFormat, D2D1::RectF(rect.left, rect.top, rect.right, rect.bottom), _this.title_bg_brush.Get());
            //         }
            //     }
            // }
        }
        std::cout << "desktops rendered";
    });
    t1.join();
    return;
}

void WindowSwitcher::on_hotkey_event() {
    // if (!IsWindowVisible(this->hwnd)) {
    // this->show_window();
    std::thread t1([=]() {
        int last_checked = 0;
        while (GetAsyncKeyState(0x42)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if (!IsWindowVisible(this->hwnd)) return;
            if (last_checked < GetTickCount()) {
                PostThreadMessage(MAIN_THREAD_ID, MSG_UPDATE_THUMBNAILS_IF_NEEDED, 0, 0);
                // this->thumbnail_manager->update_thumbnails_if_needed();
                last_checked = GetTickCount() + 100;
            }
        }
        if (IsWindowVisible(this->hwnd)) {
            POINT p;
            if (GetCursorPos(&p)) {
                if (p.x > this->monitor->get_x()) {
                    if (p.x < this->monitor->get_x2()) {
                        if (p.y > this->monitor->get_y()) {
                            if (p.y < this->monitor->get_y2()) {
                                if (selected_window > 0) this->activate_window(this->thumbnail_manager->thumbnails[this->selected_window]->self_hwnd);
                            }
                        }
                    }
                }
            }
        }
        this->hide_window();
        return;
    });
    t1.detach();
    std::cout << "HOTKEY\n";
    // }
}

void WindowSwitcher::on_print_event() { this->render(); }

void WindowSwitcher::on_mousewheel_event(WPARAM w_param, LPARAM l_param) {
    int x = GET_X_LPARAM(l_param);
    int y = GET_Y_LPARAM(l_param);
    if (y > (this->monitor->get_y2() - (this->monitor->get_height() * 0.25))) {
        int remaining_vt = (this->thumbnail_manager->virtual_desktops.size() - (this->monitor->get_width() / (this->monitor->vt_size->width + this->monitor->vt_size->h_margin)));
        if (remaining_vt > 0) {
            if (GET_WHEEL_DELTA_WPARAM(w_param) < 0 && this->virtual_desktop_scroll < int(remaining_vt * (this->monitor->vt_size->width + this->monitor->vt_size->h_margin)))
                this->virtual_desktop_scroll += (this->monitor->vt_size->width + this->monitor->vt_size->h_margin) / 1.4;
            else if (GET_WHEEL_DELTA_WPARAM(w_param) > 0 && (this->virtual_desktop_scroll > -256))
                this->virtual_desktop_scroll -= this->monitor->vt_size->width;
            else
                return;
        } else
            return;
        this->thumbnail_manager->calculate_all_virtualdesktops_positions();
        InvalidateRect(this->hwnd, NULL, FALSE);
    } else {
        if (GET_WHEEL_DELTA_WPARAM(w_param) < 0 && this->selected_window + 1 < this->thumbnail_manager->thumbnails.size())
            this->selected_window++;
        else if (GET_WHEEL_DELTA_WPARAM(w_param) > 0 && this->selected_window > 0)
            this->selected_window--;
        else
            return;
        InvalidateRect(this->hwnd, NULL, FALSE);
    }
    return;
}

void WindowSwitcher::on_mousemdown_event(WPARAM w_param, LPARAM l_param) {
    if (this->mouse_on != -1) {
        PostMessage(this->thumbnail_manager->thumbnails[this->mouse_on]->self_hwnd, WM_CLOSE, 0, 0);
    }
    return;
}

void WindowSwitcher::on_mouseldown_event(WPARAM w_param, LPARAM l_param) {
    if (this->mouse_on != -1) {
        this->mouse_down_on[0] = GET_X_LPARAM(l_param);
        this->mouse_down_on[1] = GET_Y_LPARAM(l_param);
        this->mouse_down = true;
        if (this->mouse_on < 1000000) {
            this->catched_thumbnail_ref_coord[0] = this->thumbnail_manager->thumbnails[mouse_on]->thumbnail_position.x - GET_X_LPARAM(l_param);
            this->catched_thumbnail_ref_coord[1] = this->thumbnail_manager->thumbnails[mouse_on]->thumbnail_position.y - GET_Y_LPARAM(l_param);
            this->thumbnail_manager->thumbnails[mouse_on]->unregister_thumbnail();  // update z-order of the thumbnail
            this->thumbnail_manager->thumbnails[mouse_on]->register_thumbnail();
        } else {
            auto temp = this->thumbnail_manager->virtual_desktops_index.find(mouse_on - 1000000);
            if (temp != this->thumbnail_manager->virtual_desktops_index.end()) {
                auto temp2 = this->thumbnail_manager->virtual_desktops.find(temp->second);
                if (temp2 != this->thumbnail_manager->virtual_desktops.end()) {
                    this->catched_thumbnail_ref_coord[0] = -(this->virtual_desktop_scroll + GET_X_LPARAM(l_param));
                    this->catched_thumbnail_ref_coord[1] = 0;
                }
            }
        }
        // this->activate_window(this->thumbnail_manager->thumbnails[this->mouse_on]->self_hwnd);
        // this->hide_window();
        std::cout << "WM_LBUTTONDOWN\n";
    }
    return;
}

void WindowSwitcher::on_mouselup_event(WPARAM w_param, LPARAM l_param) {
    this->mouse_down_on[0] -= GET_X_LPARAM(l_param);
    this->mouse_down_on[1] -= GET_Y_LPARAM(l_param);
    if (this->mouse_on != -1) {
        if (this->mouse_down_on[0] < 64 && this->mouse_down_on[0] > -64) {
            if (this->mouse_down_on[1] < 64 && this->mouse_down_on[1] > -64) {
                if (this->mouse_on < 1000000) {
                    this->activate_window(this->thumbnail_manager->thumbnails[this->mouse_on]->self_hwnd);
                    this->hide_window();
                } else {
                    auto temp = this->thumbnail_manager->virtual_desktops_index.find(this->mouse_on - 1000000);
                    if (temp != this->thumbnail_manager->virtual_desktops_index.end()) {
                        auto temp2 = this->thumbnail_manager->virtual_desktops.find(temp->second);
                        if (temp2 != this->thumbnail_manager->virtual_desktops.end()) {
                            VirtualDesktopManager::go_to(temp2->second->i_vt);
                            PostThreadMessage(MAIN_THREAD_ID, MSG_UPDATE_THUMBNAILS_FORCE, 0, 0);
                        }
                    }
                }
                this->mouse_down = false;
                return;
            }
        }
        if (this->mouse_on < 1000000) {
            int x = GET_X_LPARAM(l_param);
            int y = GET_Y_LPARAM(l_param);
            for (auto i : this->thumbnail_manager->virtual_desktops) {
                if (x > i.second->render_left) {
                    if (y > i.second->render_top) {
                        if (x < i.second->render_left + i.second->get_width()) {
                            if (y < i.second->render_top + i.second->get_height()) {
                                IApplicationView* view;
                                auto hr = VirtualDesktopManager::application_view_collection->GetViewForHwnd(this->thumbnail_manager->thumbnails[mouse_on]->self_hwnd, &view);
                                if (SUCCEEDED(hr)) {
                                    VirtualDesktopManager::desktop_manager_internal->MoveViewToDesktop(view, i.second->i_vt);
                                    PostThreadMessage(MAIN_THREAD_ID, MSG_UPDATE_THUMBNAILS_FORCE, 0, 0);
                                    this->mouse_down = false;
                                    return;
                                }
                            }
                        }
                    }
                }
            }
        }
        PostThreadMessage(MAIN_THREAD_ID, MSG_UPDATE_THUMBNAIL_POS, 0, 0);
        InvalidateRect(this->hwnd, NULL, FALSE);
    }
    this->mouse_down = false;
    std::cout << "WM_LBUTTONUP\n";
    return;
}

void WindowSwitcher::on_mousemove_event(LPARAM l_param) {
    int x = GET_X_LPARAM(l_param);
    int y = GET_Y_LPARAM(l_param);
    if (this->mouse_down) {
        if (this->thumbnail_manager->thumbnails.size() >= this->mouse_on && this->mouse_on < 1000000) {
            this->thumbnail_manager->thumbnails[mouse_on]->repose_thumbnail(x + this->catched_thumbnail_ref_coord[0], y + this->catched_thumbnail_ref_coord[1]);
            InvalidateRect(this->hwnd, NULL, FALSE);
        } else if (this->thumbnail_manager->virtual_desktops.size() >= (this->mouse_on - 1000000)) {
            // this->thumbnail_manager->thumbnails[mouse_on]->repose_thumbnail(x + this->catched_thumbnail_ref_coord[0], y + this->catched_thumbnail_ref_coord[1]);
            this->virtual_desktop_scroll = -(x + this->catched_thumbnail_ref_coord[0]);
            this->thumbnail_manager->calculate_all_virtualdesktops_positions();
            InvalidateRect(this->hwnd, NULL, FALSE);
        }
        return;
    }
    for (auto i : this->thumbnail_manager->thumbnails) {
        if (x > i->thumbnail_position.x) {
            if (y > i->thumbnail_position.y) {
                if (x < i->thumbnail_position.x + i->thumbnail_position.width) {
                    if (y < i->thumbnail_position.y + i->thumbnail_position.height) {
                        if (this->mouse_on != i->order) {
                            this->mouse_on = i->order;
                            InvalidateRect(this->hwnd, NULL, FALSE);
                        }
                        return;
                    }
                }
            }
        }
    }
    for (auto i : this->thumbnail_manager->virtual_desktops) {
        if (x > i.second->render_left) {
            if (y > i.second->render_top) {
                if (x < i.second->render_left + i.second->get_width()) {
                    if (y < i.second->render_top + i.second->get_height()) {
                        if (this->mouse_on != (i.second->index + 1000000)) {
                            this->mouse_on = (i.second->index + 1000000);
                            InvalidateRect(this->hwnd, NULL, FALSE);
                        }
                        return;
                    }
                }
            }
        }
    }
    if (this->mouse_on != -1) {
        this->mouse_on = -1;
        InvalidateRect(this->hwnd, NULL, FALSE);
    }
    return;
}
