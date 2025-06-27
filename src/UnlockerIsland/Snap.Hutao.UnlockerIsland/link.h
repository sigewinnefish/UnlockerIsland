#pragma once

#include <windows.h>
#include <stdio.h>

#define LogA LogW
#define LogW sendlog

inline HANDLE hPipe;

inline TCHAR  chBuf[1024];
inline BOOL   fSuccess = FALSE;
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

    fSuccess = WriteFile(
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