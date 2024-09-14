#pragma once
#include <format>

namespace Detail
{
int32_t ShowMessageBox(const wchar_t* msg, const wchar_t* title, int32_t type = 16 /*MB_ICONERROR | MB_OK*/);
const wchar_t* AssertionsGetAddonName();
void AssertionLog(const char*);
bool IsDebuggerPresent();
}

void CreateMiniDump();

template<typename... Args>
int32_t FormattedMessageBox(const wchar_t* contents, const wchar_t* title, Args&&... args) {
    wchar_t buf[2048];
    swprintf_s(buf, contents, std::forward<Args>(args)...);

    return Detail::ShowMessageBox(buf, title);
}

template<typename... Args>
int32_t FormattedMessageBoxTyped(const wchar_t* contents, const wchar_t* title, int32_t type, Args&&... args) {
    wchar_t buf[2048];
    swprintf_s(buf, contents, std::forward<Args>(args)...);

    return Detail::ShowMessageBox(buf, title, type);
}

template<typename... Args>
void CriticalMessageBox(const wchar_t* contents, Args&&... args) {
    FormattedMessageBox(contents, std::format(L"{} Fatal Error", Detail::AssertionsGetAddonName()).c_str(), std::forward<Args>(args)...);
    exit(1);
}

#ifdef _DEBUG
#define GW2_ASSERT(test) GW2Assert(test, L#test)

__forceinline void GW2Assert(bool test, const wchar_t* testText) {
    if(test)
        return;

    if(Detail::IsDebuggerPresent())
        __debugbreak();
    else {
        CreateMiniDump();
        int32_t rv = FormattedMessageBoxTyped(L"Assertion failure: \"%s\"!", L"Assertion Failed!", 21 /*MB_ICONERROR | MB_RETRYCANCEL*/, testText);
        if(rv == 2 /*IDCANCEL*/)
            std::terminate();
    }
}

__forceinline void GW2CheckedHResult(long hr, const wchar_t* testText) {
    GW2Assert(hr >= 0, std::format(L"{} -> 0x{:x}", testText, static_cast<unsigned>(hr)).c_str());
}

#define GW2_CHECKED_HRESULT(call) GW2CheckedHResult(HRESULT(call), L#call)
#else
#define GW2_ASSERT(test)                                        \
    do {                                                        \
        if(!(test))                                             \
            ::Detail::AssertionLog("Assertion failed: " #test); \
    }                                                           \
    while(0)

#define GW2_CHECKED_HRESULT(call)                               \
    do {                                                        \
        if(call < 0)                                            \
            ::Detail::AssertionLog("Assertion failed: " #call); \
    }                                                           \
    while(0)
#endif
