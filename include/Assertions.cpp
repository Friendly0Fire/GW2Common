#include "Assertions.h"

#include "Common.h"

namespace Detail
{
int32_t ShowMessageBox(const wchar_t* msg, const wchar_t* title, int32_t type) {
    return MessageBoxW(nullptr, msg, title, type);
}

const wchar_t* AssertionsGetAddonName() {
    return GetAddonNameW();
}

void AssertionLog(const char* text) {
    LogError(text);
}

bool IsDebuggerPresent() {
    return ::IsDebuggerPresent();
}
}
