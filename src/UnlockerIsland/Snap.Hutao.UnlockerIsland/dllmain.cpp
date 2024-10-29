#include "pch.h"
#include <string>
#include <vector>

using namespace Snap::Hutao::UnlockerIsland;

HANDLE hThread = NULL;
BOOL bDllExit = FALSE;

std::string minnie;

struct IslandEnvironment* pEnvironment = NULL;
struct IslandStaging staging {};

static VOID MickeyWonderPartner2Endpoint(LPVOID mickey, LPVOID house, LPVOID spell)
{
    Il2CppString* pString = staging.MickeyWonderPartner(minnie.c_str());
    Il2CppString** ppCurrent = (Il2CppString**)((PBYTE)house + 0x90);

    if (*ppCurrent == NULL || !IsValidReadPtr(*ppCurrent, sizeof(Il2CppString)))
    {
        LogA("There is no mouse.\n");
        return staging.MickeyWonderPartner2(mickey, house, spell);
    }

    LogA("Current class: %p\n", (*ppCurrent)->object.klass);
    LogW(L"Current Minnie: %s\n", &(*ppCurrent)->chars[0]);
    LogA("String class: %p\n", pString->object.klass);
    LogW(L"String Minnie: %s\n", &pString->chars[0]);

    if ((*ppCurrent)->object.klass == pString->object.klass)
    {
        *ppCurrent = pString;
    }

    staging.MickeyWonderPartner2(mickey, house, spell);
}

static VOID SetFieldOfViewEndpoint(LPVOID pThis, FLOAT value)
{
    value = std::floor(value);
    pEnvironment->DebugOriginalFieldOfView = value;

    staging.SetTargetFrameRate(pEnvironment->TargetFrameRate);
    if (value <= 30.0f)
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
    pEnvironment->DebugOpenTeamCount++;
    if (pEnvironment->RemoveOpenTeamProgress)
    {
        staging.OpenTeamPageAccordingly(false);
    }
    else
    {
        staging.OpenTeam();
    }
}

static VOID InitializeIslandStaging(IslandStaging& staging, UINT64 base, IslandEnvironment* pEnvironment)
{
    // Magic
    staging.MickeyWonder = reinterpret_cast<MickeyWonderMethod>(base + pEnvironment->FunctionOffsetMickeyWonderMethod);
    staging.MickeyWonderPartner = reinterpret_cast<MickeyWonderMethodPartner>(base + pEnvironment->FunctionOffsetMickeyWonderMethodPartner);
    staging.MickeyWonderPartner2 = reinterpret_cast<MickeyWonderMethodPartner2>(base + pEnvironment->FunctionOffsetMickeyWonderMethodPartner2);

    // Basic functions
    staging.SetFieldOfView = reinterpret_cast<SetFieldOfViewFunc>(base + pEnvironment->FunctionOffsetSetFieldOfView);
    staging.SetEnableFogRendering = reinterpret_cast<SetEnableFogRenderingFunc>(base + pEnvironment->FunctionOffsetSetEnableFogRendering);
    staging.SetTargetFrameRate = reinterpret_cast<SetTargetFrameRateFunc>(base + pEnvironment->FunctionOffsetSetTargetFrameRate);

    // Team functions
    staging.OpenTeam = reinterpret_cast<OpenTeamFunc>(base + pEnvironment->FunctionOffsetOpenTeam);
    staging.OpenTeamPageAccordingly = reinterpret_cast<OpenTeamPageAccordinglyFunc>(base + pEnvironment->FunctionOffsetOpenTeamPageAccordingly);
}

static VOID DisableVirtualMemoryProtect() {
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

    if (pEnvironment->LoopAdjustFpsOnly)
    {
        while (true)
        {
            staging.SetTargetFrameRate(pEnvironment->TargetFrameRate);
            Sleep(62);
        }
    }
    else
    {
        for (INT32 n = 0; n < 3; n++)
        {
            Il2CppArraySize* const result = staging.MickeyWonder(n);
            minnie += std::string(reinterpret_cast<char*>(&result->vector[0]), result->max_length);
        }

        Detours::Hook(&(LPVOID&)staging.MickeyWonderPartner2, MickeyWonderPartner2Endpoint);
        Detours::Hook(&(LPVOID&)staging.SetFieldOfView, SetFieldOfViewEndpoint);
        Detours::Hook(&(LPVOID&)staging.OpenTeam, OpenTeamEndpoint);

        WaitForSingleObject(GetCurrentThread(), INFINITE);
    }

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
#pragma endregion