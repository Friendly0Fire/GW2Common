#pragma once
#include <Common.h>
#include <Singleton.h>
#include <filesystem>

class GFXSettings : public Singleton<GFXSettings> {
	std::filesystem::path filePath_;
	std::filesystem::file_time_type lastFileTime_;

	std::map<std::string, std::string> settings_;

	bool dpiScaling_ = false;
public:
	GFXSettings();

	void Reload();

	void OnUpdate();

    [[nodiscard]] bool dpiScaling() const { return dpiScaling_; }
};