#include <SettingsMenu.h>
#include <imgui.h>
#include <ImGuiExtensions.h>
#include <imgui_internal.h>
#include <UpdateCheck.h>

SettingsMenu::SettingsMenu()
	: showKeybind_("show_settings", "Show settings", "__core__", GetSettingsKeyCombo(), false)
{
	showKeybind_.callback([&](Activated a) {
		if (a) {
			isVisible_ = true;
			Input::i().ClearActive();
		}
		return true;
	});
}

void SettingsMenu::OnInputLanguageChange()
{
	showKeybind_.key(GetSettingsKeyCombo().key());
}

void SettingsMenu::Draw()
{
	isFocused_ = false;

	if(isVisible_)
		Input::i().BlockKeybinds(1);
	else
		Input::i().UnblockKeybinds(1);

	if (isVisible_)
	{
		ImGui::SetNextWindowSize({ 750, 600 }, ImGuiCond_FirstUseEver);
		if(!ImGui::Begin(std::format("{} Options Menu", GetAddonName()).c_str(), &isVisible_, ImGuiWindowFlags_AlwaysVerticalScrollbar))
		{
			ImGui::End();
			return;
		}

		isFocused_ = ImGui::IsWindowFocused();
	
		if (!implementers_.empty())
		{
			if(currentTab_ == nullptr)
				currentTab_ = implementers_.front();

			if(ImGui::BeginTabBar("GW2AddonMainTabBar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_FittingPolicyScroll)) {
			    for (const auto& i : implementers_)
			    {
					if(!i->visible())
						continue;

				    if(ImGui::BeginTabItem(i->GetTabName(), nullptr, 0)) {
					    currentTab_ = i;
					    i->DrawMenu(&currentEditedKeybind_);
				        ImGui::EndTabItem();
				    }
			    }
				
			    ImGui::EndTabBar();
			}
		}

		ImGui::End();
	}
}