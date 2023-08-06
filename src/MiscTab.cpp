#include "MiscTab.h"

#include <imgui.h>

#include "GFXSettings.h"
#include "ImGuiExtensions.h"
#include "MumbleLink.h"
#include "UpdateCheck.h"

MiscTab::MiscTab() { SettingsMenu::i().AddImplementer(this); }

MiscTab::~MiscTab() {
    SettingsMenu::f([&](auto& i) { i.RemoveImplementer(this); });
}
void MiscTab::DrawMenu(Keybind**) {
    ImGuiTitle("General");

    ImGui::Text("Version %s", GetAddonVersionString());

    ImGuiConfigurationWrapper(ImGui::Checkbox, UpdateCheck::i().checkEnabled_);

    if(ImGui::Button("Open Log Window"))
        Log::i().isVisible(true);

    AdditionalGUI();

#ifdef _DEBUG
    const auto& pos = MumbleLink::i().position();
    ImGui::Text("position = %f, %f, %f", pos.x, pos.y, pos.z);

    bool dpiScaling = GFXSettings::i().dpiScaling();
    ImGui::Text(dpiScaling ? "DPI scaling enabled" : "DPI scaling disabled");
#endif
}
