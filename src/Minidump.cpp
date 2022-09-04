#pragma warning(disable : 4091)

#include <Common.h>
#include <DbgHelp.h>
#include <filesystem>

// based on dbghelp.h
using MINIDUMPWRITEDUMP = BOOL(WINAPI *)(HANDLE                            hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
                                         PMINIDUMP_EXCEPTION_INFORMATION   ExceptionParam,
                                         PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
                                         PMINIDUMP_CALLBACK_INFORMATION    CallbackParam
    );

void WriteMiniDump(struct _EXCEPTION_POINTERS* pExceptionInfo)
{
    HMODULE hDll = ::LoadLibrary(TEXT("DBGHELP.DLL"));
    if (hDll)
    {
        auto pDump = reinterpret_cast<MINIDUMPWRITEDUMP>(GetProcAddress(hDll, "MiniDumpWriteDump"));
        if (pDump)
        {
            std::filesystem::path basePath = std::filesystem::current_path();

            wchar_t szDumpPath[_MAX_PATH];
            wchar_t szDumpPathFirst[_MAX_PATH];

            time_t tNow = time(nullptr);
            tm     t;
            localtime_s(&t, &tNow);
            std::wstring fname = std::format(L"{}_%d.%m.%Y_%H.%M.%S", GetAddonNameW());
            wcsftime(szDumpPathFirst, sizeof(szDumpPathFirst), fname.c_str(), &t);

            int n = 1;
            do
            {
                swprintf_s(szDumpPath, L"%s\\%s-%d.dmp", basePath.c_str(), szDumpPathFirst, n);
                n++;
            }
            while (std::filesystem::exists(szDumpPath));

            LogInfo(L"Writing minidump '{}'...", szDumpPath);

            // create the file
            HANDLE hFile = CreateFileW(szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS,
                                       FILE_ATTRIBUTE_NORMAL, nullptr);

            if (hFile != INVALID_HANDLE_VALUE)
            {
                if (pExceptionInfo)
                {
                    _MINIDUMP_EXCEPTION_INFORMATION ExInfo;
                    ExInfo.ThreadId          = GetCurrentThreadId();
                    ExInfo.ClientPointers    = TRUE;
                    ExInfo.ExceptionPointers = pExceptionInfo;
                    pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, nullptr, nullptr);
                }
                else
                {
                    pDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, nullptr, nullptr, nullptr);
                }
                CloseHandle(hFile);
            }
        }
    }
}

BYTE oldSetUnhandledExceptionFilter[5];
LPTOP_LEVEL_EXCEPTION_FILTER previousTopLevelExceptionFilter = nullptr;

LONG WINAPI GW2TopLevelFilter(EXCEPTION_POINTERS* pExceptionInfo)
{
    // Special code to ignore a consistent exception in Nvidia's driver
    if (pExceptionInfo->ExceptionRecord->ExceptionCode == 0xe06d7363)
    {
        if (pExceptionInfo->ExceptionRecord->NumberParameters == 4)
        {
            auto mbHandle = GetModuleHandleW(L"MessageBus.dll");
            if (mbHandle && mbHandle == reinterpret_cast<HANDLE>(pExceptionInfo->ExceptionRecord->ExceptionInformation[3]))
                return EXCEPTION_CONTINUE_SEARCH;
        }
    }

    if (pExceptionInfo->ExceptionRecord->ExceptionCode >= 0x80000000L)
    {
        if(HMODULE exceptionModule; SUCCEEDED(GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (LPCSTR)pExceptionInfo->ExceptionRecord->ExceptionAddress, &exceptionModule)))
        {
            std::string exceptionModuleFileName(MAX_PATH, char());

            if(auto sz = GetModuleFileNameA(exceptionModule, exceptionModuleFileName.data(), exceptionModuleFileName.size()); sz != 0)
            {
                exceptionModuleFileName.resize(sz);
                LogWarn("Intercepted exception in module '{}', address {:#x}, code {:#x}."
                    , exceptionModuleFileName
                    , size_t(pExceptionInfo->ExceptionRecord->ExceptionAddress)
                    , pExceptionInfo->ExceptionRecord->ExceptionCode);
                ranges::transform(exceptionModuleFileName, exceptionModuleFileName.begin(), [](const char c) { return std::tolower(uint8_t(c)); });
                if(exceptionModuleFileName.contains("arcdps") || exceptionModuleFileName.contains("gw2") || exceptionModuleFileName.contains("d3d11") || exceptionModuleFileName.contains("dxgi"))
                    return EXCEPTION_CONTINUE_SEARCH;
            }
        }

        switch (pExceptionInfo->ExceptionRecord->ExceptionCode)
        {
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

int FilterExceptionAndContinueExecution(EXCEPTION_POINTERS * exceptionPointers)
{
    WriteMiniDump(exceptionPointers);
    // With the following return statement
    // Execution continues after intentionally created access violation exception
    return EXCEPTION_EXECUTE_HANDLER;
}

void CreateMiniDump()
{
    __try
    {
        int* createException = nullptr;
        *createException = 0x42;
    }
    __except (FilterExceptionAndContinueExecution(GetExceptionInformation()))
    {
        // Use filter exception to generate minidump with the "exception record"
        // Then continue execution
    }
}

int CRTReportHook(int reportType, char* message, int* returnValue)
{
    const char* reportString;
    switch (reportType)
    {
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