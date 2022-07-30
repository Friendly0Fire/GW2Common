#pragma once
#include <Win.h>

#include <vector>
#include <string>
#include <memory>
#include <span>
#include <wrl.h>
#include <d3d11.h>
#include <fstream>
#include <filesystem>
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>

#include <Log.h>
#include <EnumUtils.h>
#include <Singleton.h>
#include <Defs.h>
#include <BaseCore.h>

#define COM_RELEASE(x) if(x) { x->Release(); x = nullptr; }
#define NULL_COALESCE(a, b) ((a) != nullptr ? (a) : (b))
#define OPT_COALESCE(a, b) ((a) ? (a) : (b))
#define SQUARE(x) ((x) * (x))

template<typename... Args>
void FormattedMessageBox(const wchar_t* contents, const wchar_t* title, Args&&...args)
{
    wchar_t buf[2048];
    swprintf_s(buf, contents, std::forward<Args>(args)...);

    MessageBoxW(nullptr, buf, title, MB_ICONERROR | MB_OK);
}

template<typename... Args>
void CriticalMessageBox(const wchar_t* contents, Args&&...args)
{
    FormattedMessageBox(contents, std::format(L"{} Fatal Error", GetAddonNameW()).c_str(), std::forward<Args>(args)...);
    exit(1);
}

#ifdef _DEBUG
#define GW2_ASSERT(test) GW2Assert(test, L#test)

__forceinline void GW2Assert(bool test, const wchar_t* testText)
{
    if (test)
        return;

    if (IsDebuggerPresent())
        __debugbreak();
    else
        CriticalMessageBox(L"Assertion failure: \"%s\"!", testText);
}

__forceinline void GW2Assert(HRESULT hr, const wchar_t* testText)
{
    GW2Assert(SUCCEEDED(hr), std::format(L"{} -> 0x{:x}", testText, static_cast<unsigned>(hr)).c_str());
}

#define GW2_CHECKED_HRESULT(call) GW2Assert(HRESULT(call), L#call)
#else
#define GW2_ASSERT(test)                             \
    do                                               \
    {                                                \
        if (!(test))                                 \
            LogError("Assertion failed: " #test); \
    }                                                \
    while (0)

#define GW2_CHECKED_HRESULT(call)                    \
    do                                               \
    {                                                \
        if (FAILED(call))                            \
            LogError("Assertion failed: " #call); \
    }                                                \
    while (0)
#endif

using Microsoft::WRL::ComPtr;

using uchar = unsigned char;
using uint = unsigned int;
using ushort = unsigned short;
using tstring = std::basic_string<TCHAR>;
using mstime = unsigned __int64;

using std::tie;

#ifndef HID_USAGE_PAGE_GENERIC
#define HID_USAGE_PAGE_GENERIC         ((USHORT) 0x01)
#endif
#ifndef HID_USAGE_GENERIC_MOUSE
#define HID_USAGE_GENERIC_MOUSE        ((USHORT) 0x02)
#endif

#ifndef M_PI
#define M_PI 3.14159265359
#endif

#ifdef _DEBUG
#define HOT_RELOAD_SHADERS
#endif

struct fVector4
{
    float x;
    float y;
    float z;
    float w;
};

struct fVector3
{
    float x;
    float y;
    float z;
};

struct fVector2
{
    float x;
    float y;
};

struct iVector4
{
    int x;
    int y;
    int z;
    int w;
};

struct iVector3
{
    int x;
    int y;
    int z;
};

struct iVector2
{
    int x;
    int y;
};

struct fMatrix44
{
    float mat[3][3];
};

bool ExceptionHandlerMiniDump(
    struct _EXCEPTION_POINTERS* pExceptionInfo, const char* function, const char* file, int line);

int CRTReportHook(int reportType, char* message, int* returnValue);
