#pragma once
#include <SimpleIni.h>
#include <nlohmann/json.hpp>

#include "Common.h"
#include "Singleton.h"

class ConfigurationFile
{
public:
    virtual void Reload();
    virtual void Save();

    const std::string& lastSaveError() const { return lastSaveError_; }
    bool lastSaveErrorChanged() const { return lastSaveErrorChanged_; }
    void lastSaveErrorChanged(bool v) {
        lastSaveErrorChanged_ = v;
        lastSaveError_.clear();
    }

    const auto& folder() const { return folder_; }

protected:
    virtual const std::wstring_view configFileName() const = 0;

    std::optional<std::filesystem::path> folder_;
    bool readOnly_ = false;
    bool readOnlyWarned_ = false;

    bool lastSaveErrorChanged_ = false;
    std::string lastSaveError_;
};

class INIConfigurationFile : public ConfigurationFile, public Singleton<INIConfigurationFile>
{
    CSimpleIniA ini_;
    static const std::wstring_view ConfigFileName;
    const std::wstring_view configFileName() const override { return ConfigFileName; }
    static void LoadImGuiSettings(const std::wstring& location);
    static void SaveImGuiSettings(const std::wstring& location);

public:
    INIConfigurationFile();
    CSimpleIniA& ini() { return ini_; }

    void Reload() override;
    void Save() override;
    void OnUpdate() const;
};

class JSONConfigurationFile : public ConfigurationFile, public Singleton<JSONConfigurationFile>
{
    nlohmann::json json_;
    static const std::wstring_view ConfigFileName;
    const std::wstring_view configFileName() const override { return ConfigFileName; }

public:
    JSONConfigurationFile();
    nlohmann::json& json() { return json_; }

    void Reload() override;
    void Save() override;
};
