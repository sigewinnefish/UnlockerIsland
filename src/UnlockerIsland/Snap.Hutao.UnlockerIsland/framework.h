#pragma once

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
// Windows 头文件
#include <windows.h>
#include <wil/resource.h>

#define ISLAND_API EXTERN_C __declspec(dllexport)

// Feature switch
constexpr auto ISLAND_FEATURE_HANDLE_DLL_PROCESS_DETACH = true;

constexpr PCWSTR ISLAND_ENVIRONMENT_NAME = L"4F3E8543-40F7-4808-82DC-21E48A6037A7";

// This function is only meant to get an arbitrary function pointer from the DLL
// So that we can use SetWindowHookEx to inject the DLL into the game
ISLAND_API HRESULT WINAPI IslandGetWindowHook(_Out_ HOOKPROC* pHookProc);

static VOID DisableVirtualMemoryProtect()
{
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
    if (VirtualQuery(ptr, &mbi, sizeof(mbi))) {
        return (mbi.Protect & PAGE_READWRITE) || (mbi.Protect & PAGE_READONLY);
    }
    return false;
}

namespace Snap
{
    namespace Hutao
    {
        namespace UnlockerIsland
        {
            using UNIQUE_HANDLE = wil::unique_any<HANDLE, decltype(&CloseHandle), CloseHandle>;
            using UNIQUE_VIEW_OF_FILE = wil::unique_any<LPVOID, decltype(&UnmapViewOfFile), UnmapViewOfFile>;

            enum struct IslandState;

            struct FunctionOffsets;
            struct IslandEnvironment;
            struct IslandStaging;

            static VOID InitializeIslandStaging(IslandStaging& staging, UINT64 base, IslandEnvironment* pEnvironment);
        }
    }
};

enum struct Snap::Hutao::UnlockerIsland::IslandState : int
{
    None = 0,
    Error = 1,
    Started = 2,
    Stopped = 3,
};

struct Snap::Hutao::UnlockerIsland::FunctionOffsets
{
    UINT32 MickeyWonderMethod;
    UINT32 MickeyWonderMethodPartner;
    UINT32 MickeyWonderMethodPartner2;
    UINT32 SetFieldOfView;
    UINT32 SetEnableFogRendering;
    UINT32 SetTargetFrameRate;
    UINT32 OpenTeam;
    UINT32 OpenTeamPageAccordingly;
    UINT32 CheckCanEnter;
    UINT32 SetupView;
    UINT32 FindGameObject;
    UINT32 SetActive;
};

struct Snap::Hutao::UnlockerIsland::IslandEnvironment
{
    enum IslandState State;
    DWORD LastError;

    FunctionOffsets FunctionOffsets;

    BOOL HookingSetFieldOfView;
    BOOL EnableSetFieldOfView;
    FLOAT FieldOfView;
    BOOL FixLowFovScene;
    BOOL DisableFog;
    BOOL EnableSetTargetFrameRate;
    INT32 TargetFrameRate;
    BOOL HookingOpenTeam;
    BOOL RemoveOpenTeamProgress;
    BOOL HookingMickeyWonderPartner2;
    BOOL HookingSetupView;
    BOOL HideQuestBanner;
};

typedef struct Il2CppObject
{
    LPVOID klass;
    LPVOID monitor;
} Il2CppObject;

typedef struct Il2CppArraySize
{
    Il2CppObject object;
    LPVOID bounds;
    SIZE_T max_length;
    UCHAR vector[32];
} Il2CppArraySize;

typedef struct Il2CppString
{
    Il2CppObject object;
    INT32 length;
    WCHAR chars[32];
} Il2CppString;

