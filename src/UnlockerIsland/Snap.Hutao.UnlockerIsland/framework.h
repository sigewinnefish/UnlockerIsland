#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <wil/resource.h>
#include "hook.h"

#define ISLAND_API EXTERN_C __declspec(dllexport)

// Feature switch
constexpr bool ISLAND_FEATURE_HANDLE_DLL_PROCESS_DETACH = true;

constexpr PCWSTR ISLAND_ENVIRONMENT_NAME = L"4F3E8543-40F7-4808-82DC-21E48A6037A7";

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);

static VOID DisableProtectVirtualMemory()
{
    // ntdll.dll must be loaded
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (!ntdll)
    {
        return;
    }

    FARPROC pNtProtectVirtualMemory = GetProcAddress(ntdll, "NtProtectVirtualMemory");
    FARPROC pNtQuerySection = GetProcAddress(ntdll, "NtQuerySection");

    DWORD old;
    VirtualProtect(pNtProtectVirtualMemory, 1, PAGE_EXECUTE_READWRITE, &old);
    *(PUINT64)pNtProtectVirtualMemory = *(PUINT64)pNtQuerySection & ~(0xFFUi64 << 32) | (UINT64)(*(PUINT32)((UINT64)pNtQuerySection + 4) - 1) << 32;
    VirtualProtect(pNtProtectVirtualMemory, 1, old, &old);
}

static bool IsValidReadPtr(LPVOID ptr, SIZE_T size)
{
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery(ptr, &mbi, sizeof(mbi)))
    {
        return WI_IsAnyFlagSet(mbi.Protect, PAGE_READWRITE | PAGE_READONLY);
    }

    return false;
}

inline void LogA(PCSTR format, ...)
{
#ifdef _DEBUG
    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
#endif
}

inline void LogW(PCWSTR format, ...)
{
#ifdef _DEBUG
    va_list args;
    va_start(args, format);
    vfwprintf(stdout, format, args);
    va_end(args);
#endif
}