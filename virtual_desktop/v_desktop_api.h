#pragma once

#include <ObjectArray.h>
#include <hstring.h>
#include <inspectable.h>
#include <objbase.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "../thumbnail/thumbnail.h"

const CLSID CLSID_ImmersiveShell = {0xC2F03A33, 0x21F5, 0x47FA, 0xB4, 0xBB, 0x15, 0x63, 0x62, 0xA2, 0xF2, 0x39};
const CLSID CLSID_IVirtualDesktopManagerInternal = {0xc5e0cdca, 0x7b6e, 0x41b2, 0x9f, 0xc4, 0xd9, 0x39, 0x75, 0xcc, 0x46, 0x7b};
const CLSID CLSID_VirtualDesktopPinnedApps = {0xb5a399e7, 0x1c87, 0x46b8, 0x88, 0xe9, 0xfc, 0x57, 0x47, 0xb1, 0x71, 0xbd};
const CLSID IID_IApplicationView = {0x372e1d3b, 0x38d3, 0x42e4, 0xa1, 0x5b, 0x8a, 0xb2, 0xb1, 0x78, 0xf5, 0x13};
const CLSID IID_IApplicationViewCollection = {0x1841c6d7, 0x4f9d, 0x42c0, 0xaf, 0x41, 0x87, 0x47, 0x53, 0x8f, 0x10, 0xe5};
const IID IID_IVirtualDesktop = {0x536d3495, 0xb208, 0x4cc9, 0xae, 0x26, 0xde, 0x81, 0x11, 0x27, 0x5b, 0xf8};

// const CLSID CLSID_ImmersiveShell = {0xC2F03A33, 0x21F5, 0x47FA, 0xB4, 0xBB, 0x15, 0x63, 0x62, 0xA2, 0xF2, 0x39};
// const CLSID CLSID_VirtualDesktopAPI_Unknown = {0xC5E0CDCA, 0x7B6E, 0x41B2, 0x9F, 0xC4, 0xD9, 0x39, 0x75, 0xCC, 0x46, 0x7B};
// const IID IID_IVirtualDesktopManagerInternal = {0xEF9F1A6C, 0xD3CC, 0x4358, 0xB7, 0x12, 0xF8, 0x4B, 0x63, 0x5B, 0xEB, 0xE7};
// const CLSID CLSID_IVirtualNotificationService = {0xA501FDEC, 0x4A09, 0x464C, 0xAE, 0x4E, 0x1B, 0x9C, 0x21, 0xB8, 0x49, 0x18};

typedef UINT IAsyncCallback;
typedef UINT IImmersiveMonitor;
typedef UINT APPLICATION_VIEW_COMPATIBILITY_POLICY;
typedef UINT IShellPositionerPriority;
typedef UINT IApplicationViewOperation;
typedef UINT APPLICATION_VIEW_CLOAK_TYPE;
typedef UINT IApplicationViewPosition;
typedef UINT IImmersiveApplication;
typedef UINT IApplicationViewChangeListener;
typedef UINT AdjacentDesktop;

