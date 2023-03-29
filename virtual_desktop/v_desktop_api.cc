#include "v_desktop_api.h"

#include <winerror.h>

#include "v_desktop.h"

//
// ONLY WORKS BETWEEN 21313 >= BUILD < 22449:
//
IServiceProvider *VDesktopAPI::service_provider = nullptr;
IVirtualDesktopManagerInternal2 *VDesktopAPI::desktop_manager_internal = nullptr;
IApplicationViewCollection *VDesktopAPI::application_view_collection = nullptr;

bool VDesktopAPI::init() {
    ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    return VDesktopAPI::init_service() && VDesktopAPI::init_desktop_manager_internal() && VDesktopAPI::init_application_view_collection();
}

bool VDesktopAPI::init_service() {
    if (VDesktopAPI::service_provider == nullptr) {
        HRESULT hr = ::CoCreateInstance(CLSID_ImmersiveShell, NULL, CLSCTX_LOCAL_SERVER, __uuidof(IServiceProvider), (PVOID *)&VDesktopAPI::service_provider);
        if (SUCCEEDED(hr)) {
            return true;
        } else {
            delete VDesktopAPI::service_provider;
            VDesktopAPI::service_provider = nullptr;
            return false;
        }
    }
    return true;
}

bool VDesktopAPI::init_desktop_manager_internal() {
    if (VDesktopAPI::service_provider == nullptr) return false;
    if (VDesktopAPI::desktop_manager_internal == nullptr) {
        HRESULT hr = VDesktopAPI::service_provider->QueryService(CLSID_IVirtualDesktopManagerInternal, &VDesktopAPI::desktop_manager_internal);
        if (SUCCEEDED(hr)) {
            return true;
        } else {
            delete VDesktopAPI::desktop_manager_internal;
            VDesktopAPI::desktop_manager_internal = nullptr;
            return false;
        }
    }
    return true;
}

bool VDesktopAPI::init_application_view_collection() {
    if (VDesktopAPI::service_provider == nullptr) return false;
    if (VDesktopAPI::application_view_collection == nullptr) {
        HRESULT hr = VDesktopAPI::service_provider->QueryService(IID_IApplicationViewCollection, &VDesktopAPI::application_view_collection);
        if (SUCCEEDED(hr)) {
            return true;
        } else {
            delete VDesktopAPI::application_view_collection;
            VDesktopAPI::application_view_collection = nullptr;
            return false;
        }
    }
    return true;
}

int VDesktopAPI::get_desktop_count() {
    UINT temp = 0;
    VDesktopAPI::desktop_manager_internal->GetCount(0, &temp);
    return temp;
}

void VDesktopAPI::go_to(IVirtualDesktop *i_vt) { VDesktopAPI::desktop_manager_internal->SwitchDesktop(0, i_vt); }

std::wstring VDesktopAPI::get_desktop_guid_as_string(IVirtualDesktop *i_vt) {
    GUID guid;
    WCHAR id[256];
    i_vt->GetID(&guid);
    auto hr = StringFromGUID2(guid, id, _countof(id));
    return SUCCEEDED(hr) ? std::wstring(id) : L"";
}

IVirtualDesktop *VDesktopAPI::get_current_desktop() {
    IVirtualDesktop *current_desktop;
    auto hr = VDesktopAPI::desktop_manager_internal->GetCurrentDesktop(0, &current_desktop);
    return current_desktop;
}

std::wstring VDesktopAPI::get_current_desktop_guid_as_string() {
    GUID guid;
    WCHAR id[256];
    IVirtualDesktop *current_desktop;
    auto hr = VDesktopAPI::desktop_manager_internal->GetCurrentDesktop(0, &current_desktop);
    if (SUCCEEDED(hr)) {
        current_desktop->GetID(&guid);
        hr = StringFromGUID2(guid, id, _countof(id));
    }
    return std::wstring(id);
}

std::wstring VDesktopAPI::guid_to_string(GUID guid) {
    WCHAR id[256];
    auto hr = StringFromGUID2(guid, id, _countof(id));
    return SUCCEEDED(hr) ? std::wstring(id) : L"";
}

