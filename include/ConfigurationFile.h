#pragma once
#include <Common.h>
#include <SimpleIni.h>
#include <Singleton.h>

class ConfigurationFile : public Singleton<ConfigurationFile>
{
public:
	ConfigurationFile();

	void Reload();
	void Save();
	void OnUpdate() const;
	
	const std::string& lastSaveError() const { return lastSaveError_; }
	bool lastSaveErrorChanged() const { return lastSaveErrorChanged_; }
	void lastSaveErrorChanged(bool v) { lastSaveErrorChanged_ = v; lastSaveError_.clear(); }

	CSimpleIniA& ini() { return ini_; }

	const auto& folder() const { return folder_; }

protected:
	static void LoadImGuiSettings(const std::wstring& location);
	static void SaveImGuiSettings(const std::wstring& location);

	CSimpleIniA ini_;
	std::optional<std::filesystem::path> folder_;
	bool readOnly_ = false;
	bool readOnlyWarned_ = false;

	bool lastSaveErrorChanged_ = false;
	std::string lastSaveError_;
};