MIDL_INTERFACE("372E1D3B-38D3-42E4-A15B-8AB2B178F513")
IApplicationView : public IUnknown {
   public:
    // IInspectable methods=0;
    virtual HRESULT STDMETHODCALLTYPE GetIids(ULONG * _ULONG, GUID * *_GUID) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetRuntimeClassName(HSTRING * _HSTRING) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetTrustLevel(TrustLevel * _TrustLevel) = 0;
    // IApplicationView methods=0;
    virtual HRESULT STDMETHODCALLTYPE SetFocus() = 0;
    virtual HRESULT STDMETHODCALLTYPE SwitchTo() = 0;
    virtual HRESULT STDMETHODCALLTYPE TryInvokeBack(IAsyncCallback * _IAsyncCallback) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetThumbnailWindow(HWND * hwnd) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetMonitor(IImmersiveMonitor * *_IImmersiveMonitor) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetVisibility(UINT * pVisible) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetCloak(APPLICATION_VIEW_CLOAK_TYPE, UINT) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetPosition(REFIID, LPVOID * _LPVOID) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetPosition(IApplicationViewPosition * _IApplicationViewPosition) = 0;
    virtual HRESULT STDMETHODCALLTYPE InsertAfterWindow(HWND) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetExtendedFramePosition(RECT * _RECT) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetAppUserModelId(PWSTR * pId) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetAppUserModelId(LPCWSTR) = 0;
    virtual HRESULT STDMETHODCALLTYPE IsEqualByAppUserModelId(LPCWSTR, UINT * _UINT) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetViewState(UINT * _UINT) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetViewState(UINT) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetNeediness(UINT * _UINT) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetLastActivationTimestamp(ULONGLONG * pGuid) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetLastActivationTimestamp(ULONGLONG) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetVirtualDesktopId(GUID * pGuid) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetVirtualDesktopId(REFGUID) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetShowInSwitchers(UINT * pShown) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetShowInSwitchers(UINT) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetScaleFactor(UINT * _UINT) = 0;
    virtual HRESULT STDMETHODCALLTYPE CanReceiveInput(BOOL * _BOOL) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetCompatibilityPolicyType(APPLICATION_VIEW_COMPATIBILITY_POLICY * _APPLICATION_VIEW_COMPATIBILITY_POLICY) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetCompatibilityPolicyType(APPLICATION_VIEW_COMPATIBILITY_POLICY) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetSizeConstraints(IImmersiveMonitor * _IImmersiveMonitor, SIZE * _SIZE, SIZE * _SIZE2) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetSizeConstraintsForDpi(UINT, SIZE * _SIZE, SIZE * _SIZE2) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetSizeConstraintsForDpi(UINT * _UINT, SIZE * _SIZE, SIZE * _SIZE2) = 0;
    virtual HRESULT STDMETHODCALLTYPE OnMinSizePreferencesUpdated(HWND) = 0;
    virtual HRESULT STDMETHODCALLTYPE ApplyOperation(IApplicationViewOperation * _IApplicationViewOperation) = 0;
    virtual HRESULT STDMETHODCALLTYPE IsTray(BOOL * _BOOL) = 0;
    virtual HRESULT STDMETHODCALLTYPE IsInHighZOrderBand(BOOL * _BOOL) = 0;
    virtual HRESULT STDMETHODCALLTYPE IsSplashScreenPresented(BOOL * _BOOL) = 0;
    virtual HRESULT STDMETHODCALLTYPE Flash() = 0;
    virtual HRESULT STDMETHODCALLTYPE GetRootSwitchableOwner(IApplicationView * *_IApplicationView) = 0;
    virtual HRESULT STDMETHODCALLTYPE EnumerateOwnershipTree(IObjectArray * *_IObjectArray) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetEnterpriseId(PWSTR * _PWSTR) = 0;
    virtual HRESULT STDMETHODCALLTYPE IsMirrored(BOOL * _BOOL) = 0;
    virtual HRESULT STDMETHODCALLTYPE Unknown1(UINT * _UINT) = 0;
    virtual HRESULT STDMETHODCALLTYPE Unknown2(UINT * _UINT) = 0;
    virtual HRESULT STDMETHODCALLTYPE Unknown3(UINT * _UINT) = 0;
    virtual HRESULT STDMETHODCALLTYPE Unknown4(UINT) = 0;
    virtual HRESULT STDMETHODCALLTYPE Unknown5(UINT * _UINT) = 0;
    virtual HRESULT STDMETHODCALLTYPE Unknown6(UINT) = 0;
    virtual HRESULT STDMETHODCALLTYPE Unknown7() = 0;
    virtual HRESULT STDMETHODCALLTYPE Unknown8(UINT * _UINT) = 0;
    virtual HRESULT STDMETHODCALLTYPE Unknown9(UINT) = 0;
    virtual HRESULT STDMETHODCALLTYPE Unknown10(UINT, UINT) = 0;
    virtual HRESULT STDMETHODCALLTYPE Unknown11(UINT) = 0;
    virtual HRESULT STDMETHODCALLTYPE Unknown12(SIZE * _SIZE) = 0;
};

// EXTERN_C const IID IID_IVirtualDesktop;

// FF72FFDD-BE7E-43FC-9C03-AD81681E88E4
MIDL_INTERFACE("536D3495-B208-4CC9-AE26-DE8111275BF8")
IVirtualDesktop : public IUnknown {
   public:
    virtual HRESULT STDMETHODCALLTYPE IsViewVisible(IApplicationView * _IApplicationView, UINT * _UINT) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetID(GUID * pGuid) = 0;
    virtual HRESULT STDMETHODCALLTYPE Unknown(HWND * pW) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetName(HSTRING * pName) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetWallpaperPath(HSTRING * pPath) = 0;
};