typedef Il2CppArraySize* (*MickeyWonderMethod)(INT32 value);
typedef Il2CppString* (*MickeyWonderMethodPartner)(PCSTR value);
typedef VOID (*MickeyWonderMethodPartner2)(LPVOID mickey, LPVOID house, LPVOID spell);
typedef VOID (*SetFieldOfViewFunc)(LPVOID this__, FLOAT value);
typedef VOID (*SetEnableFogRenderingFunc)(bool value);
typedef VOID (*SetTargetFrameRateFunc)(INT32 value);
typedef VOID (*OpenTeamFunc)();
typedef VOID (*OpenTeamPageAccordinglyFunc)(bool value);
typedef bool (*CheckCanEnterFunc)();
typedef VOID (*SetupViewFunc)(LPVOID this__);
typedef LPVOID (*FindGameObjectFunc)(Il2CppString* name);
typedef VOID (*SetActiveFunc)(LPVOID this__, bool value);

struct Snap::Hutao::UnlockerIsland::IslandStaging
{
    MickeyWonderMethod MickeyWonder;
    MickeyWonderMethodPartner MickeyWonderPartner;
    MickeyWonderMethodPartner2 MickeyWonderPartner2;
    SetFieldOfViewFunc SetFieldOfView;
    SetEnableFogRenderingFunc SetEnableFogRendering;
    SetTargetFrameRateFunc SetTargetFrameRate;
    OpenTeamFunc OpenTeam;
    OpenTeamPageAccordinglyFunc OpenTeamPageAccordingly;
    CheckCanEnterFunc CheckCanEnter;
    SetupViewFunc SetupView;
    FindGameObjectFunc FindGameObject;
    SetActiveFunc SetActive;
};

static VOID Snap::Hutao::UnlockerIsland::InitializeIslandStaging(IslandStaging& staging, UINT64 base, IslandEnvironment* pEnvironment)
{
    // Magic
    staging.MickeyWonder = reinterpret_cast<MickeyWonderMethod>(base + pEnvironment->FunctionOffsets.MickeyWonderMethod);
    staging.MickeyWonderPartner = reinterpret_cast<MickeyWonderMethodPartner>(base + pEnvironment->FunctionOffsets.MickeyWonderMethodPartner);
    staging.MickeyWonderPartner2 = reinterpret_cast<MickeyWonderMethodPartner2>(base + pEnvironment->FunctionOffsets.MickeyWonderMethodPartner2);

    // Basic functions
    staging.SetFieldOfView = reinterpret_cast<SetFieldOfViewFunc>(base + pEnvironment->FunctionOffsets.SetFieldOfView);
    staging.SetEnableFogRendering = reinterpret_cast<SetEnableFogRenderingFunc>(base + pEnvironment->FunctionOffsets.SetEnableFogRendering);
    staging.SetTargetFrameRate = reinterpret_cast<SetTargetFrameRateFunc>(base + pEnvironment->FunctionOffsets.SetTargetFrameRate);

    // Team functions
    staging.OpenTeam = reinterpret_cast<OpenTeamFunc>(base + pEnvironment->FunctionOffsets.OpenTeam);
    staging.OpenTeamPageAccordingly = reinterpret_cast<OpenTeamPageAccordinglyFunc>(base + pEnvironment->FunctionOffsets.OpenTeamPageAccordingly);
    staging.CheckCanEnter = reinterpret_cast<CheckCanEnterFunc>(base + pEnvironment->FunctionOffsets.CheckCanEnter);

    // Banner functions
    staging.SetupView = reinterpret_cast<SetupViewFunc>(base + pEnvironment->FunctionOffsets.SetupView);
    staging.FindGameObject = reinterpret_cast<FindGameObjectFunc>(base + pEnvironment->FunctionOffsets.FindGameObject);
    staging.SetActive = reinterpret_cast<SetActiveFunc>(base + pEnvironment->FunctionOffsets.SetActive);
}

inline void LogA(const char* format, ...)
{
#ifdef _DEBUG
    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
#endif
}

inline void LogW(const WCHAR* format, ...)
{
#ifdef _DEBUG
    va_list args;
    va_start(args, format);
    vfwprintf(stdout, format, args);
    va_end(args);
#endif
}