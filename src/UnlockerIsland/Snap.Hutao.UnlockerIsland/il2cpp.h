#pragma once

#include "framework.h"

typedef struct tagIl2CppObject
{
    LPVOID klass;
    LPVOID monitor;
} Il2CppObject;

typedef struct tagIl2CppArraySize
{
    Il2CppObject object;
    LPVOID bounds;
    SIZE_T max_length;
    UCHAR vector[32];
} Il2CppArraySize;

typedef struct tagIl2CppString
{
    Il2CppObject object;
    INT32 length;
    WCHAR chars[32];
} Il2CppString;

typedef struct tagIl2CppExceptionWrapper
{
    LPVOID ex;
} Il2CppExceptionWrapper;