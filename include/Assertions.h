#pragma once
#include <Win.h>

#include <format>

#include <Defs.h>
#include <Log.h>

void CreateMiniDump();

template<typename... Args>
int FormattedMessageBox(const wchar_t* contents, const wchar_t* title, Args&&...args)
{
    wchar_t buf[2048];
    swprintf_s(buf, contents, std::forward<Args>(args)...);

    return MessageBoxW(nullptr, buf, title, MB_ICONERROR | MB_OK);
}

template<typename... Args>
int FormattedMessageBoxTyped(const wchar_t* contents, const wchar_t* title, int type, Args&&...args)
{
    wchar_t buf[2048];
    swprintf_s(buf, contents, std::forward<Args>(args)...);

    return MessageBoxW(nullptr, buf, title, type);
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
    {
        CreateMiniDump();
        int rv = FormattedMessageBoxTyped(L"Assertion failure: \"%s\"!", L"Assertion Failed!", MB_ICONERROR | MB_RETRYCANCEL, testText);
        if(rv == IDCANCEL)
            exit(1);
    }
}

__forceinline void GW2CheckedHResult(HRESULT hr, const wchar_t* testText)
{
    GW2Assert(SUCCEEDED(hr), std::format(L"{} -> 0x{:x}", testText, static_cast<unsigned>(hr)).c_str());
}

#define GW2_CHECKED_HRESULT(call) GW2CheckedHResult(HRESULT(call), L#call)
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