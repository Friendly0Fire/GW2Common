#include <ImGuiExtensions.h>
#include <imgui_internal.h>
#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include <Input.h>

ImVec2 operator*(const ImVec2& a, const ImVec2& b)
{
	return { a.x * b.x, a.y * b.y };
}
ImVec2 operator*(const ImVec2& a, float b)
{
	return { a.x * b, a.y * b };
}
ImVec2 operator/(const ImVec2& a, const ImVec2& b)
{
	return { a.x / b.x, a.y / b.y };
}
ImVec2 operator/(const ImVec2& a, float b)
{
	return { a.x / b, a.y / b };
}
ImVec2 operator-(const ImVec2& a, const ImVec2& b)
{
	return { a.x - b.x, a.y - b.y };
}
ImVec2 operator+(const ImVec2& a, const ImVec2& b)
{
	return { a.x + b.x, a.y + b.y };
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

ImVec4 operator*(const ImVec4& a, float b)
{
	return { a.x * b, a.y * b, a.z * b, a.w * b };
}

ImVec4 operator*(const ImVec4& a, const ImVec4& b)
{
	return { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
}

ImVec4 operator/(const ImVec4& v, const float f)
{
	return { v.x / f, v.y / f, v.z / f, v.w / f };
}

ImVec4 operator*=(ImVec4& a, float b)
{
	a = a * b;
	return a;
}

ImVec4 operator*=(ImVec4& a, const ImVec4& b)
{
	a = a * b;
	return a;
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

void ImGuiTitle(const char * text, float scale)
{
	float sc = ImGui::GetCurrentWindow()->FontWindowScale;
	ImGui::Dummy({0, ImGui::GetStyle().ItemSpacing.y * 2 });
	ImGui::PushFont(GetBaseCore().fontBlack());
	ImGui::SetWindowFontScale(scale);
	ImGui::TextUnformatted(text);
	ImGui::SetWindowFontScale(sc);
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

void ImGuiHelpTooltip(std::initializer_list<std::pair<ImGuiHelpTooltipElementType, const char*>> desc, float scale, bool includeScrollbars)
{
	float sc = ImGui::GetCurrentWindow()->FontWindowScale;
	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGuiHelpTooltipSize() - (includeScrollbars ? (ImGui::GetScrollX() + ImGui::GetStyle().ScrollbarSize) : 0.f));
	ImGui::PushFont(GetBaseCore().fontIcon());
    ImGui::TextDisabled(reinterpret_cast<const char*>(ICON_FA_QUESTION_CIRCLE));
	ImGui::PopFont();
	ImGui::SetWindowFontScale(sc);
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        for(const auto& [t, d] : desc)
        {
            switch(t)
            {
            default:
            case ImGuiHelpTooltipElementType::DEFAULT:
                ImGui::TextUnformatted(d);
                break;
                case ImGuiHelpTooltipElementType::BULLET:
                ImGui::Bullet();
                ImGui::TextUnformatted(d);
                break;
            }
        }
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

float ImGuiCloseSize() {
	ImGui::PushFont(GetBaseCore().fontIcon());
	auto r = ImGui::CalcTextSize(reinterpret_cast<const char*>(ICON_FA_TIMES)).x + ImGui::GetStyle().ItemSpacing.x + 1.f;
	ImGui::PopFont();

	return r;
}

bool ImGuiClose(const char* id, float scale, bool includeScrollbars)
{
	float sc = ImGui::GetCurrentWindow()->FontWindowScale;
	ImGui::SameLine();
	ImGui::SetCursorPosX(ImGui::GetWindowWidth() - ImGuiHelpTooltipSize() - (includeScrollbars ? (ImGui::GetScrollX() + ImGui::GetStyle().ScrollbarSize) : 0.f));
	ImGui::PushFont(GetBaseCore().fontIcon());
	ImGui::SetWindowFontScale(scale);
	bool r = ImGui::Button(std::format("{}##{}", ICON_FA_TIMES, id).c_str());
	ImGui::PopFont();
	ImGui::SetWindowFontScale(sc);
	return r;
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

thread_local int s_max_timeline_value;

bool ImGuiBeginTimeline(const char* str_id, int max_value)
{
	s_max_timeline_value = max_value;
	return ImGui::BeginChild(str_id);
}


static constexpr float TIMELINE_RADIUS = 6.f;

bool ImGuiTimelineEvent(const char* str_id, ImTimelineRange& values, bool* selected)
{
    using namespace ImGui;
	ImGuiWindow* win = GetCurrentWindow();
	const ImU32 inactive_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Button]);
	const ImU32 active_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_ButtonHovered]);
	const ImU32 line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_SliderGrabActive]);
	bool changed = false;
	ImVec2 cursor_pos = win->DC.CursorPos;
	
	for (int i = 0; i < 2; ++i)
	{
        if(values[i] < 0 || values[i] > s_max_timeline_value)
            continue;

		ImVec2 pos = cursor_pos;
		pos.x += win->Size.x * float(values[i]) / float(s_max_timeline_value) + TIMELINE_RADIUS;
		pos.y += TIMELINE_RADIUS;

		SetCursorScreenPos(pos - ImVec2(TIMELINE_RADIUS, TIMELINE_RADIUS));
		PushID(i);
		InvisibleButton(str_id, ImVec2(2 * TIMELINE_RADIUS, 2 * TIMELINE_RADIUS));
		if (IsItemActive() || IsItemHovered())
		{
			ImGui::SetTooltip("%d", values[i]);
			ImVec2 a(pos.x, GetWindowContentRegionMin().y + win->Pos.y);
			ImVec2 b(pos.x, GetWindowContentRegionMax().y + win->Pos.y);
			win->DrawList->AddLine(a, b, line_color);
		}
		if (IsItemActive() && IsMouseDragging(ImGuiMouseButton_Left))
		{
			values[i] = int((GetIO().MousePos.x - cursor_pos.x) / win->Size.x * float(s_max_timeline_value));
			changed = true;
		}
		PopID();
		win->DrawList->AddCircleFilled(
			pos, TIMELINE_RADIUS, IsItemActive() || IsItemHovered() ? active_color : inactive_color);
	}
	
	ImVec2 start = cursor_pos;
	start.x += win->Size.x * float(std::max(0, values[0])) / float(s_max_timeline_value) + 2 * TIMELINE_RADIUS;
	start.y += TIMELINE_RADIUS * 0.5f;
	ImVec2 end = start + ImVec2(win->Size.x * float(std::min(s_max_timeline_value, values[1]) - std::max(0, values[0])) / float(s_max_timeline_value) - 2 * TIMELINE_RADIUS,
							 TIMELINE_RADIUS);
    
	PushID(-1);
	SetCursorScreenPos(start);
	InvisibleButton(str_id, end - start);
	if (IsItemActive() && selected)
	{
        *selected = true;
	}
	PopID();

	SetCursorScreenPos(cursor_pos + ImVec2(0, GetTextLineHeightWithSpacing()));

	win->DrawList->AddRectFilled(start, end, IsItemActive() || IsItemHovered() ? active_color : inactive_color);
	
	if (values[0] > values[1])
	{
		auto tmp = values[0];
		values[0] = values[1];
		values[1] = tmp;
	}
	if (values[1] > s_max_timeline_value) values[1] = s_max_timeline_value;
	if (values[0] < 0) values[0] = 0;

	return changed;
}


