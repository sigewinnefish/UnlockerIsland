#include "exports.h"
#include "island.h"
#include <appmodel.h>

static LRESULT WINAPI IslandGetWindowHookImpl(int code, WPARAM wParam, LPARAM lParam)
{
    return CallNextHookEx(NULL, code, wParam, lParam);
}

HRESULT DllGetWindowsHookForHutao(HOOKPROC* pHookProc)
{
    UINT32 n{};
    if (GetCurrentPackageFamilyName(&n, nullptr) == APPMODEL_ERROR_NO_PACKAGE)
    {
        *pHookProc = nullptr;
        RETURN_WIN32(APPMODEL_ERROR_NO_PACKAGE);
    }

    *pHookProc = IslandGetWindowHookImpl;
    return S_OK;
}