// EXTERN_C const IID IID_IVirtualDesktop2;
MIDL_INTERFACE("31EBDE3F-6EC3-4CBD-B9FB-0EF6D09B41F4")
IVirtualDesktop2 : public IUnknown {
   public:
    virtual HRESULT STDMETHODCALLTYPE IsViewVisible(IApplicationView * _IApplicationView, UINT * _UINT) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetID(GUID * pGuid) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetName(HSTRING * pName) = 0;
};

// EXTERN_C const IID IID_IVirtualDesktopManagerInternal;

MIDL_INTERFACE("B2F925B9-5A0F-4D2E-9F4D-2B1507593C10")
IVirtualDesktopManagerInternal : public IUnknown {
   public:
    virtual HRESULT STDMETHODCALLTYPE GetCount(HWND hwnd, UINT * pCount) = 0;
    virtual HRESULT STDMETHODCALLTYPE MoveViewToDesktop(IApplicationView * _IApplicationView, IVirtualDesktop * _IVirtualDesktop) = 0;
    virtual HRESULT STDMETHODCALLTYPE CanViewMoveDesktops(IApplicationView * _IApplicationView, UINT * _UINT) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetCurrentDesktop(HWND hwnd, IVirtualDesktop * *pDesktop) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetDesktops(HWND hwnd, IObjectArray * *array) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetAdjacentDesktop(IVirtualDesktop * _IVirtualDesktop, AdjacentDesktop _AdjacentDesktop, IVirtualDesktop * *_VirtualDesktop) = 0;
    virtual HRESULT STDMETHODCALLTYPE SwitchDesktop(HWND _HWND, IVirtualDesktop * _IVirtualDesktop) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateDesktopW(HWND hwnd, IVirtualDesktop * *pDesktop) = 0;
    virtual HRESULT STDMETHODCALLTYPE MoveDesktop(IVirtualDesktop * _IVirtualDesktop, HWND _HWND, INT _INT) = 0;
    virtual HRESULT STDMETHODCALLTYPE RemoveDesktop(IVirtualDesktop * destroyDesktop, IVirtualDesktop * fallbackDesktop) = 0;
    virtual HRESULT STDMETHODCALLTYPE FindDesktop(GUID * pGuid, IVirtualDesktop * *pDesktop) = 0;
    virtual HRESULT STDMETHODCALLTYPE Unknown(IVirtualDesktop * _IVirtualDesktop, IObjectArray * *_ObjectArray, IObjectArray * *_ObjectArray2) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetName(IVirtualDesktop * pDesktop, HSTRING name) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetWallpaper(IVirtualDesktop * pDesktop, HSTRING path) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetWallpaperForAllDesktops(HSTRING path) = 0;
    virtual HRESULT STDMETHODCALLTYPE CopyDesktopState(IApplicationView * pView0, IApplicationView * pView02) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetDesktopPerMonitor(BOOL * state) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetDesktopPerMonitor(BOOL state) = 0;
};

// EXTERN_C const IID IID_IVirtualDesktopManager;
// a5cd92ff-29be-454c-8d04-d82879fb3f1b
MIDL_INTERFACE("A5CD92FF-29BE-454C-8D04-D82879FB3F1B")
IVirtualDesktopManager : public IUnknown {
   public:
    virtual HRESULT STDMETHODCALLTYPE IsWindowOnCurrentVirtualDesktop(HWND hwnd, BOOL * isOnCurrent) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetWindowDesktopId(HWND _HWND, GUID * _GUID) = 0;
    virtual HRESULT STDMETHODCALLTYPE MoveWindowToDesktop(HWND _HWND, REFGUID _REFGUID) = 0;
};