void ImGuiEndTimeline(int line_count, int* lines)
{
    using namespace ImGui;
	ImGuiWindow* win = GetCurrentWindow();
	
	ImU32 color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Button]);
	ImU32 line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Border]);
	ImU32 text_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Text]);
	float rounding = GImGui->Style.ScrollbarRounding;
	ImVec2 start(GetWindowContentRegionMin().x + win->Pos.x,
		GetWindowContentRegionMax().y - GetTextLineHeightWithSpacing() + win->Pos.y);
	ImVec2 end = GetWindowContentRegionMax() + win->Pos;

	win->DrawList->AddRectFilled(start, end, color, rounding);
    
	const ImVec2 text_offset(0, GetTextLineHeightWithSpacing());
	for (int i = 0; i < line_count; ++i)
	{
		ImVec2 a = GetWindowContentRegionMin() + win->Pos + ImVec2(TIMELINE_RADIUS, 0);
		a.x += lines ? float(lines[i]) / float(s_max_timeline_value) * ImGuiGetWindowContentRegionWidth() : float(i) * ImGuiGetWindowContentRegionWidth() / float(line_count);
		ImVec2 b = a;
		b.y = start.y;
		win->DrawList->AddLine(a, b, line_color);
		char tmp[256];
		ImFormatString(tmp, sizeof(tmp), "%d", lines ? lines[i] : i * s_max_timeline_value / line_count);
		win->DrawList->AddText(b, text_color, tmp);
	}

	EndChild();
}