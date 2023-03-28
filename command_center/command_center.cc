#include "command_center.h"

#include <windowsx.h>

#include <thread>

#include "../monitor_resolver/monitor_resolver.h"
#include "../thumbnail/thumbnail.h"
#include "../virtual_desktop/v_desktop.h"
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

WNDCLASSEX CommandCenter::wc;
D2D1_COLOR_F const CommandCenter::background_color = D2D1::ColorF(0.0, 0.0, 0.0, 0.5);
D2D1_COLOR_F const CommandCenter::on_mouse_color = D2D1::ColorF(0.5, 0.5, 0.5, 1.0);
D2D1_COLOR_F const CommandCenter::selected_color = D2D1::ColorF(1.0, 1.0, 1.0, 1.0);
D2D1_COLOR_F const CommandCenter::title_color = D2D1::ColorF(0.05, 0.05, 0.05, 0.8);
D2D1_COLOR_F const CommandCenter::title_bg_color = D2D1::ColorF(0.05, 0.05, 0.05, 0.9);
D2D1_COLOR_F const CommandCenter::title_onmouse_bg_color = D2D1::ColorF(0.8, 0.8, 0.8, 0.8);
D2D1_COLOR_F const CommandCenter::title_active_vt_color = D2D1::ColorF(0.9, 0.9, 0.9, 0.9);
D2D1_COLOR_F const CommandCenter::vt_bg_color = D2D1::ColorF(1.0, 1.0, 1.0, 1.0);
D2D1_COLOR_F const CommandCenter::active_vt_bg_color = D2D1::ColorF(0.1, 0.1, 0.1, 0.9);

int CommandCenter::selected_border = 4;
int CommandCenter::onmouse_border = 4;

auto user32Lib = LoadLibrary(L"user32.dll");
auto lSetWindowCompositionAttribute = (SetWindowCompositionAttribute)GetProcAddress(user32Lib, "SetWindowCompositionAttribute");

