#include "island.h"
#include <cmath>
#include <mutex>
#include <string>
#include "link.h"

namespace Snap::Hutao::UnlockerIsland
{
    std::string minnie;
    IslandEnvironment* pEnvironment = nullptr;
    IslandStaging staging{};

    static std::once_flag ofTouchScreen;

    DWORD IslandThread(LPVOID lpParam)
    {
#ifdef _DEBUG
        RETURN_LAST_ERROR_IF(!AllocConsole());
        freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
#endif
        HANDLE hThread = ::CreateThread(NULL, 0, LogServerProc, NULL, 0, NULL);
        if (hThread != NULL)
        {
            CloseHandle(hThread);
            hThread = NULL;
        }

        wil::unique_handle hFile(OpenFileMappingW(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, ISLAND_ENVIRONMENT_NAME));
        RETURN_LAST_ERROR_IF_NULL(hFile);

        wil::unique_any<LPVOID, decltype(&UnmapViewOfFile), UnmapViewOfFile> lpView(MapViewOfFile(hFile.get(), FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0));
        RETURN_LAST_ERROR_IF_NULL(lpView);

        pEnvironment = static_cast<IslandEnvironment*>(lpView.get());

        pEnvironment->State = IslandState::Started;

        InitializeIslandStaging(staging, (UINT64)GetModuleHandleW(NULL), pEnvironment);

        Detours::Hook(&(LPVOID&)staging.GameManagerAwake, GameManagerAwakeEndpoint);
        Detours::Hook(&(LPVOID&)staging.MickeyWonderPartner2, MickeyWonderPartner2Endpoint);
        Detours::Hook(&(LPVOID&)staging.GetTargetFrameRate, GetTargetFrameRateEndpoint);
        Detours::Hook(&(LPVOID&)staging.SetFieldOfView, SetFieldOfViewEndpoint);
        Detours::Hook(&(LPVOID&)staging.OpenTeam, OpenTeamEndpoint);
        Detours::Hook(&(LPVOID&)staging.SetupQuestBanner, SetupQuestBannerEndpoint);
        Detours::Hook(&(LPVOID&)staging.EventCameraMove, EventCameraMoveEndpoint);
        Detours::Hook(&(LPVOID&)staging.ShowOneDamageTextEx, ShowOneDamageTextExEndpoint);
        Detours::Hook(&(LPVOID&)staging.MickeyWonderCombineEntry, MickeyWonderCombineEntryEndpoint);

        RETURN_LAST_ERROR_IF(WAIT_FAILED == WaitForSingleObject(GetCurrentThread(), INFINITE));

        pEnvironment->State = IslandState::Stopped;

        FreeLibraryAndExitThread(static_cast<HMODULE>(lpParam), 0);
        return 0;
    }

    static VOID InitializeIslandStaging(IslandStaging& staging, UINT64 base, IslandEnvironment* pEnvironment)
    {
#define BIND(target, method) target = reinterpret_cast<decltype(target)>(base + pEnvironment->FunctionOffsets.method)

        // Magic
        BIND(staging.GameManagerAwake, GameManagerAwake);
        BIND(staging.MickeyWonder, MickeyWonder);
        BIND(staging.MickeyWonderPartner, MickeyWonderPartner);
        BIND(staging.MickeyWonderPartner2, MickeyWonderPartner2);

        // Basic functions
        BIND(staging.SetFieldOfView, SetFieldOfView);
        BIND(staging.SetEnableFogRendering, SetEnableFogRendering);
        BIND(staging.GetTargetFrameRate, GetTargetFrameRate);
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

        // Combine functions
        BIND(staging.MickeyWonderCombineEntry, MickeyWonderCombineEntry);
        BIND(staging.MickeyWonderCombineEntryPartner, MickeyWonderCombineEntryPartner);
    }

    static void MickeyWonderfunc() {
        for (INT32 n = 0; n < 3; n++)
        {
            Il2CppArraySize* const result = staging.MickeyWonder(n);
            minnie += std::string(reinterpret_cast<char*>(&result->vector[0]), result->max_length);
        }
    }

    static VOID GameManagerAwakeEndpoint()
    {
        staging.GameManagerAwake();
        BOOL fPending;
        if (!InitOnceBeginInitialize(&InitOnce, 0, &fPending, 0))
            abort();
        if (fPending)
        {
            MickeyWonderfunc();
            if (!InitOnceComplete(&InitOnce, 0, 0))
                abort();
        }

    }

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
            LogA(L"Minnie at 0x%x\r\n", offset);
            break;
        }

        if (!bFound)
        {
            LogA(L"Minnie not found.\r\n");
            return staging.MickeyWonderPartner2(mickey, house, spell);
        }

        LogA(L"Current class: %p\r\n", (*ppCurrent)->object.klass);
        LogW(L"Current Minnie: %s\r\n", &(*ppCurrent)->chars[0]);
        LogA(L"String class: %p\r\n", pString->object.klass);
        LogW(L"String Minnie: %s\r\n", &pString->chars[0]);

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
                        LogA(L"Call SwitchInputDeviceToTouchScreen");
                        staging.SwitchInputDeviceToTouchScreen(NULL);
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER)
                    {
                        LogA(L"Catch SwitchInputDeviceToTouchScreen");
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

    static INT32 GetTargetFrameRateEndpoint()
    {
        return 0x3C;
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
        LogA(L"SetupViewEndpoint called\r\n");
        if (!pEnvironment->HideQuestBanner)
        {
            staging.SetupQuestBanner(pThis);
        }
        else
        {
            LogA(L"Hiding banner\r\n");
            Il2CppString* bannerString = staging.MickeyWonderPartner("Canvas/Pages/InLevelMapPage/GrpMap/GrpPointTips/Layout/QuestBanner");
            LogA(L"BannerString at 0x%x\r\n", bannerString);
            LPVOID banner = staging.FindGameObject(bannerString);
            if (banner)
            {
                LogA(L"Banner found\r\n");
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
        LogA(L"[Damage]:[type: %d] [damageType: %d] [showType: %d] [damage: %f] [%p] [%p] [%d]\r\n", type, damageType, showType, damage, worldPos, attackee, elementReactionType);
        sharedamage(type, damage);
        if (pEnvironment->DisableShowDamageText)
        {
            return;
        }

        staging.ShowOneDamageTextEx(pThis, type, damageType, showType, damage, showText, worldPos, attackee, elementReactionType);
    }
    
    static VOID MickeyWonderCombineEntryEndpoint(LPVOID pThis)
    {
        if (pEnvironment->RedirectCombineEntry)
        {
            staging.MickeyWonderCombineEntryPartner(staging.MickeyWonderPartner("SynthesisPage"), NULL, NULL, NULL, NULL);
            return;
        }

        staging.MickeyWonderCombineEntry(pThis);
    }
}