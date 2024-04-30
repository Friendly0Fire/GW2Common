#pragma warning(disable : 4091)

#include <filesystem>

#include <Win.h>
// ReSharper disable once CppWrongIncludesOrder
#include <DbgHelp.h>

#include "Common.h"
#include "StackWalker.h"
#include "Utility.h"

class StackWalkerGW2 : public StackWalker
{
public:
    using StackWalker::StackWalker;
    void SetModuleName(const std::string& moduleName) { moduleName_ = ToLower(moduleName); }

    [[nodiscard]] bool callstackIncludesAddon() const { return callstackIncludesAddon_; }

protected:
    std::string moduleName_;
    bool callstackIncludesAddon_ = false;

    void OnCallstackEntry(CallstackEntryType eType, CallstackEntry& entry) override {
        if(entry.moduleName[0] != 0) {
            std::string entryModule = ToLower(entry.moduleName);
            if(entryModule.contains(moduleName_))
                callstackIncludesAddon_ = true;
        }

        if(eType != lastEntry && entry.offset != 0) {
            const char* name;

            if(entry.undFullName[0] != 0)
                name = entry.undFullName;
            else if(entry.undName[0] != 0)
                name = entry.undName;
            else if(entry.name[0] != 0)
                name = entry.name;
            else
                name = "(unnamed)";

            if(entry.lineFileName[0] == 0) {
                const char* moduleName = entry.moduleName[0] == 0 ? "(unknown module)" : entry.moduleName;
                LogWarn("{:>32}+{:#08x} {}", moduleName, entry.offset, name);
            }
            else
                LogWarn("{}:{} {}", entry.lineFileName, entry.lineNumber, name);
        }
    }

    void OnOutput(LPCSTR szText) override { LogDebug(szText); }
};

bool ShouldWriteMinidump(_EXCEPTION_POINTERS* pExceptionInfo) {
    StackWalkerGW2 sw { StackWalkerGW2::AfterExcept, StackWalkerGW2::RetrieveSymbol | StackWalkerGW2::RetrieveLine, pExceptionInfo };
    char moduleName[MAX_PATH];
    GetModuleFileNameA(GetBaseCore().dllModule(), moduleName, MAX_PATH);

    std::filesystem::path modulePath { moduleName };
    sw.SetModuleName(modulePath.stem().string().c_str());

    sw.ShowCallstack(GetCurrentThread(), pExceptionInfo->ContextRecord);

    if(!sw.callstackIncludesAddon())
        LogWarn("Exception callstack does not involve current module ({}), preventing minidump...", moduleName);

    return sw.callstackIncludesAddon();
}

// based on dbghelp.h
using MINIDUMPWRITEDUMP = BOOL(WINAPI*)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
                                        PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
                                        PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

void WriteMiniDump(_EXCEPTION_POINTERS* pExceptionInfo) {
    HMODULE hDll = GetModuleHandle(TEXT("DBGHELP.DLL"));
    if(!hDll)
        hDll = LoadLibrary(TEXT("DBGHELP.DLL"));

    if(hDll) {
        if(!ShouldWriteMinidump(pExceptionInfo))
            return;

        auto pDump = reinterpret_cast<MINIDUMPWRITEDUMP>(GetProcAddress(hDll, "MiniDumpWriteDump"));
        if(pDump) {
            std::filesystem::path basePath = std::filesystem::current_path();

            wchar_t szDumpPath[_MAX_PATH];
            wchar_t szDumpPathFirst[_MAX_PATH];

            time_t tNow = time(nullptr);
            tm t;
            localtime_s(&t, &tNow);
            std::wstring fname = std::format(L"{}_%d.%m.%Y_%H.%M.%S", GetAddonNameW());
            wcsftime(szDumpPathFirst, sizeof(szDumpPathFirst), fname.c_str(), &t);

            i32 n = 1;
            do {
                swprintf_s(szDumpPath, L"%s\\%s-%d.dmp", basePath.c_str(), szDumpPathFirst, n);
                n++;
            }
            while(std::filesystem::exists(szDumpPath));

            LogInfo(L"Writing minidump '{}'...", szDumpPath);

            // create the file
            HANDLE hFile = CreateFileW(szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);

            if(hFile != INVALID_HANDLE_VALUE) {
                if(pExceptionInfo) {
                    _MINIDUMP_EXCEPTION_INFORMATION ExInfo;
                    ExInfo.ThreadId = GetCurrentThreadId();
                    ExInfo.ClientPointers = TRUE;
                    ExInfo.ExceptionPointers = pExceptionInfo;
                    pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, nullptr, nullptr);
                }
                else {
                    pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, nullptr, nullptr, nullptr);
                }
                CloseHandle(hFile);
            }
        }
    }
}

