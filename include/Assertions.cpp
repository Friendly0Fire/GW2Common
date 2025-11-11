#include "Assertions.h"

#include "Common.h"

namespace Detail
{
int32_t ShowMessageBox(const wchar_t* msg, const wchar_t* title, int32_t type) {
    return MessageBoxW(nullptr, msg, title, type);
}

std::wstring_view AssertionsGetAddonName() {
    return AddonNameW;
}

void AssertionLog(const char* text) {
    LogError(text);
}

bool IsDebuggerPresent() {
    return ::IsDebuggerPresent();
}
}
