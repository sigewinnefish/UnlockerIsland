#include "pch.h"
#include <string>
#include <vector>
#include <mutex>

using namespace Snap::Hutao::UnlockerIsland;

HANDLE hThread = NULL;
BOOL bDllExit = FALSE;
std::once_flag ofTouchScreen;

std::string minnie;

struct Snap::Hutao::UnlockerIsland::IslandEnvironment* pEnvironment = NULL;
struct IslandStaging staging {};

static VOID MickeyWonderPartner2Endpoint(LPVOID mickey, LPVOID house, LPVOID spell)
{
    BOOL bFound = FALSE;

    Il2CppString* pString = staging.MickeyWonderPartner(minnie.c_str());
    Il2CppString** ppCurrent = NULL;
    for (int offset = 0x10; offset < 0x233; offset += 0x8)
    {
        ppCurrent = (Il2CppString**)((PBYTE)house + offset);
        if (*ppCurrent == NULL || !IsValidReadPtr(*ppCurrent, sizeof(Il2CppString)))
        {
            continue;
        }

        if ((*ppCurrent)->length != 66)
        {
            continue;
        }

        bFound = TRUE;
        LogA("Minnie at 0x%x\n", offset);
        break;
    }

    if (!bFound)
    {
        LogA("Minnie not found.\n");
        return staging.MickeyWonderPartner2(mickey, house, spell);
    }
    
    LogA("Current class: %p\n", (*ppCurrent)->object.klass);
    LogW(L"Current Minnie: %s\n", &(*ppCurrent)->chars[0]);
    LogA("String class: %p\n", pString->object.klass);
    LogW(L"String Minnie: %s\n", &pString->chars[0]);

    *ppCurrent = pString;

    staging.MickeyWonderPartner2(mickey, house, spell);
}

static VOID SetFieldOfViewEndpoint(LPVOID pThis, FLOAT value)
{
    std::call_once(ofTouchScreen, [&]()
    {
        if (pEnvironment->UsingTouchScreen)
        {
            __try
            {
                LogA("Call SwitchInputDeviceToTouchScreen");
                staging.SwitchInputDeviceToTouchScreen(NULL);
            }
            __except (EXCEPTION_EXECUTE_HANDLER)
            {
                LogA("Catch SwitchInputDeviceToTouchScreen");
            }
        }
    });

    if (pEnvironment->EnableSetTargetFrameRate)
    {
        staging.SetTargetFrameRate(pEnvironment->TargetFrameRate);
    }

    if (!pEnvironment->EnableSetFieldOfView)
    {
        return staging.SetFieldOfView(pThis, value);
    }

    if (std::floor(value) <= 30.0f)
    {
        staging.SetEnableFogRendering(false);
        staging.SetFieldOfView(pThis, pEnvironment->FixLowFovScene ? value : pEnvironment->FieldOfView);
    }
    else
    {
        staging.SetEnableFogRendering(!pEnvironment->DisableFog);
        staging.SetFieldOfView(pThis, pEnvironment->FieldOfView);
    }
}

static VOID OpenTeamEndpoint()
{
    if (pEnvironment->RemoveOpenTeamProgress && staging.CheckCanEnter())
    {
        staging.OpenTeamPageAccordingly(false);
    }
    else
    {
        staging.OpenTeam();
    }
}

static VOID SetupQuestBannerEndpoint(LPVOID pThis)
{
    LogA("SetupViewEndpoint called\n");
    if (!pEnvironment->HideQuestBanner)
    {
        staging.SetupQuestBanner(pThis);
    }
    else
    {
        LogA("Hiding banner\n");
        Il2CppString* bannerString = staging.MickeyWonderPartner("Canvas/Pages/InLevelMapPage/GrpMap/GrpPointTips/Layout/QuestBanner");
        LogA("BannerString at 0x%x\n", bannerString);
        LPVOID banner = staging.FindGameObject(bannerString);
        if (banner)
        {
            LogA("Banner found\n");
            staging.SetActive(banner, false);
        }
    }
}

static bool EventCameraMoveEndpoint(LPVOID pThis, LPVOID event)
{
    if (pEnvironment->DisableEventCameraMove)
    {
        return true;
    }
    else
    {
        return staging.EventCameraMove(pThis, event);
    }
}

