#pragma once
#include <SettingsMenu.h>
#include <Singleton.h>

class MiscTab : public SettingsMenu::Implementer, public Singleton<MiscTab>
{
public:
	MiscTab();
	~MiscTab();

	virtual void AdditionalGUI() {}

	const char * GetTabName() const override { return "Misc"; }
	void DrawMenu(Keybind**) override;
};