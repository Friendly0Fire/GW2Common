#include <ImGuiExtensions.h>
#include <imgui_internal.h>
#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include <Input.h>

ImVec2 operator*(const ImVec2& a, const ImVec2& b)
{
	return ImVec2(a.x * b.x, a.y * b.y);
}
ImVec2 operator*(const ImVec2& a, float b)
{
	return ImVec2(a.x * b, a.y * b);
}
ImVec2 operator/(const ImVec2& a, const ImVec2& b)
{
	return ImVec2(a.x / b.x, a.y / b.y);
}
ImVec2 operator/(const ImVec2& a, float b)
{
	return ImVec2(a.x / b, a.y / b);
}
ImVec2 operator-(const ImVec2& a, const ImVec2& b)
{
	return ImVec2(a.x - b.x, a.y - b.y);
}
ImVec2 operator+(const ImVec2& a, const ImVec2& b)
{
	return ImVec2(a.x + b.x, a.y + b.y);
}

ImVec2 operator*=(ImVec2& a, const ImVec2& b)
{
	a = a * b;
	return a;
}

ImVec2 operator*=(ImVec2& a, float b)
{
	a = a * b;
	return a;
}

ImVec4 operator/(const ImVec4& v, const float f)
{
	return { v.x / f, v.y / f, v.z / f, v.w / f };
}

void ImGuiKeybindInput(Keybind& keybind, Keybind** keybindBeingModified, const char* tooltip)
{
	bool beingModified = *keybindBeingModified == &keybind;
	bool disableSet = !beingModified && *keybindBeingModified != nullptr;
	std::string suffix = "##" + keybind.nickname();

	float windowWidth = ImGui::GetWindowWidth() - ImGuiHelpTooltipSize();

	ImGui::PushItemWidth(windowWidth * 0.45f);

	int popcount = 1;
	if (beingModified)
	{
		popcount = 3;
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(201, 215, 255, 200) / 255.f);
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1));
		ImGui::PushStyleColor(ImGuiCol_TextDisabled, ImVec4(0, 0, 0, 1));
	}
	else
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.15f, 1));

	ImGui::InputText(suffix.c_str(), keybind.keysDisplayString(), keybind.keysDisplayStringSize(), ImGuiInputTextFlags_ReadOnly);

	ImGui::PopItemWidth();

	ImGui::PopStyleColor(popcount);

	ImGui::SameLine();

	if (disableSet) {
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_Button));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_Button));
	}

	if (!beingModified && ImGui::Button(("Set" + suffix).c_str(), ImVec2(windowWidth * 0.1f, 0.f)) && !disableSet)
	{
		if(keybindBeingModified)
			Input::i().CancelRecordInputs();

		*keybindBeingModified = &keybind;
		keybind.keysDisplayString()[0] = '\0';
		Input::i().BeginRecordInputs([&keybind, keybindBeingModified](KeyCombo kc, bool final) {
			if (final) {
				keybind.keyCombo(kc);
				if (*keybindBeingModified == &keybind) {
					*keybindBeingModified = nullptr;
				}
			}
			else
				keybind.UpdateDisplayString(kc);
			
		});
	}
	else if (beingModified && ImGui::Button(("Clear" + suffix).c_str(), ImVec2(windowWidth * 0.1f, 0.f)))
	{
		Input::i().CancelRecordInputs();
		*keybindBeingModified = nullptr;
		keybind.keyCombo({});
	}

	if (disableSet) {
		ImGui::PopStyleVar();
		ImGui::PopStyleColor(2);
	}

	ImGui::SameLine();

	ImGui::PushItemWidth(windowWidth * 0.5f);

	ImGui::Text(keybind.displayName().c_str());

	ImGui::PopItemWidth();

	if(tooltip)
	    ImGuiHelpTooltip(tooltip);
}

void ImGuiTitle(const char * text)
{
	ImGui::Dummy({0, ImGui::GetStyle().ItemSpacing.y * 2 });
	ImGui::PushFont(GetBaseCore().fontBlack());
	ImGui::TextUnformatted(text);
	ImGui::Separator();
	ImGui::PopFont();
	ImGui::Spacing();
}

float ImGuiHelpTooltipSize() {
	ImGui::PushFont(GetBaseCore().fontIcon());
    auto r = ImGui::CalcTextSize(reinterpret_cast<const char*>(ICON_FA_QUESTION_CIRCLE)).x + ImGui::GetStyle().ItemSpacing.x + 1.f;
	ImGui::PopFont();

	return r;
}

void ImGuiHelpTooltip(const char* desc)
{
	ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGuiHelpTooltipSize() - ImGui::GetScrollX() - ImGui::GetStyle().ScrollbarSize);
	ImGui::PushFont(GetBaseCore().fontIcon());
    ImGui::TextDisabled(reinterpret_cast<const char*>(ICON_FA_QUESTION_CIRCLE));
	ImGui::PopFont();
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

bool ImGuiDisabler::disabled_ = false;

ImGuiDisabler::ImGuiDisabler(bool disable, float alpha) {
	if (!disable || disabled_)
		return;

	disabled_ = true;
    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * alpha);
	auto disabledColor = ImGui::GetColorU32(ImGuiCol_TextDisabled);
	ImGui::PushStyleColor(ImGuiCol_FrameBg, disabledColor);
	ImGui::PushStyleColor(ImGuiCol_CheckMark, disabledColor);
	ImGui::PushStyleColor(ImGuiCol_Text, disabledColor);
	ImGui::PushStyleColor(ImGuiCol_Button, disabledColor);
}

ImGuiDisabler::~ImGuiDisabler() {
	if (!disabled_)
		return;

	ImGui::PopStyleColor(4);
    ImGui::PopStyleVar();
    ImGui::PopItemFlag();
	disabled_ = false;
}