#include "exports.h"
#include "island.h"
#include <appmodel.h>

static LRESULT WINAPI IslandGetWindowHookImpl(int code, WPARAM wParam, LPARAM lParam)
{
    return CallNextHookEx(NULL, code, wParam, lParam);
}

HRESULT DllGetWindowsHookForHutao(_Out_ HOOKPROC* pHookProc)
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

HRESULT IslandGetFunctionOffsetsSize(_Out_ UINT64* pCount)
{
    *pCount = sizeof(Snap::Hutao::UnlockerIsland::FunctionOffsets);
    return S_OK;
}