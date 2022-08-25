#include <Utility.h>
#include <winuser.h>
#include <fstream>
#include <sstream>
#include <shellapi.h>
#include <Knownfolders.h>
#include <ShlObj.h>

std::string utf8_encode(const std::wstring &wstr)
{
    if( wstr.empty() ) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo( size_needed, 0 );
    WideCharToMultiByte                  (CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

std::wstring utf8_decode(const std::string &str)
{
    if( str.empty() ) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo( size_needed, 0 );
    MultiByteToWideChar                  (CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

void SplitFilename(const tstring& str, tstring* folder, tstring* file)
{
	const auto found = str.find_last_of(TEXT("/\\"));
	if (folder) *folder = str.substr(0, found);
	if (file) *file = str.substr(found + 1);
}

mstime TimeInMilliseconds()
{
	mstime iCount;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&iCount));
	mstime iFreq;
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&iFreq));
	return 1000 * iCount / iFreq;
}

std::span<byte> LoadResource(HMODULE dll, UINT resId)
{
    const auto res = FindResource(dll, MAKEINTRESOURCE(resId), RT_RCDATA);
	if(res)
	{
        const auto handle = LoadResource(dll, res);
		if(handle)
		{
			size_t sz = SizeofResource(dll, res);
			void* ptr = LockResource(handle);

			return std::span<byte>((byte*)ptr, sz);
		}
	}

    return {};
}

uint RoundUpToMultipleOf(uint numToRound, uint multiple)
{
    if (multiple == 0)
        return numToRound;

    uint remainder = numToRound % multiple;
    if (remainder == 0)
        return numToRound;

    return numToRound + multiple - remainder;
}

std::filesystem::path GetGameFolder()
{
    wchar_t exeFullPath[MAX_PATH];
    GetModuleFileNameW(nullptr, exeFullPath, MAX_PATH);
    std::wstring exeFolder;
    SplitFilename(exeFullPath, &exeFolder, nullptr);

#if _DEBUG
    Log::i().Print(Severity::Debug, L"Game folder path: {}", exeFolder.c_str());
#endif

    return exeFolder;
}

std::optional<std::filesystem::path> GetDocumentsFolder()
{
    wchar_t* myDocuments;
    if (FAILED(SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_CREATE, nullptr, &myDocuments)))
        return std::nullopt;

    std::filesystem::path documentsGW2 = myDocuments;
    documentsGW2 /= L"GUILD WARS 2";

#if _DEBUG
    Log::i().Print(Severity::Debug, L"Documents folder path: {}", documentsGW2.c_str());
#endif

    if (std::filesystem::is_directory(documentsGW2))
        return documentsGW2;

    if (SUCCEEDED(SHCreateDirectoryExW(nullptr, documentsGW2.c_str(), nullptr)))
        return documentsGW2;

    Log::i().Print(Severity::Warn, L"Could not open or create documents folder '{}'.", documentsGW2.wstring());

    return std::nullopt;
}

std::optional<std::filesystem::path> GetAddonFolder()
{
    auto folder = (GetGameFolder() / L"addons" / ToLower(GetAddonNameW())).make_preferred();

    LogDebug(L"Addons folder path: {}", folder.c_str());

    if (std::filesystem::is_directory(folder))
        return folder;

    if (SUCCEEDED(SHCreateDirectoryExW(nullptr, folder.c_str(), nullptr)))
        return folder;

    LogWarn(L"Could not open or create configuration folder '{}'.", folder.wstring());

    auto docs = GetDocumentsFolder();
    if (!docs) {
        LogError(L"Could not locate Documents folder (fallback).");
        return std::nullopt;
    }

    folder = (*docs / L"addons" / ToLower(GetAddonNameW())).make_preferred();

    if (std::filesystem::is_directory(folder))
        return folder;

    if (SUCCEEDED(SHCreateDirectoryExW(nullptr, folder.c_str(), nullptr)))
        return folder;

    LogError(L"Could not open or create configuration folder '{}'.", folder.wstring());

    return std::nullopt;
}

std::span<const wchar_t*> GetCommandLineArgs() {
    auto cmdLine = GetCommandLineW();
    int num = 0;
    wchar_t** args = CommandLineToArgvW(cmdLine, &num);

    return std::span { const_cast<const wchar_t**>(args), size_t(num) };
}

const wchar_t* GetCommandLineArg(const wchar_t* name) {
    bool saveNextArg = false;
    for (auto* arg : GetCommandLineArgs()) {
        if (saveNextArg) {
            return arg;
        }

        auto l = wcslen(arg);
        if (l > 1 && (arg[0] == L'/' || arg[0] == L'-') && _wcsnicmp(name, &arg[1], 6) == 0) {
            if (l > 7 && arg[7] == L':') {
                return &arg[8];
                break;
            }
            else
                saveNextArg = true;
        }
    }

    return nullptr;
}


typedef LONG NTSTATUS, *PNTSTATUS;
#define STATUS_SUCCESS (0x00000000)

typedef NTSTATUS (WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

RTL_OSVERSIONINFOW GetOSVersion()
{
    if (HMODULE hMod = GetModuleHandleW(L"ntdll.dll")) {
        auto fxPtr = reinterpret_cast<RtlGetVersionPtr>(::GetProcAddress(hMod, "RtlGetVersion"));
        if (fxPtr != nullptr) {
            RTL_OSVERSIONINFOW rovi  = { 0 };
            rovi.dwOSVersionInfoSize = sizeof(rovi);
            if ( STATUS_SUCCESS == fxPtr(&rovi) ) {
                return rovi;
            }
        }
    }
    RTL_OSVERSIONINFOW rovi = { 0 };
    return rovi;
}

#include <intrin.h>

std::string GetCpuInfo()
{
    // 4 is essentially hardcoded due to the __cpuid function requirements.
    // NOTE: Results are limited to whatever the sizeof(int) * 4 is...
    std::array<int, 4> integerBuffer       = {};
    constexpr size_t   sizeofIntegerBuffer = sizeof(int) * integerBuffer.size();

    std::array<char, 64> charBuffer = {};

    // The information you wanna query __cpuid for.
    // https://docs.microsoft.com/en-us/cpp/intrinsics/cpuid-cpuidex?view=vs-2019
    constexpr std::array<int, 3> functionIds = {
        // Manufacturer
        //  EX: "Intel(R) Core(TM"
        0x8000'0002,
        // Model
        //  EX: ") i7-8700K CPU @"
        0x8000'0003,
        // Clockspeed
        //  EX: " 3.70GHz"
        0x8000'0004
    };

    std::string cpu;

    for (int id : functionIds)
    {
        // Get the data for the current ID.
        __cpuid(integerBuffer.data(), id);
        
        // Copy the raw data from the integer buffer into the character buffer
        std::memcpy(charBuffer.data(), integerBuffer.data(), sizeofIntegerBuffer);

        // Copy that data into a std::string
        cpu += std::string(charBuffer.data());
    }

    return cpu;
}
