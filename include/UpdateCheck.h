#pragma once
#include "Common.h"
#include "ConfigurationOption.h"
#include "SettingsMenu.h"
#include "Singleton.h"

class UpdateCheck : public Singleton<UpdateCheck>
{
public:
    UpdateCheck(const std::wstring& repoId);

    void CheckForUpdates();

    bool updateAvailable() const { return updateAvailable_; }
    bool updateDismissed() const { return updateDismissed_; }
    void updateDismissed(bool v) { updateDismissed_ = v; }

    const std::wstring& repoId() const { return repoId_; }
    std::wstring apiCheckPartialUrl() const;
    std::wstring repoUrl(const std::wstring& end = L"") const;

protected:
    std::string FetchReleaseData() const;

    ConfigurationOption<bool> checkEnabled_;

    bool checkSucceeded_ = false;
    bool updateAvailable_ = false;
    bool updateDismissed_ = false;
    int checkAttempts_ = 0;
    const int maxCheckAttempts_ = 10;
    mstime lastCheckTime_ = 0;
    const mstime checkTimeSpan_ = 1000;

    friend class MiscTab;

    const std::wstring repoId_;
};
