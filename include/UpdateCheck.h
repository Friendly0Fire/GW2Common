#pragma once
#include <Common.h>
#include <Singleton.h>
#include <ConfigurationOption.h>
#include <SettingsMenu.h>

class UpdateCheck : public Singleton<UpdateCheck>
{
public:
	UpdateCheck(const std::wstring& repoUrl);

	void CheckForUpdates();

	bool updateAvailable() const { return updateAvailable_; }
	bool updateDismissed() const { return updateDismissed_; }
	void updateDismissed(bool v) { updateDismissed_ = v; }

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

	const std::wstring repoUrl_;
};