static VOID ShowOneDamageTextExEndpoint(LPVOID pThis, int type, int damageType, int showType, float damage, Il2CppString* showText, LPVOID worldPos, LPVOID attackee, int elementReactionType)
{
    LogA("[Damage]:[type: %d] [damageType: %d] [showType: %d] [damage: %f] [%p] [%p] [%d]\n", type, damageType, showType, damage, worldPos, attackee, elementReactionType);
    if (pEnvironment->DisableShowDamageText)
    {
        return;
    }

    staging.ShowOneDamageTextEx(pThis, type, damageType, showType, damage, showText, worldPos, attackee, elementReactionType);
}

static DWORD WINAPI IslandThread(LPVOID lpParam)
{
#ifdef _DEBUG
    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
#endif
    
    const UNIQUE_HANDLE hFile = UNIQUE_HANDLE(OpenFileMappingW(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, ISLAND_ENVIRONMENT_NAME));
    if (!hFile)
    {
        return GetLastError();
    }

    const UNIQUE_VIEW_OF_FILE lpView = UNIQUE_VIEW_OF_FILE(MapViewOfFile(hFile.get(), FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0));
    if (!lpView)
    {
        return GetLastError();
    }

    pEnvironment = static_cast<IslandEnvironment*>(lpView.get());

    pEnvironment->State = IslandState::Started;

    UINT64 base = (UINT64)GetModuleHandleW(NULL);
    InitializeIslandStaging(staging, base, pEnvironment);

    for (INT32 n = 0; n < 3; n++)
    {
        Il2CppArraySize* const result = staging.MickeyWonder(n);
        minnie += std::string(reinterpret_cast<char*>(&result->vector[0]), result->max_length);
    }

    Detours::Hook(&(LPVOID&)staging.MickeyWonderPartner2, MickeyWonderPartner2Endpoint);
    Detours::Hook(&(LPVOID&)staging.SetFieldOfView, SetFieldOfViewEndpoint);
    Detours::Hook(&(LPVOID&)staging.OpenTeam, OpenTeamEndpoint);
    Detours::Hook(&(LPVOID&)staging.SetupQuestBanner, SetupQuestBannerEndpoint);
    Detours::Hook(&(LPVOID&)staging.EventCameraMove, EventCameraMoveEndpoint);
    Detours::Hook(&(LPVOID&)staging.ShowOneDamageTextEx, ShowOneDamageTextExEndpoint);

    WaitForSingleObject(GetCurrentThread(), INFINITE);

    pEnvironment->State = IslandState::Stopped;

    FreeLibraryAndExitThread(static_cast<HMODULE>(lpParam), 0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    if (hModule)
    {
        DisableThreadLibraryCalls(hModule);
    }

    // Check if the module is loaded by us
    if (GetModuleHandleW(L"Snap.Hutao.dll"))
    {
        return TRUE;
    }

    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        if (hModule)
        {
            LdrAddRefDll(LDR_ADDREF_DLL_PIN, hModule);
        }

        DisableVirtualMemoryProtect();

        hThread = CreateThread(NULL, 0, IslandThread, hModule, 0, NULL);
        if (!hThread)
        {
            return FALSE;
        }

        CloseHandle(hThread);
        break;

    case DLL_PROCESS_DETACH:
        if (!ISLAND_FEATURE_HANDLE_DLL_PROCESS_DETACH)
        {
            break;
        }

        if (lpReserved)
        {
            bDllExit = TRUE;
        }

        Sleep(500);
        break;
    }

    return TRUE;
}

#pragma region Exports

static LRESULT WINAPI IslandGetWindowHookImpl(int code, WPARAM wParam, LPARAM lParam)
{
    return CallNextHookEx(NULL, code, wParam, lParam);
}

ISLAND_API HRESULT WINAPI IslandGetWindowHook(_Out_ HOOKPROC* pHookProc)
{
    *pHookProc = IslandGetWindowHookImpl;
    return S_OK;
}

ISLAND_API HRESULT WINAPI IslandGetFunctionOffsetsSize(_Out_ UINT64* pCount)
{
    *pCount = sizeof(FunctionOffsets);
    return S_OK;
}
#pragma endregion