#pragma once

#include "framework.h"

ISLAND_API HRESULT WINAPI DllGetWindowHookForHutao(_Out_ HOOKPROC* pHookProc);

ISLAND_API HRESULT WINAPI IslandGetWindowHook(_Out_ HOOKPROC* pHookProc);

ISLAND_API HRESULT WINAPI IslandGetFunctionOffsetsSize(_Out_ UINT64* pCount);