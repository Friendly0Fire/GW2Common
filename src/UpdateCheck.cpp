#include "UpdateCheck.h"

#include <sstream>

#include <WinInet.h>

#include "Utility.h"

UpdateCheck::UpdateCheck(const std::wstring& repoId)
    : checkEnabled_("Automatically check for update", "check_for_updates", "Core", true), repoId_(repoId) { }

void UpdateCheck::CheckForUpdates() {
    using json = nlohmann::json;

    const auto currentTime = TimeInMilliseconds();
    if(lastCheckTime_ + checkTimeSpan_ > currentTime)
        return;

    lastCheckTime_ = currentTime;

    if(checkSucceeded_ || checkAttempts_ >= maxCheckAttempts_ || !checkEnabled_.value())
        return;

    checkAttempts_++;

    try {
        auto data = FetchReleaseData();
        if(data.empty())
            return;

        auto j = json::parse(data);

        auto tagName = j["tag_name"].get<std::string>();

        std::string_view tagNameStripped = tagName;
        tagNameStripped = tagNameStripped.substr(1, tagName.find("-pre") - 1);
        std::vector<std::string> tagElements;
        SplitString(tagNameStripped, ".", std::back_inserter(tagElements));

        if (tagName.find("-pre") != std::string::npos)
            tagElements.push_back(tagName.substr(tagName.find("-pre") + 4));

        u64 tagVersion = 0;
        for (u64 i = 0; i < tagElements.size(); ++i)
            tagVersion += static_cast<u64>(atoll(tagElements[i].c_str())) << (16 * (3 - i));

        u64 currentVersion = GetAddonVersion();

        if(currentVersion < tagVersion)
            updateAvailable_ = true;

        checkSucceeded_ = true;
    }
    catch(...) {
        checkSucceeded_ = false;
    }
}

// Retrieving Headers Using a Constant
BOOL DumpHeaders(HINTERNET hRequest) {
    char* lpOutBuffer = NULL;
    DWORD dwSize = 0;

retry:

    // This call will fail on the first pass, because
    // no buffer is allocated.
    if(!HttpQueryInfo(hRequest, HTTP_QUERY_RAW_HEADERS_CRLF, (LPVOID)lpOutBuffer, &dwSize, NULL)) {
        if(GetLastError() == ERROR_HTTP_HEADER_NOT_FOUND) {
            // Code to handle the case where the header isn't available.
            return TRUE;
        }
        else {
            // Check for an insufficient buffer.
            if(GetLastError() == ERROR_INSUFFICIENT_BUFFER) {
                // Allocate the necessary buffer.
                lpOutBuffer = new char[dwSize];

                // Retry the call.
                goto retry;
            }
            else {
                // Error handling code.
                if(lpOutBuffer) {
                    delete[] lpOutBuffer;
                }
                return FALSE;
            }
        }
    }

    if(lpOutBuffer) {
        delete[] lpOutBuffer;
    }

    return TRUE;
}

std::wstring UpdateCheck::apiCheckPartialUrl() const { return std::format(L"repos/{}/releases/latest", repoId_); }

std::wstring UpdateCheck::repoUrl(const std::wstring& end) const { return std::format(L"https://github.com/{}/{}", repoId_, end); }

std::string UpdateCheck::FetchReleaseData() const {
    std::string retVal;
    if(const auto hInternet = InternetOpen(GetAddonNameW(), INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0); hInternet) {
        if(const auto hConnection =
               InternetConnect(hInternet, L"api.github.com", INTERNET_DEFAULT_HTTPS_PORT, L"", L"", INTERNET_SERVICE_HTTP, 0, 0);
           hConnection) {
            const auto hRequest = HttpOpenRequest(hConnection, L"GET", apiCheckPartialUrl().c_str(), nullptr, nullptr, nullptr,
                                                  INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_UI | INTERNET_FLAG_SECURE, 0);
            if(hRequest) {
                if(HttpSendRequest(hRequest, nullptr, 0, nullptr, 0)) {
                    DWORD statusCode = 0;
                    DWORD statusCodeLen = sizeof(statusCode);
                    const auto httpQueryInfoReceived =
                        HttpQueryInfo(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &statusCodeLen, nullptr);
                    if(httpQueryInfoReceived && statusCode == 200) {
                        std::stringstream ss;

                        DWORD availDataLen;
                        while(InternetQueryDataAvailable(hRequest, &availDataLen, 0, 0) && availDataLen > 0) {
                            DWORD readCount;
                            std::vector<char> data;
                            data.resize(availDataLen);
                            InternetReadFile(hRequest, data.data(), DWORD(data.size()), &readCount);

                            ss.write(data.data(), data.size());
                        }

                        retVal = ss.str();
                    }
                }
                InternetCloseHandle(hRequest);
            }
            InternetCloseHandle(hConnection);
        }
        InternetCloseHandle(hInternet);
    }

    return retVal;
}
