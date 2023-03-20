#include "virtual_desktop.h"

#include <winerror.h>

//
// ONLY WORKS BETWEEN 21313 >= BUILD < 22449:
//
IServiceProvider *VirtualDesktopManager::service_provider = nullptr;
IVirtualDesktopManagerInternal *VirtualDesktopManager::desktop_manager_internal = nullptr;
IApplicationViewCollection *VirtualDesktopManager::application_view_collection = nullptr;

bool VirtualDesktopManager::init() {
    ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
    return VirtualDesktopManager::init_service() && VirtualDesktopManager::init_desktop_manager_internal() && VirtualDesktopManager::init_application_view_collection();
}

bool VirtualDesktopManager::init_service() {
    if (VirtualDesktopManager::service_provider == nullptr) {
        HRESULT hr = ::CoCreateInstance(CLSID_ImmersiveShell, NULL, CLSCTX_LOCAL_SERVER, __uuidof(IServiceProvider), (PVOID *)&VirtualDesktopManager::service_provider);
        if (SUCCEEDED(hr)) {
            return true;
        } else {
            delete VirtualDesktopManager::service_provider;
            VirtualDesktopManager::service_provider = nullptr;
            return false;
        }
    }
    return true;
}

bool VirtualDesktopManager::init_desktop_manager_internal() {
    if (VirtualDesktopManager::service_provider == nullptr) return false;
    if (VirtualDesktopManager::desktop_manager_internal == nullptr) {
        HRESULT hr = VirtualDesktopManager::service_provider->QueryService(CLSID_IVirtualDesktopManagerInternal, &VirtualDesktopManager::desktop_manager_internal);
        if (SUCCEEDED(hr)) {
            return true;
        } else {
            delete VirtualDesktopManager::desktop_manager_internal;
            VirtualDesktopManager::desktop_manager_internal = nullptr;
            return false;
        }
    }
    return true;
}

bool VirtualDesktopManager::init_application_view_collection() {
    if (VirtualDesktopManager::service_provider == nullptr) return false;
    if (VirtualDesktopManager::application_view_collection == nullptr) {
        HRESULT hr = VirtualDesktopManager::service_provider->QueryService(IID_IApplicationViewCollection, &VirtualDesktopManager::application_view_collection);
        if (SUCCEEDED(hr)) {
            return true;
        } else {
            delete VirtualDesktopManager::application_view_collection;
            VirtualDesktopManager::application_view_collection = nullptr;
            return false;
        }
    }
    return true;
}

int VirtualDesktopManager::get_desktop_count() {
    UINT temp = 0;
    VirtualDesktopManager::desktop_manager_internal->GetCount(0, &temp);
    return temp;
}

void VirtualDesktopManager::go_to(IVirtualDesktop *i_vt) { VirtualDesktopManager::desktop_manager_internal->SwitchDesktop(0, i_vt); }

std::wstring VirtualDesktopManager::get_desktop_guid_as_string(IVirtualDesktop *i_vt) {
    GUID guid;
    WCHAR id[256];
    i_vt->GetID(&guid);
    auto hr = StringFromGUID2(guid, id, _countof(id));
    return SUCCEEDED(hr) ? std::wstring(id) : L"";
}

IVirtualDesktop *VirtualDesktopManager::get_current_desktop() {
    IVirtualDesktop *current_desktop;
    auto hr = VirtualDesktopManager::desktop_manager_internal->GetCurrentDesktop(0, &current_desktop);
    return current_desktop;
}

std::wstring VirtualDesktopManager::get_current_desktop_guid_as_string() {
    GUID guid;
    WCHAR id[256];
    IVirtualDesktop *current_desktop;
    auto hr = VirtualDesktopManager::desktop_manager_internal->GetCurrentDesktop(0, &current_desktop);
    if (SUCCEEDED(hr)) {
        current_desktop->GetID(&guid);
        hr = StringFromGUID2(guid, id, _countof(id));
    }
    return std::wstring(id);
}

std::wstring VirtualDesktopManager::guid_to_string(GUID guid) {
    WCHAR id[256];
    auto hr = StringFromGUID2(guid, id, _countof(id));
    return SUCCEEDED(hr) ? std::wstring(id) : L"";
}