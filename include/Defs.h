#pragma once

struct InputLanguageChangeListener {
    virtual void OnInputLanguageChange() = 0;
};

struct ImFont;
struct ImGuiFonts
{
	virtual ImFont* fontBlack() = 0;
	virtual ImFont* fontIcon() = 0;
	virtual ImFont* fontMono() = 0;
};

const char* GetAddonName();
const wchar_t* GetAddonNameW();