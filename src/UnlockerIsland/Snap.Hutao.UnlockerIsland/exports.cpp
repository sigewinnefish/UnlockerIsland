#include "exports.h"
#include "island.h"

static LRESULT WINAPI IslandGetWindowHookImpl(int code, WPARAM wParam, LPARAM lParam)
{
    return CallNextHookEx(NULL, code, wParam, lParam);
}

HRESULT IslandGetWindowHook(_Out_ HOOKPROC* pHookProc)
{
    *pHookProc = IslandGetWindowHookImpl;
    return S_OK;
}

HRESULT DllGetWindowsHookForHutao(_Out_ HOOKPROC* pHookProc)
{
    *pHookProc = IslandGetWindowHookImpl;
    return S_OK;
}

HRESULT IslandGetFunctionOffsetsSize(_Out_ UINT64* pCount)
{
    *pCount = sizeof(Snap::Hutao::UnlockerIsland::FunctionOffsets);
    return S_OK;
}