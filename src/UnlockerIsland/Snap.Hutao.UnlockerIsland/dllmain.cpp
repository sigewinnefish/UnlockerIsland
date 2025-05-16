#include "framework.h"
#include "island.h"
#include "ntprivate.h"

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

        DisableProtectVirtualMemory();

        {
            wil::unique_handle hThread(CreateThread(NULL, 0, &Snap::Hutao::UnlockerIsland::IslandThread, hModule, 0, NULL));
            if (!hThread)
            {
                return FALSE;
            }
        }

        break;

    case DLL_PROCESS_DETACH:
        if (!ISLAND_FEATURE_HANDLE_DLL_PROCESS_DETACH)
        {
            break;
        }

        // What's this for?
        Sleep(500);
        break;
    }

    return TRUE;
}