BYTE oldSetUnhandledExceptionFilter[5];
LPTOP_LEVEL_EXCEPTION_FILTER previousTopLevelExceptionFilter = nullptr;
void* vectoredExceptionHandlerHandle = nullptr;

LONG WINAPI GW2TopLevelFilter(EXCEPTION_POINTERS* pExceptionInfo) {
    // Special code to ignore a consistent exception in Nvidia's driver
    if(pExceptionInfo->ExceptionRecord->ExceptionCode == 0xe06d7363) {
        if(pExceptionInfo->ExceptionRecord->NumberParameters == 4) {
            auto mbHandle = GetModuleHandleW(L"MessageBus.dll");
            if(mbHandle && mbHandle == reinterpret_cast<HANDLE>(pExceptionInfo->ExceptionRecord->ExceptionInformation[3]))
                return EXCEPTION_CONTINUE_SEARCH;
        }
    }

    if(pExceptionInfo->ExceptionRecord->ExceptionCode >= 0x80000000L) {
        if(HMODULE exceptionModule;
           SUCCEEDED(GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                                        (LPCSTR)pExceptionInfo->ExceptionRecord->ExceptionAddress, &exceptionModule))) {
            std::string exceptionModuleFileName(MAX_PATH, char());

            if(auto sz = GetModuleFileNameA(exceptionModule, exceptionModuleFileName.data(), static_cast<DWORD>(exceptionModuleFileName.size())); sz != 0) {
                exceptionModuleFileName.resize(sz);
                LogWarn("Intercepted exception in module '{}', address {:#08x}, code {:#x}.", exceptionModuleFileName,
                        size_t(pExceptionInfo->ExceptionRecord->ExceptionAddress), pExceptionInfo->ExceptionRecord->ExceptionCode);
            }
        }

        switch(pExceptionInfo->ExceptionRecord->ExceptionCode) {
        case STATUS_FLOAT_DENORMAL_OPERAND:
        case STATUS_FLOAT_DIVIDE_BY_ZERO:
        case STATUS_FLOAT_INEXACT_RESULT:
        case STATUS_FLOAT_INVALID_OPERATION:
        case STATUS_FLOAT_OVERFLOW:
        case STATUS_FLOAT_STACK_CHECK:
        case STATUS_FLOAT_UNDERFLOW:
        case STATUS_FLOAT_MULTIPLE_FAULTS:
        case STATUS_FLOAT_MULTIPLE_TRAPS:
        case STATUS_INTEGER_DIVIDE_BY_ZERO:
            break;
        default:
            WriteMiniDump(pExceptionInfo);
        }
    }

    // Pass exception on anyway, we only wanted the minidump
    return EXCEPTION_CONTINUE_SEARCH;
}

i32 FilterExceptionAndContinueExecution(EXCEPTION_POINTERS* exceptionPointers) {
    WriteMiniDump(exceptionPointers);
    // With the following return statement
    // Execution continues after intentionally created access violation exception
    return EXCEPTION_EXECUTE_HANDLER;
}

void CreateMiniDump() {
    __try {
        i32* createException = nullptr;
        *createException = 0x42;
    }
    __except(FilterExceptionAndContinueExecution(GetExceptionInformation())) {
        // Use filter exception to generate minidump with the "exception record"
        // Then continue execution
    }
}

i32 CRTReportHook(i32 reportType, char* message, i32* returnValue) {
    const char* reportString;
    switch(reportType) {
    case _CRT_WARN:
        reportString = "Warning";
        break;
    case _CRT_ERROR:
        reportString = "Error";
        break;
    case _CRT_ASSERT:
        reportString = "Assertion";
        break;
    default:
        reportString = "Unknown Event";
        break;
    }
    LogWarn("CRT {}: {}", reportString, message);

    CreateMiniDump();

    if(returnValue)
        *returnValue = 0;

    return TRUE;
}
