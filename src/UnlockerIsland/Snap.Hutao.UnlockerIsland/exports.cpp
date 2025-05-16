#include "exports.h"
#include "island.h"

static LRESULT WINAPI IslandGetWindowHookImpl(int code, WPARAM wParam, LPARAM lParam)
{
    return CallNextHookEx(NULL, code, wParam, lParam);
}

ISLAND_API HRESULT WINAPI IslandGetWindowHook(_Out_ HOOKPROC* pHookProc)
{
    *pHookProc = IslandGetWindowHookImpl;
    return S_OK;
}

ISLAND_API HRESULT WINAPI DllGetWindowHookForHutao(_Out_ HOOKPROC* pHookProc)
{
    *pHookProc = IslandGetWindowHookImpl;
    return S_OK;
}

ISLAND_API HRESULT WINAPI IslandGetFunctionOffsetsSize(_Out_ UINT64* pCount)
{
    *pCount = sizeof(Snap::Hutao::UnlockerIsland::FunctionOffsets);
    return S_OK;
}