void CommandCenter::CreateRoundRect(int x, int y, int x2, int y2, int leftTop, int rightTop, int rightBottom, int leftBottom, ID2D1Brush* brush) {
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

LRESULT CALLBACK CommandCenter::window_proc_static(HWND handle_window, UINT message, WPARAM w_param, LPARAM l_param) {
    CommandCenter* command_center = reinterpret_cast<CommandCenter*>(GetWindowLongPtr(handle_window, GWLP_USERDATA));
    if (command_center) return command_center->window_proc(handle_window, message, w_param, l_param);
    return DefWindowProc(handle_window, message, w_param, l_param);
}

LRESULT CALLBACK CommandCenter::window_proc(HWND handle_window, UINT message, WPARAM w_param, LPARAM l_param) {
    switch (message) {
        case WM_MOUSEWHEEL: {
            this->on_mousewheel_event(w_param, l_param);
            break;
        }
        case WM_MBUTTONDOWN: {
            this->on_mousemdown_event(w_param, l_param);
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
        case WM_HOTKEY: {
            this->on_hotkey_event();
            break;
        }
        case WM_PAINT: {
            this->render_n_detach();
            break;
        }
        case WM_ERASEBKGND: {
            this->render_n_detach();
            break;
        }
        case WM_MOUSEMOVE: {
            this->on_mousemove_event(l_param);
            break;
            case WM_CLOSE:
                this->hide_window();
                break;
            case WM_DESTROY:
                PostQuitMessage(0);
                break;
        }
        default:
            return DefWindowProc(handle_window, message, w_param, l_param);
    }
    return 0;
}

CommandCenter::CommandCenter(Monitor* monitor, std::vector<CommandCenter*>* command_centers, DWORD MAIN_THREAD_ID) {
    this->MAIN_THREAD_ID = MAIN_THREAD_ID;
    this->command_centers = command_centers;
    this->monitor = monitor;
    this->desktop_manager = new VDesktopManager(this, this->monitor->get_width() - 128, this->monitor->get_height() - 128);
}

int CommandCenter::create_window() {
    this->hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, CommandCenter::wc.lpszClassName, L"Better Desktop Manager", WS_POPUP, this->monitor->get_x(), this->monitor->get_y(), this->monitor->get_width(), this->monitor->get_height(), NULL, NULL, NULL, NULL);

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
    HR(D2D1CreateFactory(D2D1_FACTORY_TYPE_MULTI_THREADED, options, d2Factory.GetAddressOf()));
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

    HR(dc->CreateSolidColorBrush(CommandCenter::background_color, this->background_brush.GetAddressOf()));
    HR(dc->CreateSolidColorBrush(CommandCenter::on_mouse_color, this->on_mouse_brush.GetAddressOf()));
    HR(dc->CreateSolidColorBrush(CommandCenter::selected_color, this->selected_brush.GetAddressOf()));
    HR(dc->CreateSolidColorBrush(CommandCenter::title_color, this->title_brush.GetAddressOf()));
    HR(dc->CreateSolidColorBrush(CommandCenter::title_bg_color, this->title_bg_brush.GetAddressOf()));
    HR(dc->CreateSolidColorBrush(CommandCenter::title_onmouse_bg_color, this->title_onmouse_bg_brush.GetAddressOf()));
    HR(dc->CreateSolidColorBrush(CommandCenter::title_active_vt_color, this->title_active_vt_brush.GetAddressOf()));
    HR(dc->CreateSolidColorBrush(CommandCenter::vt_bg_color, this->vt_bg_brush.GetAddressOf()));
    HR(dc->CreateSolidColorBrush(CommandCenter::active_vt_bg_color, this->active_vt_bg_brush.GetAddressOf()));

    DWRITE_TRIMMING dwrite_trimming = {DWRITE_TRIMMING_GRANULARITY_NONE, 0, 0};

    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(writeFactory), reinterpret_cast<IUnknown**>(&writeFactory));
    writeFactory->CreateTextFormat(L"Segoe UI", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 14.0f, L"en-us", &writeTextFormat);
    writeTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    writeTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    writeTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    writeFactory->CreateTextFormat(L"Segoe UI", NULL, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 64.0f, L"en-us", &virtual_desktop_label_format);
    virtual_desktop_label_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    virtual_desktop_label_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    virtual_desktop_label_format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    writeFactory->CreateTextFormat(L"Segoe UI", NULL, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 18.0f, L"en-us", &virtual_desktop_window_title_format);
    virtual_desktop_window_title_format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
    virtual_desktop_window_title_format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    virtual_desktop_window_title_format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

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

void CommandCenter::activate_this_window(HWND hwnd_window) { SwitchToThisWindow(hwnd_window, TRUE); }

void CommandCenter::reset_selected() {
    if (!(*this->desktop_manager->active_desktop)) return;
    if ((int)(*this->desktop_manager->active_desktop)->windows.size() > 1)
        this->selected_window = 1;
    else if ((int)(*this->desktop_manager->active_desktop)->windows.size() > 0)
        this->selected_window = 0;
    else
        this->selected_window = -1;
}

void CommandCenter::show_window() {
    SetWindowPos(this->hwnd, HWND_TOPMOST, this->monitor->get_x(), this->monitor->get_y(), this->monitor->get_width(), this->monitor->get_height(), SWP_SHOWWINDOW);
    return;
}

void CommandCenter::hide_window() {
    ShowWindow(this->hwnd, SW_HIDE);
    std::lock_guard<std::mutex> lock(this->render_lock);
    this->mouse_on = -1;
    this->mouse_down = false;
    this->selected_window = -1;
    this->desktop_manager->unregister_thumbnails();
    return;
}

void CommandCenter::render_n_detach() {
    std::lock_guard<std::mutex> lock(this->render_lock);
    std::thread t1([=]() {
        std::lock_guard<std::mutex> lock(this->render_lock);
        this->render();
    });
    t1.detach();
}

void CommandCenter::render() {
    if (!(*this->desktop_manager->active_desktop)) return;
    auto tick = GetTickCount64();
    dc->BeginDraw();
    dc->Clear();
    std::thread t1([=]() { this->render_vdesktops(); });

    dc->FillRectangle(D2D1::RectF(0, 0, monitor->get_width(), monitor->get_height()), this->background_brush.Get());
    for (auto i : (*this->desktop_manager->active_desktop)->windows) {
        int border_width = 1;
        rect.left = i->thumbnail_position.x - border_width;
        rect.top = i->thumbnail_position.y - border_width - this->title_height;
        rect.right = i->thumbnail_position.x + i->thumbnail_position.width + border_width;
        rect.bottom = i->thumbnail_position.y + i->thumbnail_position.height + border_width;
        this->CreateRoundRect(rect.left, rect.top, rect.right, rect.bottom, 16, 16, 0, 0, this->on_mouse_brush.Get());
    }

    if (this->selected_window >= 0 && (int)(*this->desktop_manager->active_desktop)->windows.size() > this->selected_window) {
        int border_width = this->mouse_on == this->selected_window ? selected_border + 4 : selected_border;
        rect.left = (*this->desktop_manager->active_desktop)->windows[this->selected_window]->thumbnail_position.x - border_width;
        rect.top = (*this->desktop_manager->active_desktop)->windows[this->selected_window]->thumbnail_position.y - border_width - this->title_height;
        rect.right = (*this->desktop_manager->active_desktop)->windows[this->selected_window]->thumbnail_position.x + (*this->desktop_manager->active_desktop)->windows[this->selected_window]->thumbnail_position.width + border_width;
        rect.bottom = (*this->desktop_manager->active_desktop)->windows[this->selected_window]->thumbnail_position.y + (*this->desktop_manager->active_desktop)->windows[this->selected_window]->thumbnail_position.height + border_width;
        this->CreateRoundRect(rect.left, rect.top, rect.right, rect.bottom, 16, 16, 0, 0, this->selected_brush.Get());
    }

    if (this->mouse_on >= 0 && (int)(*this->desktop_manager->active_desktop)->windows.size() > this->mouse_on) {
        if (this->mouse_on < 1000000) {
            int border_width = onmouse_border;
            rect.left = (*this->desktop_manager->active_desktop)->windows[this->mouse_on]->thumbnail_position.x - border_width;
            rect.top = (*this->desktop_manager->active_desktop)->windows[this->mouse_on]->thumbnail_position.y - border_width - this->title_height;
            rect.right = (*this->desktop_manager->active_desktop)->windows[this->mouse_on]->thumbnail_position.x + (*this->desktop_manager->active_desktop)->windows[this->mouse_on]->thumbnail_position.width + border_width;
            rect.bottom = (*this->desktop_manager->active_desktop)->windows[this->mouse_on]->thumbnail_position.y + (*this->desktop_manager->active_desktop)->windows[this->mouse_on]->thumbnail_position.height + border_width;
            this->CreateRoundRect(rect.left, rect.top, rect.right, rect.bottom, 16, 16, 0, 0, this->on_mouse_brush.Get());
        }
    }

    for (auto i : (*this->desktop_manager->active_desktop)->windows) {
        rect.left = i->thumbnail_position.x;
        rect.top = i->thumbnail_position.y - this->title_height;
        rect.right = i->thumbnail_position.x + i->thumbnail_position.width;
        rect.bottom = i->thumbnail_position.y;

        this->CreateRoundRect(rect.left, rect.top, rect.right, rect.bottom, 16, 16, 0, 0, this->title_bg_brush.Get());
        {
            int max_len = i->thumbnail_position.width / 8;
            std::wstring title;
            if (i->title.size() > max_len) {
                title = i->title.substr(0, max_len).append(L"...");
            } else {
                title = i->title;
            }
            dc->DrawText(title.c_str(), title.size(), writeTextFormat, D2D1::RectF(rect.left, rect.top, rect.right, rect.bottom), this->selected_brush.Get());
        }

        // icon
        if (i->bmp != nullptr) {
            dc->DrawBitmap(i->bmp, D2D1::RectF(rect.left + 8, rect.top + 4, rect.left + 8 + 16, rect.top + 4 + 16));
        }
    }

    t1.join();
    HR(dc->EndDraw());
    HR(swapChain->Present(1, 0));
    ValidateRect(hwnd, NULL);
}

void CommandCenter::render_vdesktops() {
    int desktop_count = this->desktop_manager->virtual_desktops.size();
    for (auto i : this->desktop_manager->virtual_desktops_index) {
        auto temp_obj2 = this->desktop_manager->virtual_desktops.find(i.second);
        if (temp_obj2 != this->desktop_manager->virtual_desktops.end()) {
            if (temp_obj2->second->guid.compare(L"add_desktop") == 0) {
                this->CreateRoundRect(temp_obj2->second->render_left, temp_obj2->second->render_top, temp_obj2->second->render_right, temp_obj2->second->render_bottom, 8, 8, 8, 8, this->vt_bg_brush.Get());
                dc->DrawText(std::wstring(L"+").c_str(), 1, virtual_desktop_label_format, D2D1::RectF(temp_obj2->second->render_left, temp_obj2->second->render_top, temp_obj2->second->render_right, temp_obj2->second->render_bottom), this->title_bg_brush.Get());
            } else {
                ID2D1SolidColorBrush* _bg_brush = nullptr;
                ID2D1SolidColorBrush* _title_brush = nullptr;
                if (temp_obj2->second->guid.compare(VDesktopAPI::get_current_desktop_guid_as_string()) == 0) {
                    _bg_brush = active_vt_bg_brush.Get();
                    _title_brush = title_active_vt_brush.Get();
                } else {
                    _bg_brush = vt_bg_brush.Get();
                    _title_brush = title_bg_brush.Get();
                }
                this->CreateRoundRect(temp_obj2->second->render_left, temp_obj2->second->render_top, temp_obj2->second->render_right, temp_obj2->second->render_bottom, 8, 8, 8, 8, _bg_brush);
                dc->DrawText(std::to_wstring(i.first + 1).c_str(), i.first + 1 > 0 ? (int)log10((double)i.first + 1) + 1 : 1, virtual_desktop_label_format, D2D1::RectF(temp_obj2->second->render_right - 80, temp_obj2->second->render_bottom - 80, temp_obj2->second->render_right, temp_obj2->second->render_bottom), _title_brush);
                dc->DrawText(std::to_wstring(temp_obj2->second->windows.size()).c_str(), temp_obj2->second->windows.size() > 0 ? (int)log10((double)temp_obj2->second->windows.size()) + 1 : 1, virtual_desktop_label_format, D2D1::RectF(temp_obj2->second->render_right - 80, temp_obj2->second->render_top, temp_obj2->second->render_right, temp_obj2->second->render_top + 80), _title_brush);
                int u = 0;
                for (auto i : temp_obj2->second->windows) {
                    std::wstring title;
                    if (i->title.size() > this->monitor->vt_size->title_maxsize_on_vt) {
                        title = i->title.substr(0, this->monitor->vt_size->title_maxsize_on_vt).append(L"...");
                    } else {
                        title = i->title;
                    }
                    dc->DrawText(title.c_str(), title.size(), virtual_desktop_window_title_format, D2D1::RectF(temp_obj2->second->render_left + 48, temp_obj2->second->render_top + 32 + (32 * u), temp_obj2->second->render_right - 96, temp_obj2->second->render_top + 32 + 32 + (32 * u)), _title_brush);
                    if (i->bmp != nullptr) {
                        dc->DrawBitmap(i->bmp, D2D1::RectF(temp_obj2->second->render_left + 16, temp_obj2->second->render_top + 8 + 32 + (32 * u), temp_obj2->second->render_left + 16 + 16, temp_obj2->second->render_top + 8 + 16 + 32 + (32 * u)));
                    }
                    u++;
                    if (u >= this->monitor->vt_size->max_title_draw) break;
                }
            }
        }
    }
    return;
}

void CommandCenter::on_hotkey_event() {
    int last_checked = 0;
    this->desktop_manager->refresh_data();
    this->desktop_manager->register_thumbnails();
    this->reset_selected();
    this->show_window();
    // InvalidateRect(this->hwnd, NULL, FALSE);
    this->render_n_detach();
    std::thread t1([=]() {
        int i = 0;
        while (GetAsyncKeyState(VK_OEM_102)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            if (!IsWindowVisible(this->hwnd)) return;
            if (i > 20) {
                this->desktop_manager->refresh_data();
                i = 0;
            }
            i++;
        }
        this->mouse_down = false;
        this->mouse_on = -1;
        if ((*this->desktop_manager->active_desktop) && IsWindowVisible(this->hwnd)) {
            POINT p;
            if (GetCursorPos(&p)) {
                if (p.x >= this->monitor->get_x()) {
                    if (p.x <= this->monitor->get_x2()) {
                        if (p.y >= this->monitor->get_y()) {
                            if (p.y <= this->monitor->get_y2()) {
                                if (selected_window >= 0 && (int)(*this->desktop_manager->active_desktop)->windows.size() > selected_window) {
                                    this->activate_this_window((*this->desktop_manager->active_desktop)->windows[this->selected_window]->self_hwnd);
                                }
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

void CommandCenter::on_print_event() {
    std::lock_guard<std::mutex> lock(this->render_lock);
    this->render();
}

void CommandCenter::on_mousewheel_event(WPARAM w_param, LPARAM l_param) {
    if (!(*this->desktop_manager->active_desktop)) return;
    int x = GET_X_LPARAM(l_param);
    int y = GET_Y_LPARAM(l_param);
    // if (y > (this->monitor->get_y2() - (this->monitor->get_height() * 0.25))) {// move vdesktop bar with scroll disabled
    if (y > (this->monitor->get_y2() - this->monitor->vt_size->v_margin)) {
        if (y > (this->monitor->get_y2() - this->monitor->vt_size->v_margin)) {
            if (GET_WHEEL_DELTA_WPARAM(w_param) < 0) {
                VDesktopAPI::goto_next_desktop();
                for (auto i : *this->command_centers) {
                    std::thread t1([=]() {
                        {  // repose if desktop out of screen or close to the edge
                            int new_index = (*i->desktop_manager->active_desktop)->index;
                            int new_pose = -(i->monitor->get_width() / 2) + (new_index * (i->monitor->vt_size->width + i->monitor->vt_size->h_margin));
                            if (i->virtual_desktop_scroll < new_pose) {
                                int max_pose = ((i->desktop_manager->virtual_desktops.size()) * (i->monitor->vt_size->width + i->monitor->vt_size->h_margin)) + i->monitor->vt_size->h_margin - i->monitor->get_width();
                                if (new_pose > max_pose) {
                                    new_pose = max_pose;
                                }
                                i->virtual_desktop_scroll = new_pose;
                                i->desktop_manager->calculate_vdesktops_pose();
                            }
                        }
                        i->desktop_manager->refresh_data();
                        i->selected_window = (*i->desktop_manager->active_desktop)->windows.size() > 0 ? 0 : -1;
                        i->render_n_detach();
                    });
                    t1.detach();
                }
            } else if (GET_WHEEL_DELTA_WPARAM(w_param) > 0) {
                VDesktopAPI::goto_previous_desktop();
                for (auto i : *this->command_centers) {
                    std::thread t1([=]() {
                        {  // repose if desktop out of screen or close to the edge
                            int new_index = (*i->desktop_manager->active_desktop)->index + 1;
                            int new_pose = -(i->monitor->get_width() / 2) + (new_index * (i->monitor->vt_size->width + i->monitor->vt_size->h_margin));
                            if (i->virtual_desktop_scroll > new_pose) {
                                if (new_pose < 0) {
                                    new_pose = 0;
                                }
                                i->virtual_desktop_scroll = new_pose;
                                i->desktop_manager->calculate_vdesktops_pose();
                            }
                        }
                        i->desktop_manager->refresh_data();
                        i->selected_window = (*i->desktop_manager->active_desktop)->windows.size() > 0 ? 0 : -1;
                        i->render_n_detach();
                    });
                    t1.detach();
                }
            }
            return;
        } else {
            int remaining_vt = (this->desktop_manager->virtual_desktops.size() - (this->monitor->get_width() / (this->monitor->vt_size->width + this->monitor->vt_size->h_margin)));
            if (remaining_vt > 0) {
                if (GET_WHEEL_DELTA_WPARAM(w_param) < 0 && this->virtual_desktop_scroll < int(remaining_vt * (this->monitor->vt_size->width + this->monitor->vt_size->h_margin)))
                    this->virtual_desktop_scroll += (this->monitor->vt_size->width + this->monitor->vt_size->h_margin) / 1.4;
                else if (GET_WHEEL_DELTA_WPARAM(w_param) > 0 && (this->virtual_desktop_scroll > -256))
                    this->virtual_desktop_scroll -= (this->monitor->vt_size->width + this->monitor->vt_size->h_margin) / 1.4;
                else
                    return;
            } else
                return;
            this->desktop_manager->calculate_vdesktops_pose();
        }
        this->render_n_detach();
    } else {
        if (this->selected_window == -1 && (int)(*this->desktop_manager->active_desktop)->windows.size() > 0) {
            this->selected_window = 0;
        } else if (GET_WHEEL_DELTA_WPARAM(w_param) < 0 && (int)(*this->desktop_manager->active_desktop)->windows.size() > this->selected_window + 1)
            this->selected_window++;
        else if (GET_WHEEL_DELTA_WPARAM(w_param) > 0 && this->selected_window > 0)
            this->selected_window--;
        else
            return;
        this->render_n_detach();
    }
    return;
}

void CommandCenter::on_mousemdown_event(WPARAM w_param, LPARAM l_param) {
    if (!(*this->desktop_manager->active_desktop)) return;
    if (this->mouse_on != -1) {
        PostMessage((*this->desktop_manager->active_desktop)->windows[this->mouse_on]->self_hwnd, WM_CLOSE, 0, 0);
    }
    return;
}

void CommandCenter::on_mouseldown_event(WPARAM w_param, LPARAM l_param) {
    std::lock_guard<std::mutex> lock(this->thumbnail_destroyer_lock);
    if (!(*this->desktop_manager->active_desktop)) return;
    if (this->mouse_on != -1) {
        this->mouse_down_on[0] = GET_X_LPARAM(l_param);
        this->mouse_down_on[1] = GET_Y_LPARAM(l_param);
        this->mouse_down = true;
        if (this->mouse_on < 1000000) {
            this->catched_thumbnail_ref_coord[0] = (*this->desktop_manager->active_desktop)->windows[mouse_on]->thumbnail_position.x - GET_X_LPARAM(l_param);
            this->catched_thumbnail_ref_coord[1] = (*this->desktop_manager->active_desktop)->windows[mouse_on]->thumbnail_position.y - GET_Y_LPARAM(l_param);
            (*this->desktop_manager->active_desktop)->windows[mouse_on]->unregister_thumbnail();  // update z-order of the thumbnail
            (*this->desktop_manager->active_desktop)->windows[mouse_on]->register_thumbnail();
        } else {
            auto temp = this->desktop_manager->virtual_desktops_index.find(mouse_on - 1000000);
            if (temp != this->desktop_manager->virtual_desktops_index.end()) {
                auto temp2 = this->desktop_manager->virtual_desktops.find(temp->second);
                if (temp2 != this->desktop_manager->virtual_desktops.end()) {
                    this->catched_thumbnail_ref_coord[0] = -(this->virtual_desktop_scroll + GET_X_LPARAM(l_param));
                    this->catched_thumbnail_ref_coord[1] = 0;
                }
            }
        }
        std::cout << "WM_LBUTTONDOWN\n";
    }
    return;
}

void CommandCenter::on_mouselup_event(WPARAM w_param, LPARAM l_param) {
    std::unique_lock<std::mutex> lock(this->thumbnail_destroyer_lock);
    if (!(*this->desktop_manager->active_desktop)) return;
    this->mouse_down_on[0] -= GET_X_LPARAM(l_param);
    this->mouse_down_on[1] -= GET_Y_LPARAM(l_param);
    if (this->mouse_on != -1) {
        if (this->mouse_down_on[0] < 64 && this->mouse_down_on[0] > -64) {
            if (this->mouse_down_on[1] < 64 && this->mouse_down_on[1] > -64) {
                if (this->mouse_on < 1000000) {
                    this->activate_this_window((*this->desktop_manager->active_desktop)->windows[this->mouse_on]->self_hwnd);
                    this->hide_window();
                } else {
                    auto temp = this->desktop_manager->virtual_desktops_index.find(this->mouse_on - 1000000);
                    if (temp != this->desktop_manager->virtual_desktops_index.end()) {
                        auto temp2 = this->desktop_manager->virtual_desktops.find(temp->second);
                        if (temp2 != this->desktop_manager->virtual_desktops.end()) {
                            if (temp2->first.compare((*this->desktop_manager->active_desktop)->guid) == 0) {
                                this->hide_window();
                                return;
                            }
                            std::thread t1([=]() {  // using thread because virtual desktop api does not allow winproc thread
                                if (temp2->first.compare(L"add_desktop") == 0) {
                                    VDesktopAPI::create_desktop();
                                } else {
                                    VDesktopAPI::go_to(temp2->second->i_vt);
                                }
                            });
                            t1.join();
                            lock.unlock();
                            for (auto i : *this->command_centers) {
                                i->desktop_manager->refresh_data();
                                i->selected_window = (*i->desktop_manager->active_desktop)->windows.size() > 0 ? 0 : -1;
                                i->render_n_detach();
                            }
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
            for (auto i : this->desktop_manager->virtual_desktops) {
                if (x > i.second->render_left) {
                    if (y > i.second->render_top) {
                        if (x < i.second->render_left + i.second->get_width()) {
                            if (y < i.second->render_top + i.second->get_height()) {
                                std::thread t1([=]() {  // using thread because virtual desktop api does not allow winproc thread
                                    IApplicationView* view;
                                    auto hr = VDesktopAPI::application_view_collection->GetViewForHwnd((*this->desktop_manager->active_desktop)->windows[mouse_on]->self_hwnd, &view);
                                    if (SUCCEEDED(hr)) {
                                        VDesktopAPI::desktop_manager_internal->MoveViewToDesktop(view, i.second->i_vt);
                                        this->mouse_down = false;
                                    }
                                });
                                t1.detach();
                            }
                        }
                    }
                }
            }
        }
        this->desktop_manager->calculate_thumbnails_pose();
        this->desktop_manager->update_thumbnails_pose();
        this->render_n_detach();
    }
    this->mouse_down = false;
    std::cout << "WM_LBUTTONUP\n";
    return;
}

void CommandCenter::on_mousemove_event(LPARAM l_param) {
    std::lock_guard<std::mutex> lock(this->thumbnail_destroyer_lock);
    if (!(*this->desktop_manager->active_desktop)) return;
    int x = GET_X_LPARAM(l_param);
    int y = GET_Y_LPARAM(l_param);
    if (this->mouse_down) {
        if ((int)(*this->desktop_manager->active_desktop)->windows.size() > this->mouse_on && this->mouse_on < 1000000) {
            (*this->desktop_manager->active_desktop)->windows[mouse_on]->repose_thumbnail(x + this->catched_thumbnail_ref_coord[0], y + this->catched_thumbnail_ref_coord[1]);
            this->render_n_detach();
        } else if (this->desktop_manager->virtual_desktops.size() >= (this->mouse_on - 1000000)) {
            this->virtual_desktop_scroll = -(x + this->catched_thumbnail_ref_coord[0]);
            this->desktop_manager->calculate_vdesktops_pose();
            this->render_n_detach();
        }
        return;
    }
    for (auto i : (*this->desktop_manager->active_desktop)->windows) {
        if (x > i->thumbnail_position.x) {
            if (y > i->thumbnail_position.y) {
                if (x < i->thumbnail_position.x + i->thumbnail_position.width) {
                    if (y < i->thumbnail_position.y + i->thumbnail_position.height) {
                        if (this->mouse_on != i->order) {
                            this->mouse_on = i->order;
                            this->render_n_detach();
                        }
                        return;
                    }
                }
            }
        }
    }
    for (auto i : this->desktop_manager->virtual_desktops) {
        if (x > i.second->render_left) {
            if (y > i.second->render_top) {
                if (x < i.second->render_left + i.second->get_width()) {
                    if (y < i.second->render_top + i.second->get_height()) {
                        if (this->mouse_on != (i.second->index + 1000000)) {
                            this->mouse_on = (i.second->index + 1000000);
                            this->render_n_detach();
                        }
                        return;
                    }
                }
            }
        }
    }
    if (this->mouse_on != -1) {
        this->mouse_on = -1;
        // InvalidateRect(this->hwnd, NULL, FALSE);
        this->render_n_detach();
    }
    return;
}
