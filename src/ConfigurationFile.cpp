#include "ConfigurationFile.h"

#include <filesystem>
#include <sstream>

#include <imgui.h>
#include <tchar.h>

#include "Utility.h"

const wchar_t* const INIConfigurationFile::ConfigFileName = L"config.ini";
const wchar_t* const JSONConfigurationFile::ConfigFileName = L"config.json";

const wchar_t* g_imguiConfigName = L"imgui_config.ini";

INIConfigurationFile::INIConfigurationFile()
{
	Reload();
}

JSONConfigurationFile::JSONConfigurationFile()
{
	Reload();
}

void ConfigurationFile::Reload()
{
	LogDebug("Reloading configuration files");
	readOnlyWarned_ = false;

	auto folder = GetAddonFolder();
	if (!folder)
	{
		LogWarn("Could not find addon folder");
		folder_ = std::nullopt;
		readOnly_ = false;
		return;
	}

	auto cfgFile = *folder / configFileName();
	FILE* fp = nullptr;
	if (_wfopen_s(&fp, cfgFile.c_str(), L"ab") != 0)
	{
		LogWarn(L"Could not write to config file '{}'", cfgFile.wstring());
		if (_wfopen_s(&fp, cfgFile.c_str(), L"rb") != 0)
		{
			LogError(L"Could read config file '{}'", cfgFile.wstring());
			folder_ = std::nullopt;
			readOnly_ = false;
			return;
		}
		else if(fp)
			fclose(fp);
		readOnly_ = true;
	}
	else if (fp)
		fclose(fp);
	folder_ = folder;

	LogInfo(L"Config folder is now '{}'", folder_->wstring());
}

void INIConfigurationFile::Reload()
{
	ConfigurationFile::Reload();

	if (folder_)
	{
		LoadImGuiSettings(*folder_ / g_imguiConfigName);

		ini_.SetUnicode();
		ini_.LoadFile((*folder_ / ConfigFileName).c_str());
	}
}

void JSONConfigurationFile::Reload()
{
	ConfigurationFile::Reload();

	if (folder_)
	{
		json_.clear();
		std::ifstream cfg(*folder_ / ConfigFileName);
		if (cfg.good())
		{
			std::stringstream ss;
			ss << cfg.rdbuf();
			auto str = ss.str();
			if (!str.empty())
				json_ = nlohmann::json::parse(str, nullptr, false, true);
		}
	}
}

void ConfigurationFile::Save()
{
	if (!folder_)
	{
		const auto prevSaveError = lastSaveError_;
		lastSaveError_ = "No configuration folder could be located.";
		lastSaveErrorChanged_ |= prevSaveError != lastSaveError_;
		return;
	}

	if (readOnly_)
	{
		if (!readOnlyWarned_)
		{
			LogWarn("Configuration files in read-only mode, changes will not be saved!");
			readOnlyWarned_ = true;
		}
		return;
	}
}

void INIConfigurationFile::Save()
{
	ConfigurationFile::Save();
	if (!folder_ || readOnly_)
		return;

	const auto r = ini_.SaveFile((*folder_ / ConfigFileName).c_str());

	if (r < 0)
	{
		const auto prevSaveError = lastSaveError_;
		switch (r)
		{
		case SI_FAIL:
			lastSaveError_ = "Unknown error";
			break;
		case SI_NOMEM:
			lastSaveError_ = "Out of memory";
			break;
		case SI_FILE:
			char buf[1024];
			if (strerror_s(buf, errno) == 0) {
				buf[1023] = '\0';
				lastSaveError_.assign(buf);
			}
			else
				lastSaveError_ = "Unknown error";
			break;
		default:
			lastSaveError_ = "Unknown error";
		}

		lastSaveErrorChanged_ |= prevSaveError != lastSaveError_;
	}
	else if (!lastSaveError_.empty())
	{
		lastSaveError_.clear();
		lastSaveErrorChanged_ = true;
	}
}

void JSONConfigurationFile::Save()
{
	ConfigurationFile::Save();
	if (!folder_ || readOnly_ || json_.empty())
		return;

	std::ofstream cfg(*folder_ / ConfigFileName, std::ofstream::trunc | std::ofstream::out);
	if(cfg.good())
		cfg << std::setw(4) << json_ << std::endl;
	else
	{
		const auto prevSaveError = lastSaveError_;
		if (cfg.fail())
			lastSaveError_ = "Logical error";
		else if(cfg.bad())
			lastSaveError_ = "File I/O error";
		else
			lastSaveError_ = "Unknown error";

		lastSaveErrorChanged_ |= prevSaveError != lastSaveError_;
	}
}

void INIConfigurationFile::OnUpdate() const
{
	if(folder_)
		SaveImGuiSettings(*folder_ / g_imguiConfigName);
}

void INIConfigurationFile::LoadImGuiSettings(const std::wstring & location)
{
	FILE *fp = nullptr;
	if(_wfopen_s(&fp, location.c_str(), L"rt, ccs=UTF-8") != 0 || fp == nullptr)
		return;

	fseek(fp, 0, SEEK_END);
	const auto num = ftell(fp);

	std::wstring contents;
	contents.resize(size_t(num) + 1);

	fseek(fp, 0, SEEK_SET);
	fread_s(contents.data(), contents.size() * sizeof(wchar_t), sizeof(wchar_t), num, fp);
	fclose(fp);

	auto utf8 = utf8_encode(contents);
	ImGui::LoadIniSettingsFromMemory(utf8.c_str(), utf8.size());
}

void INIConfigurationFile::SaveImGuiSettings(const std::wstring & location)
{
	auto& imio = ImGui::GetIO();
	if(!imio.WantSaveIniSettings)
		return;

	FILE *fp = nullptr;
	if(_wfopen_s(&fp, location.c_str(), L"wt, ccs=UTF-8") != 0 || fp == nullptr)
		return;

	size_t num;
	const auto contentChar = ImGui::SaveIniSettingsToMemory(&num);
	const std::string contents(contentChar, num);
	const auto unicode = utf8_decode(contents);

	fwrite(unicode.data(), sizeof(wchar_t), num, fp);
	fclose(fp);
	
	imio.WantSaveIniSettings = false;
}
