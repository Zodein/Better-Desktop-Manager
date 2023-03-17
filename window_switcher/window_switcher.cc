
#include "window_switcher.h"

#include <windowsx.h>

#include <thread>

#include "../monitor_resolver/monitor_resolver.h"
#include "../thumbnail/thumbnail.h"
#include "gdiplus.h"
#include "iostream"
#include "wingdi.h"
#include "winuser.h"

struct ComException {
    HRESULT result;
    ComException(HRESULT const value) : result(value) {}
};
void HR(HRESULT const result) {
    if (S_OK != result) {
        throw ComException(result);
    }
}
WNDCLASSEX WindowSwitcher::wc;
D2D1_COLOR_F const WindowSwitcher::background_color = D2D1::ColorF(0.2, 0.2, 0.2, 0.5f);
D2D1_COLOR_F const WindowSwitcher::on_mouse_color = D2D1::ColorF(0.4, 0.4, 0.4, 1.0f);
D2D1_COLOR_F const WindowSwitcher::selected_color = D2D1::ColorF(0.8, 0.8, 0.8, 1.0f);
D2D1_COLOR_F const WindowSwitcher::title_bg_color = D2D1::ColorF(0.2, 0.2, 0.2, 0.8f);

WindowSwitcher::WindowSwitcher(Monitor* monitor, std::vector<WindowSwitcher*>* window_switchers) {
    this->window_switchers = window_switchers;
    this->monitor = monitor;
    this->thumbnail_manager = new ThumbnailManager(this, this->monitor->get_width() - 128, this->monitor->get_height() - 128);
}

LRESULT CALLBACK WindowSwitcher::window_proc_static(HWND handle_window, UINT message, WPARAM wParam, LPARAM lParam) {
    WindowSwitcher* window_switcher = reinterpret_cast<WindowSwitcher*>(GetWindowLongPtr(handle_window, GWLP_USERDATA));
    if (window_switcher) return window_switcher->window_proc(handle_window, message, wParam, lParam);
    return DefWindowProc(handle_window, message, wParam, lParam);
}

LRESULT CALLBACK WindowSwitcher::window_proc(HWND handle_window, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
        case WM_MOUSEWHEEL: {
            if (GET_WHEEL_DELTA_WPARAM(wParam) < 0 && this->selected_window + 1 < this->thumbnail_manager->thumbnails.size())
                this->selected_window++;
            else if (GET_WHEEL_DELTA_WPARAM(wParam) > 0 && this->selected_window > 0)
                this->selected_window--;
            else
                break;
            InvalidateRect(this->hwnd, NULL, FALSE);
            break;
        }
        case WM_LBUTTONDOWN: {
            if (this->mouse_on != -1) {
                this->activate_window(this->thumbnail_manager->thumbnails[this->mouse_on]->self_hwnd);
                this->hide_window();
                std::cout << "clicked\n";
            }
            break;
        }
        case WM_MBUTTONDOWN: {
            if (this->mouse_on != -1) {
                SendMessage(this->thumbnail_manager->thumbnails[this->mouse_on]->self_hwnd, WM_CLOSE, 0, 0);
                std::cout << "clicked\n";
            }
            break;
        }
        case WM_HOTKEY: {
            if (!IsWindowVisible(this->hwnd)) {
                this->show_window();
                this->selected_window = this->thumbnail_manager->thumbnails.size() > 1;
                std::thread t1([=]() {
                    int last_checked = 0;
                    while (GetAsyncKeyState(0x42)) {
                        std::this_thread::sleep_for(std::chrono::milliseconds(10));
                        if (!IsWindowVisible(this->hwnd)) return;
                        if (last_checked < GetTickCount()) {
                            this->thumbnail_manager->update_thumbnails_if_needed();
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
                                            this->activate_window(this->thumbnail_manager->thumbnails[this->selected_window]->self_hwnd);
                                        }
                                    }
                                }
                            }
                        }
                        this->hide_window();
                    }
                    return;
                });
                t1.detach();
                std::cout << "HOTKEY\n";
            }
            break;
        }
        case WM_PAINT: {
            dc->BeginDraw();
            dc->Clear();
            dc->FillRectangle(D2D1::RectF(0, 0, monitor->get_width(), monitor->get_height()), this->background_brush.Get());
            if (this->mouse_on >= 0) {
                int border_width = 4;
                rect.left = this->thumbnail_manager->thumbnails[this->mouse_on]->thumbnail_position.x - border_width;
                rect.top = this->thumbnail_manager->thumbnails[this->mouse_on]->thumbnail_position.y - border_width - 32;
                rect.right = this->thumbnail_manager->thumbnails[this->mouse_on]->thumbnail_position.x + this->thumbnail_manager->thumbnails[this->mouse_on]->thumbnail_position.width + border_width;
                rect.bottom = this->thumbnail_manager->thumbnails[this->mouse_on]->thumbnail_position.y + this->thumbnail_manager->thumbnails[this->mouse_on]->thumbnail_position.height + border_width;
                dc->FillRectangle(D2D1::RectF(rect.left, rect.top, rect.right, rect.bottom), this->on_mouse_brush.Get());
            }
            if (this->selected_window >= 0) {
                {
                    int border_width = 2;
                    rect.left = this->thumbnail_manager->thumbnails[this->selected_window]->thumbnail_position.x - border_width;
                    rect.top = this->thumbnail_manager->thumbnails[this->selected_window]->thumbnail_position.y - border_width - 32;
                    rect.right = this->thumbnail_manager->thumbnails[this->selected_window]->thumbnail_position.x + this->thumbnail_manager->thumbnails[this->selected_window]->thumbnail_position.width + border_width;
                    rect.bottom = this->thumbnail_manager->thumbnails[this->selected_window]->thumbnail_position.y + this->thumbnail_manager->thumbnails[this->selected_window]->thumbnail_position.height + border_width;
                    dc->FillRectangle(D2D1::RectF(rect.left, rect.top, rect.right, rect.bottom), this->selected_brush.Get());
                }
                {
                    for (auto i : this->thumbnail_manager->thumbnails) {
                        int len = GetWindowTextLength(i->self_hwnd) + 1;
                        std::wstring s;
                        s.reserve(len);
                        GetWindowText(i->self_hwnd, LPWSTR(s.c_str()), len);
                        rect.left = i->thumbnail_position.x;
                        rect.top = i->thumbnail_position.y - 32;
                        rect.right = i->thumbnail_position.x + i->thumbnail_position.width;
                        rect.bottom = i->thumbnail_position.y;
                        dc->FillRectangle(D2D1::RectF(rect.left, rect.top, rect.right, rect.bottom), this->title_bg_brush.Get());
                        dc->DrawText(s.c_str(), len, writeTextFormat, D2D1::RectF(rect.left, rect.top, rect.right, rect.bottom), this->selected_brush.Get());
                    }
                }
            }

            HR(dc->EndDraw());
            HR(swapChain->Present(1, 0));
            ValidateRect(hwnd, NULL);
            std::cout << "paint\n";
            break;
        }
        case WM_ERASEBKGND: {
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
            this->on_mouse_message(lParam);
            // std::cout << "mousemove on parent\n";
            break;
        }
        default:
            return DefWindowProc(handle_window, message, wParam, lParam);
    }
    return 0;
}

