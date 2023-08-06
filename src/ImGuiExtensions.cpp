#include "ImGuiExtensions.h"

#include <imgui_internal.h>
#include <IconFontCppHeaders/IconsFontAwesome5.h>

#include "Input.h"

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

bool ImGuiDisabler::disabled_s = false;
float ImGuiDisabler::alpha_s   = 0.6f;

void ImGuiDisabler::Disable()
{
    if (disabled_s || !active_)
        return;

    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * alpha_s);
    auto disabledColor = ImGui::GetColorU32(ImGuiCol_TextDisabled);
    ImGui::PushStyleColor(ImGuiCol_FrameBg, disabledColor);
    ImGui::PushStyleColor(ImGuiCol_CheckMark, disabledColor);
    ImGui::PushStyleColor(ImGuiCol_Text, disabledColor);
    ImGui::PushStyleColor(ImGuiCol_Button, disabledColor);
    disabled_s = true;
}

void ImGuiDisabler::Enable()
{
    if (!disabled_s || !active_)
        return;

    ImGui::PopStyleColor(4);
    ImGui::PopStyleVar();
    ImGui::PopItemFlag();
    disabled_s = false;
}

ImGuiDisabler::ImGuiDisabler(bool active, float alpha)
    : active_(active)
{
    alpha_s = alpha;
    Disable();
}

ImGuiDisabler::~ImGuiDisabler() {
    Enable();
}

thread_local int s_max_timeline_value;
thread_local float s_timeline_text_width;
thread_local int   s_timeline_element_count;

bool ImGuiBeginTimeline(const char* str_id, int max_value, float text_width, int number_elements)
{
    using namespace ImGui;
	s_max_timeline_value     = max_value + 2;
    s_timeline_text_width    = text_width;
    s_timeline_element_count = number_elements;
    return ImGui::BeginChild(str_id, ImVec2(0, (1 + number_elements) * GetTextLineHeightWithSpacing()));
}


static constexpr float TIMELINE_RADIUS = 8.f;

ImTimelineResult ImGuiTimelineEvent(const char* str_id, const char* display_name, ImTimelineRange& values, bool selected)
{
    using namespace ImGui;

    const ImGuiID id = GetID(str_id);

    ImTimelineResult res { false, selected };

	ImGuiWindow* win = GetCurrentWindow();
	const ImU32 inactive_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Button]);
	const ImU32 active_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_ButtonHovered]);
	const ImU32 line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_SliderGrabActive]);
    
	ImVec2 cursor_pos = win->DC.CursorPos;
    cursor_pos.x += s_timeline_text_width;
    
    if(BeginChild(std::format("{}_Text", str_id).c_str(), ImVec2(s_timeline_text_width, win->CalcFontSize())))
        Selectable(display_name, &res.selected);
    EndChild();

	for (int i = 0; i < 2; ++i)
	{
        if(values[i] < 0 || values[i] > s_max_timeline_value)
            continue;

        float offsetX = 1.f + (2 * i - 1) * 0.5f;

		ImVec2 pos = cursor_pos;
		pos.x += win->Size.x * float(values[i] + offsetX) / float(s_max_timeline_value) + TIMELINE_RADIUS;
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
			values[i] = int((GetIO().MousePos.x - cursor_pos.x) / win->Size.x * float(s_max_timeline_value) - offsetX);
			res.changed = true;
		}
		PopID();
		win->DrawList->AddCircleFilled(
			pos, TIMELINE_RADIUS, IsItemActive() || IsItemHovered() ? active_color : inactive_color);
	}
	
	ImVec2 start = cursor_pos;
	start.x += win->Size.x * std::max(0.f, values[0] + 0.5f) / float(s_max_timeline_value) + 2 * TIMELINE_RADIUS;
	start.y += TIMELINE_RADIUS * 0.5f;
	ImVec2 end = cursor_pos;
	end.x += win->Size.x * std::max(0.f, values[1] + 1.5f) / float(s_max_timeline_value);
	end.y += TIMELINE_RADIUS * 1.5f;

    PushID(-1);
	SetCursorScreenPos(start);
	InvisibleButton(str_id, end - start);
    if(IsItemActivated())
        GetStateStorage()->SetFloat(id, GetIO().MouseClickedPos[0].x - (cursor_pos.x + win->Size.x * float(values[0] + 0.5f) / float(s_max_timeline_value)));
	if (IsItemActive() && IsMouseDragging(0))
	{
        float offset = GetStateStorage()->GetFloat(id);

        int dist = values[1] - values[0];
		values[0] = int((GetIO().MousePos.x - offset - cursor_pos.x) / win->Size.x * float(s_max_timeline_value));
		values[1] = values[0] + dist;
		res.changed = true;
	}
	PopID();

	SetCursorScreenPos(cursor_pos + ImVec2(-s_timeline_text_width, GetTextLineHeightWithSpacing()));

	win->DrawList->AddRectFilled(start, end, IsItemActive() || IsItemHovered() ? active_color : inactive_color);
	
	if (values[0] > values[1])
        std::swap(values[0], values[1]);
    for(int i = 0; i < 2; i++)
    {
	    if (values[i] > s_max_timeline_value) values[i] = s_max_timeline_value;
	    else if (values[i] < 0) values[i] = 0;
    }

	return res;
}


void ImGuiEndTimeline(int line_count, int* lines, ImVec2* mouseTop, int* mouseNumber)
{
    using namespace ImGui;
	ImGuiWindow* win = GetCurrentWindow();
	
	ImU32 color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Button]);
	ImU32 line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Border]);
	ImU32 text_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Text]);
	float rounding = GImGui->Style.ScrollbarRounding;
    ImVec2 start(win->DC.CursorPos.x + s_timeline_text_width, win->DC.CursorPos.y);
    ImVec2 end(GetWindowContentRegionMax().x + win->Pos.x, start.y + GetTextLineHeightWithSpacing());

	win->DrawList->AddRectFilled(start, end, color, rounding);
    
	const ImVec2 text_offset(0, GetTextLineHeightWithSpacing());
	for (int i = 0; i < line_count; ++i)
	{
		ImVec2 a = GetWindowContentRegionMin() + win->Pos + ImVec2(TIMELINE_RADIUS + s_timeline_text_width, 0);
		a.x += lines ? float(lines[i] + 1.f) / float(s_max_timeline_value) * ImGuiGetWindowContentRegionWidth() : float(i) * ImGuiGetWindowContentRegionWidth() / float(line_count);
		ImVec2 b = a;
		b.y = start.y;
		win->DrawList->AddLine(a, b, line_color);
		char tmp[256];
		ImFormatString(tmp, sizeof(tmp), "%d", lines ? lines[i] : i * s_max_timeline_value / line_count);
		win->DrawList->AddText(b, text_color, tmp);
	}

    if (win->Rect().Contains(GetMousePos()))
    {
        float  ratio = ImGuiGetWindowContentRegionWidth() / s_max_timeline_value;
        float  offset = win->Pos.x + TIMELINE_RADIUS + s_timeline_text_width;
        int    num    = std::round((ImGui::GetMousePos().x - offset) / ratio);
        float  x      = num * ratio + offset;

        ImVec2 a(x, win->Pos.y);
        ImVec2 b(x, start.y);

        win->DrawList->AddLine(a, b, line_color, 2.f);

        if (mouseTop)
            *mouseTop = a;

        if (mouseNumber)
            *mouseNumber = num;
    }

	EndChild();
}