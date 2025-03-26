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
    UINT32 MickeyWonder;
    UINT32 MickeyWonderPartner;
    UINT32 MickeyWonderPartner2;
    UINT32 SetFieldOfView;
    UINT32 SetEnableFogRendering;
    UINT32 SetTargetFrameRate;
    UINT32 OpenTeam;
    UINT32 OpenTeamPageAccordingly;
    UINT32 CheckCanEnter;
    UINT32 SetupQuestBanner;
    UINT32 FindGameObject;
    UINT32 SetActive;
    UINT32 EventCameraMove;
    UINT32 ShowOneDamageTextEx;
    UINT32 SwitchInputDeviceToTouchScreen;
};

struct Snap::Hutao::UnlockerIsland::IslandEnvironment
{
    enum IslandState State;
    DWORD LastError;

    FunctionOffsets FunctionOffsets;

    BOOL EnableSetFieldOfView;
    FLOAT FieldOfView;
    BOOL FixLowFovScene;
    BOOL DisableFog;
    BOOL EnableSetTargetFrameRate;
    INT32 TargetFrameRate;
    BOOL RemoveOpenTeamProgress;
    BOOL HideQuestBanner;
    BOOL DisableEventCameraMove;
    BOOL DisableShowDamageText;
    BOOL UsingTouchScreen;
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

typedef struct Il2CppExceptionWrapper {
    LPVOID ex;
} Il2CppExceptionWrapper;

typedef Il2CppArraySize* (*MickeyWonderMethod)(INT32 value);
typedef Il2CppString* (*MickeyWonderMethodPartner)(PCSTR value);
typedef VOID (*MickeyWonderMethodPartner2)(LPVOID mickey, LPVOID house, LPVOID spell);
typedef VOID (*SetFieldOfViewMethod)(LPVOID this__, FLOAT value);
typedef VOID (*SetEnableFogRenderingMethod)(bool value);
typedef VOID (*SetTargetFrameRateMethod)(INT32 value);
typedef VOID (*OpenTeamMethod)();
typedef VOID (*OpenTeamPageAccordinglyMethod)(bool value);
typedef bool (*CheckCanEnterMethod)();
typedef VOID (*SetupQuestBannerMethod)(LPVOID this__);
typedef LPVOID (*FindGameObjectMethod)(Il2CppString* name);
typedef VOID (*SetActiveMethod)(LPVOID this__, bool value);
typedef bool (*EventCameraMoveMethod)(LPVOID this__, LPVOID event);
typedef VOID (*ShowOneDamageTextExMethod)(LPVOID this__, int type, int damageType, int showType, float damage, Il2CppString* showText, LPVOID worldPos, LPVOID attackee, int elementReactionType);
typedef VOID (*SwitchInputDeviceToTouchScreenMethod)(LPVOID this__);

struct Snap::Hutao::UnlockerIsland::IslandStaging
{
    MickeyWonderMethod MickeyWonder;
    MickeyWonderMethodPartner MickeyWonderPartner;
    MickeyWonderMethodPartner2 MickeyWonderPartner2;
    SetFieldOfViewMethod SetFieldOfView;
    SetEnableFogRenderingMethod SetEnableFogRendering;
    SetTargetFrameRateMethod SetTargetFrameRate;
    OpenTeamMethod OpenTeam;
    OpenTeamPageAccordinglyMethod OpenTeamPageAccordingly;
    CheckCanEnterMethod CheckCanEnter;
    SetupQuestBannerMethod SetupQuestBanner;
    FindGameObjectMethod FindGameObject;
    SetActiveMethod SetActive;
    EventCameraMoveMethod EventCameraMove;
    ShowOneDamageTextExMethod ShowOneDamageTextEx;
    SwitchInputDeviceToTouchScreenMethod SwitchInputDeviceToTouchScreen;
};

void LogA(PCSTR format, ...);

void LogW(PCWSTR format, ...);

static VOID Snap::Hutao::UnlockerIsland::InitializeIslandStaging(IslandStaging& staging, UINT64 base, IslandEnvironment* pEnvironment)
{
#define BIND(target, method) target = reinterpret_cast<decltype(target)>(base + pEnvironment->FunctionOffsets.method)

    // Magic
    BIND(staging.MickeyWonder, MickeyWonder);
    BIND(staging.MickeyWonderPartner, MickeyWonderPartner);
    BIND(staging.MickeyWonderPartner2, MickeyWonderPartner2);

    // Basic functions
    BIND(staging.SetFieldOfView, SetFieldOfView);
    BIND(staging.SetEnableFogRendering, SetEnableFogRendering);
    BIND(staging.SetTargetFrameRate, SetTargetFrameRate);

    // Team functions
    BIND(staging.OpenTeam, OpenTeam);
    BIND(staging.OpenTeamPageAccordingly, OpenTeamPageAccordingly);
    BIND(staging.CheckCanEnter, CheckCanEnter);

    // Banner functions
    BIND(staging.SetupQuestBanner, SetupQuestBanner);
    BIND(staging.FindGameObject, FindGameObject);
    BIND(staging.SetActive, SetActive);

    // Virtual Camera functions
    BIND(staging.EventCameraMove, EventCameraMove);

    // Damage text functions
    BIND(staging.ShowOneDamageTextEx, ShowOneDamageTextEx);

    // Touch screen functions
    BIND(staging.SwitchInputDeviceToTouchScreen, SwitchInputDeviceToTouchScreen);
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