int WindowSwitcher::create_window() {
    this->hwnd = CreateWindowEx(WS_EX_TOPMOST | WS_EX_NOREDIRECTIONBITMAP, WindowSwitcher::wc.lpszClassName, L"Better Desktop Manager", WS_POPUP, this->monitor->get_x(), this->monitor->get_y(), this->monitor->get_width(), this->monitor->get_height(), NULL, NULL, NULL, NULL);

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
    // Create the Direct2D device that links back to the Direct3D device
    HR(d2Factory->CreateDevice(dxgiDevice.Get(), d2Device.GetAddressOf()));
    // Create the Direct2D device context that is the actual render target
    // and exposes drawing commands
    HR(d2Device->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, dc.GetAddressOf()));
    // Retrieve the swap chain's back buffer
    HR(swapChain->GetBuffer(0, __uuidof(surface), reinterpret_cast<void**>(surface.GetAddressOf())));
    // Create a Direct2D bitmap that points to the swap chain surface
    D2D1_BITMAP_PROPERTIES1 properties = {};
    properties.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
    properties.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
    properties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW;
    ComPtr<ID2D1Bitmap1> bitmap;
    HR(dc->CreateBitmapFromDxgiSurface(surface.Get(), properties, bitmap.GetAddressOf()));
    // Point the device context to the bitmap for rendering
    dc->SetTarget(bitmap.Get());
    // Draw something
    // dc->BeginDraw();
    // dc->Clear();
    // ComPtr<ID2D1SolidColorBrush> brush;
    // D2D1_COLOR_F const brushColor = D2D1::ColorF(0.1f, 0.1f, 0.1f, 0.75f);
    // HR(dc->CreateSolidColorBrush(brushColor, brush.GetAddressOf()));
    // D2D1_RECT_F const recta = D2D1::RectF(0, 0, monitor->get_width(), monitor->get_height());
    // dc->FillRectangle(recta, brush.Get());
    // HR(dc->EndDraw());
    // // Make the swap chain available to the composition engine
    // HR(swapChain->Present(1, 0));
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
    DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(writeFactory), reinterpret_cast<IUnknown**>(&writeFactory));
    writeFactory->CreateTextFormat(L"Courier New", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 16.0f, L"en-us", &writeTextFormat);
    writeTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    writeTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

    SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)this);

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
    this->thumbnail_manager->update_thumbnails_if_needed();
    SetWindowPos(this->hwnd, HWND_TOPMOST, this->monitor->get_x(), this->monitor->get_y(), this->monitor->get_width(), this->monitor->get_height(), SWP_SHOWWINDOW);
    return;
}

void WindowSwitcher::hide_window() {
    ShowWindow(this->hwnd, 0);
    this->thumbnail_manager->destroy_all_thumbnails();
    return;
}

void WindowSwitcher::resize_window() {
    SetWindowPos(this->hwnd, HWND_TOPMOST, this->monitor->get_x(), this->monitor->get_y(), this->monitor->get_width(), this->monitor->get_height(), SWP_SHOWWINDOW);
    return;
}

void WindowSwitcher::on_mouse_message(LPARAM lParam) {
    int x = GET_X_LPARAM(lParam);
    int y = GET_Y_LPARAM(lParam);
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
    if (this->mouse_on != -1) {
        this->mouse_on = -1;
        InvalidateRect(this->hwnd, NULL, FALSE);
    }
    return;
}