// EXTERN_C const IID IID_IVirtualDesktopPinnedApps;
MIDL_INTERFACE("4CE81583-1E4C-4632-A621-07A53543148F")
IVirtualDesktopPinnedApps : public IUnknown {
   public:
    virtual HRESULT STDMETHODCALLTYPE IsAppIdPinned(LPCWSTR appId, BOOL * isPinned) = 0;
    virtual HRESULT STDMETHODCALLTYPE PinAppID(LPCWSTR _LPCWSTR) = 0;
    virtual HRESULT STDMETHODCALLTYPE UnpinAppID(LPCWSTR _LPCWSTR) = 0;
    virtual HRESULT STDMETHODCALLTYPE IsViewPinned(IApplicationView * pView, BOOL * isPinned) = 0;
    virtual HRESULT STDMETHODCALLTYPE PinView(IApplicationView * _IApplicationView) = 0;
    virtual HRESULT STDMETHODCALLTYPE UnpinView(IApplicationView * _IApplicationView) = 0;
};

// EXTERN_C const IID IID_IApplicationViewCollection;
MIDL_INTERFACE("1841C6D7-4F9D-42C0-AF41-8747538F10E5")
IApplicationViewCollection : public IUnknown {
   public:
    virtual HRESULT STDMETHODCALLTYPE GetViews(IObjectArray * *_IObjectArray) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetViewsByZOrder(IObjectArray * *array) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetViewsByAppUserModelId(LPCWSTR _LPCWSTR, IObjectArray * *_IObjectArray) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetViewForHwnd(HWND hwnd, IApplicationView * *pView) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetViewForApplication(IImmersiveApplication * _IImmersiveApplication, IApplicationView * *_IApplicationView) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetViewForAppUserModelId(LPCWSTR _LPCWSTR, IApplicationView * *_IApplicationView) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetViewInFocus(IApplicationView * *view) = 0;
    virtual HRESULT STDMETHODCALLTYPE Unknown1(IApplicationView * *_IApplicationView) = 0;
    virtual HRESULT STDMETHODCALLTYPE RefreshCollection() = 0;
    virtual HRESULT STDMETHODCALLTYPE RegisterForApplicationViewChanges(IApplicationViewChangeListener * _IApplicationViewChangeListener, DWORD * _DWORD) = 0;
    virtual HRESULT STDMETHODCALLTYPE UnregisterForApplicationViewChanges(DWORD _DWORD) = 0;
};

class VDesktopAPI {
   public:
    static IServiceProvider *service_provider;
    static IVirtualDesktopManagerInternal *desktop_manager_internal;
    static IApplicationViewCollection *application_view_collection;

    static bool init();
    static bool init_service();
    static bool init_desktop_manager_internal();
    static bool init_application_view_collection();

    static int get_desktop_count();
    static void go_to(IVirtualDesktop *i_vt);
    static std::wstring get_desktop_guid_as_string(IVirtualDesktop *i_vt);
    static std::wstring get_current_desktop_guid_as_string();
    static IVirtualDesktop *get_current_desktop();
    static std::wstring guid_to_string(GUID guid);
};

// EXTERN_C const IID IID_IVirtualDesktopNotification;

// MIDL_INTERFACE("C179334C-4295-40D3-BEA1-C654D965605A")
// IVirtualDesktopNotification : public IUnknown {
//    public:
//     virtual HRESULT STDMETHODCALLTYPE VirtualDesktopCreated(IVirtualDesktop * pDesktop) = 0;

//     virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyBegin(IVirtualDesktop * pDesktopDestroyed, IVirtualDesktop * pDesktopFallback) = 0;

//     virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyFailed(IVirtualDesktop * pDesktopDestroyed, IVirtualDesktop * pDesktopFallback) = 0;

//     virtual HRESULT STDMETHODCALLTYPE VirtualDesktopDestroyed(IVirtualDesktop * pDesktopDestroyed, IVirtualDesktop * pDesktopFallback) = 0;

//     virtual HRESULT STDMETHODCALLTYPE ViewVirtualDesktopChanged(IApplicationView * pView) = 0;

//     virtual HRESULT STDMETHODCALLTYPE CurrentVirtualDesktopChanged(IVirtualDesktop * pDesktopOld, IVirtualDesktop * pDesktopNew) = 0;
// };

// EXTERN_C const IID IID_IVirtualDesktopNotificationService;

// MIDL_INTERFACE("0CD45E71-D927-4F15-8B0A-8FEF525337BF")
// IVirtualDesktopNotificationService : public IUnknown {
//    public:
//     virtual HRESULT STDMETHODCALLTYPE Register(IVirtualDesktopNotification * pNotification, DWORD * pdwCookie) = 0;

//     virtual HRESULT STDMETHODCALLTYPE Unregister(DWORD dwCookie) = 0;
// };