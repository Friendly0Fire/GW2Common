#include <SettingsMenu.h>
#include <imgui.h>
#include <ImGuiExtensions.h>
#include <imgui_internal.h>
#include <UpdateCheck.h>

SettingsMenu::SettingsMenu()
	: showKeybind_("show_settings", "Show settings", "__core__", { GetScanCodeFromVirtualKey('M'), Modifier::SHIFT | Modifier::ALT }, false)
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
	showKeybind_.key(GetScanCodeFromVirtualKey('M'));
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
		if(!ImGui::Begin("GW2Radial Options Menu", &isVisible_))
		{
			ImGui::End();
			return;
		}

		isFocused_ = ImGui::IsWindowFocused();
	
		if (!implementers_.empty())
		{
			if(currentTab_ == nullptr)
				currentTab_ = implementers_.front();

			if(ImGui::BeginTabBar("GW2RadialMainTabBar", ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_NoCloseWithMiddleMouseButton | ImGuiTabBarFlags_FittingPolicyScroll)) {
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