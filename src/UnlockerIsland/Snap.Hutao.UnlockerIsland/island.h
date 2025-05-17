#pragma once

#include "framework.h"
#include "il2cpp.h"

namespace Snap::Hutao::UnlockerIsland
{
    typedef Il2CppArraySize* (*MickeyWonderMethod)(INT32 value);
    typedef Il2CppString* (*MickeyWonderMethodPartner)(PCSTR value);
    typedef VOID(*MickeyWonderMethodPartner2)(LPVOID mickey, LPVOID house, LPVOID spell);
    typedef VOID(*SetFieldOfViewMethod)(LPVOID this__, FLOAT value);
    typedef VOID(*SetEnableFogRenderingMethod)(bool value);
    typedef VOID(*SetTargetFrameRateMethod)(INT32 value);
    typedef VOID(*OpenTeamMethod)();
    typedef VOID(*OpenTeamPageAccordinglyMethod)(bool value);
    typedef bool (*CheckCanEnterMethod)();
    typedef VOID(*SetupQuestBannerMethod)(LPVOID this__);
    typedef LPVOID(*FindGameObjectMethod)(Il2CppString* name);
    typedef VOID(*SetActiveMethod)(LPVOID this__, bool value);
    typedef bool (*EventCameraMoveMethod)(LPVOID this__, LPVOID event);
    typedef VOID(*ShowOneDamageTextExMethod)(LPVOID this__, int type, int damageType, int showType, float damage, Il2CppString* showText, LPVOID worldPos, LPVOID attackee, int elementReactionType);
    typedef VOID(*SwitchInputDeviceToTouchScreenMethod)(LPVOID this__);
    typedef VOID(*MickeyWonderCombineEntryMethod)(LPVOID this__);
    typedef bool(*MickeyWonderCombineEntryMethodPartner)(Il2CppString* name, LPVOID arg2, LPVOID arg3, LPVOID arg4, LPVOID arg5);

    enum struct IslandState : int
    {
        None = 0,
        Error = 1,
        Started = 2,
        Stopped = 3,
    };

    struct FunctionOffsets
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
        UINT32 MickeyWonderCombineEntry;
        UINT32 MickeyWonderCombineEntryPartner;
    };

    struct IslandEnvironment
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
        BOOL RedirectCombineEntry;
    };

    struct IslandStaging
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
        MickeyWonderCombineEntryMethod MickeyWonderCombineEntry;
        MickeyWonderCombineEntryMethodPartner MickeyWonderCombineEntryPartner;
    };

    extern DWORD CALLBACK IslandThread(LPVOID lpParam);

    VOID InitializeIslandStaging(IslandStaging& staging, UINT64 base, IslandEnvironment* pEnvironment);

    VOID MickeyWonderPartner2Endpoint(LPVOID mickey, LPVOID house, LPVOID spell);
    VOID SetFieldOfViewEndpoint(LPVOID pThis, FLOAT value);
    VOID OpenTeamEndpoint();
    VOID SetupQuestBannerEndpoint(LPVOID pThis);
    bool EventCameraMoveEndpoint(LPVOID pThis, LPVOID event);
    VOID ShowOneDamageTextExEndpoint(LPVOID pThis, int type, int damageType, int showType, float damage, Il2CppString* showText, LPVOID worldPos, LPVOID attackee, int elementReactionType);
    VOID MickeyWonderCombineEntryEndpoint(LPVOID pThis);
}