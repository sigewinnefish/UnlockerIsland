#pragma once

#include <windows.h>
#include <stdio.h>

// LOG------------------------------------------------------------------------------------------------------

#define LogA LogW
#define LogW sendlog

inline HANDLE hPipe;
inline TCHAR  chBuf[1024];
inline DWORD  cbRead, cbToWrite, cbWritten, dwMode;
inline LPCWSTR lpszPipename = L"\\\\.\\pipe\\mfcislandpipe";
inline wchar_t buf[512];

inline void startlogging() {

    WaitNamedPipe(lpszPipename, INFINITE);

    hPipe = CreateFile(
        lpszPipename,   // pipe name  
        GENERIC_WRITE,  // write access 
        0,              // no sharing 
        NULL,           // default security attributes
        OPEN_EXISTING,  // opens existing pipe 
        0,              // default attributes 
        NULL);
}

inline void sendlog(PCWSTR format, ...)
{


    va_list args;
    va_start(args, format);
    vswprintf(buf, 512, format, args);

    cbToWrite = (lstrlen(buf) + 1) * sizeof(WCHAR);
    
    WriteFile(
        hPipe,
        buf,
        cbToWrite,
        &cbWritten, 
        NULL);

    va_end(args);

}

DWORD WINAPI LogServerProc(LPVOID lpParameter)
{
    startlogging();
    return 0;
}

// DAMAGE------------------------------------------------------------------------------------------------------

inline PCWSTR ISLAND_ENVIRONMENT_EXTRA = L"B6134FBA-EBA9-49F0-B670-9DFE43293329";

struct DamageFromIsland
{
    float current;
};

struct ElementalDamages
{
    DamageFromIsland Physical;
    DamageFromIsland Pyro;
    DamageFromIsland Hydro;
    DamageFromIsland Dendro;
    DamageFromIsland Electro;
    DamageFromIsland Cryo;
    DamageFromIsland Anemo;
    DamageFromIsland Geo;
    short type;
};

inline LPVOID filemapping(PCWSTR NAME)
{
    HANDLE h = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, FALSE, NAME);

    if (h == NULL)
    {
        h = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_EXECUTE_READWRITE, 0, 1024, NAME);

    }
    LPVOID pMapViewOfFile = MapViewOfFile(_Notnull_ h, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
    return pMapViewOfFile;
}

ElementalDamages* pElementalDamages = (ElementalDamages*)filemapping(ISLAND_ENVIRONMENT_EXTRA);

HANDLE hEvent = CreateEvent(NULL, FALSE, FALSE, L"Local\\MFCIslandEvent");

void sharedamage(int type ,float damage)
{
    pElementalDamages->type = type;
    switch (type)
    {
    case 0:
        pElementalDamages->Physical.current = damage;
		SetEvent(hEvent);
		break;
    case 1:
        pElementalDamages->Pyro.current = damage;
        SetEvent(hEvent);
        break;
    case 2:
        pElementalDamages->Hydro.current = damage;
        SetEvent(hEvent);
        break;
    case 3:
        pElementalDamages->Dendro.current = damage;
        SetEvent(hEvent);
        break;
    case 4:
        pElementalDamages->Electro.current = damage;
        SetEvent(hEvent);
        break;
    case 5:
        pElementalDamages->Cryo.current = damage;
        SetEvent(hEvent);
        break;
    case 7:
        pElementalDamages->Anemo.current = damage;
        SetEvent(hEvent);
        break;
    case 8:
        pElementalDamages->Geo.current = damage;
        SetEvent(hEvent);
        break;
    default:
        break;
    }
}