bool VDesktopAPI::goto_previous_desktop() {
    IObjectArray *desktops;
    IVirtualDesktop *current_desktop;
    IVirtualDesktop *desktop;
    GUID current_guid;
    GUID guid;
    UINT size = 0;
    if (SUCCEEDED(VDesktopAPI::desktop_manager_internal->GetDesktops(0, &desktops)) && SUCCEEDED(VDesktopAPI::desktop_manager_internal->GetCurrentDesktop(0, &current_desktop))) {
        if (SUCCEEDED(desktops->GetCount(&size)) && SUCCEEDED(current_desktop->GetID(&current_guid))) {
            for (UINT i = 0; i < size; i++) {
                if (SUCCEEDED(desktops->GetAt(i, IID_IVirtualDesktop, (void **)&desktop)) && SUCCEEDED(desktop->GetID(&guid)) && guid.Data1 == current_guid.Data1) {
                    if (SUCCEEDED(desktops->GetAt(i - 1, IID_IVirtualDesktop, (void **)&desktop)) && SUCCEEDED(VDesktopAPI::desktop_manager_internal->SwitchDesktop(0, desktop))) return true;
                    return false;
                }
            }
        }
    }
    return false;
}

bool VDesktopAPI::goto_next_desktop() {
    IObjectArray *desktops;
    IVirtualDesktop *current_desktop;
    IVirtualDesktop *desktop;
    GUID current_guid;
    GUID guid;
    UINT size = 0;
    if (SUCCEEDED(VDesktopAPI::desktop_manager_internal->GetDesktops(0, &desktops)) && SUCCEEDED(VDesktopAPI::desktop_manager_internal->GetCurrentDesktop(0, &current_desktop))) {
        if (SUCCEEDED(desktops->GetCount(&size)) && SUCCEEDED(current_desktop->GetID(&current_guid))) {
            for (UINT i = 0; i < size; i++) {
                if (SUCCEEDED(desktops->GetAt(i, IID_IVirtualDesktop, (void **)&desktop)) && SUCCEEDED(desktop->GetID(&guid)) && guid.Data1 == current_guid.Data1) {
                    if (SUCCEEDED(desktops->GetAt(i + 1, IID_IVirtualDesktop, (void **)&desktop)) && SUCCEEDED(VDesktopAPI::desktop_manager_internal->SwitchDesktop(0, desktop))) return true;
                    return false;
                }
            }
        }
    }
    return false;
}

bool VDesktopAPI::goto_this_desktop(int to_this) {
    IObjectArray *desktops;
    IVirtualDesktop *current_desktop;
    IVirtualDesktop *desktop;
    GUID current_guid;
    GUID guid;
    UINT size = 0;
    if (SUCCEEDED(VDesktopAPI::desktop_manager_internal->GetDesktops(0, &desktops))) {
        if (SUCCEEDED(desktops->GetCount(&size))) {
            if (size < to_this && SUCCEEDED(desktops->GetAt(to_this, IID_IVirtualDesktop, (void **)&desktop)) && SUCCEEDED(VDesktopAPI::desktop_manager_internal->SwitchDesktop(0, desktop))) return true;
            return false;
        }
    }
    return false;
}

void VDesktopAPI::create_desktop() {
    IVirtualDesktop **temp = (IVirtualDesktop **)new int;
    VDesktopAPI::desktop_manager_internal->CreateDesktopW(0, temp);
    if (temp) (*temp)->Release();
}

void VDesktopAPI::remove_desktop(IVirtualDesktop *destroyDesktop, IVirtualDesktop *fall_back) {
    // IVirtualDesktop *temp = (IVirtualDesktop *)new int;
    VDesktopAPI::desktop_manager_internal->RemoveDesktop(destroyDesktop, fall_back);
}

// void IVirtualDesktopManagerInternal2::create_desktop(){
//         IVirtualDesktop **temp = (IVirtualDesktop **)new int;
//         VDesktopAPI::desktop_manager_internal->CreateDesktopW(0, temp);
// }