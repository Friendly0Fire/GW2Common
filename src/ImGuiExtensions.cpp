#include "ImGuiExtensions.h"

#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include <imgui_internal.h>

#include "Input.h"

ImVec2 operator*(const ImVec2& a, const ImVec2& b) { return { a.x * b.x, a.y * b.y }; }
ImVec2 operator*(const ImVec2& a, f32 b) { return { a.x * b, a.y * b }; }
ImVec2 operator/(const ImVec2& a, const ImVec2& b) { return { a.x / b.x, a.y / b.y }; }
ImVec2 operator/(const ImVec2& a, f32 b) { return { a.x / b, a.y / b }; }
ImVec2 operator-(const ImVec2& a, const ImVec2& b) { return { a.x - b.x, a.y - b.y }; }
ImVec2 operator+(const ImVec2& a, const ImVec2& b) { return { a.x + b.x, a.y + b.y }; }

ImVec2 operator+=(ImVec2& a, const ImVec2& b) {
    a = a + b;
    return a;
}

ImVec2 operator-=(ImVec2& a, const ImVec2& b) {
    a = a - b;
    return a;
}

ImVec2 operator*=(ImVec2& a, const ImVec2& b) {
    a = a * b;
    return a;
}

ImVec2 operator*=(ImVec2& a, f32 b) {
    a = a * b;
    return a;
}

ImVec4 operator*(const ImVec4& a, f32 b) { return { a.x * b, a.y * b, a.z * b, a.w * b }; }

ImVec4 operator*(const ImVec4& a, const ImVec4& b) { return { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w }; }

ImVec4 operator/(const ImVec4& v, const f32 f) { return { v.x / f, v.y / f, v.z / f, v.w / f }; }

ImVec4 operator*=(ImVec4& a, f32 b) {
    a = a * b;
    return a;
}

ImVec4 operator*=(ImVec4& a, const ImVec4& b) {
    a = a * b;
    return a;
}

void ImGuiKeybindInput(Keybind& keybind, Keybind** keybindBeingModified, const char* tooltip) {
    bool beingModified = *keybindBeingModified == &keybind;
    bool disableSet = !beingModified && *keybindBeingModified != nullptr;
    std::string suffix = "##" + keybind.nickname();

    f32 windowWidth = ImGui::GetWindowWidth() - UI::HelpTooltip.size();

    ImGui::PushItemWidth(windowWidth * 0.45f);

    i32 popcount = 1;
    if(beingModified) {
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

    if(disableSet) {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_Button));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_Button));
    }

    if(!beingModified && ImGui::Button(("Set" + suffix).c_str(), ImVec2(windowWidth * 0.1f, 0.f)) && !disableSet) {
        if(keybindBeingModified)
            Input::i().CancelRecordInputs();

        *keybindBeingModified = &keybind;
        keybind.keysDisplayString()[0] = '\0';
        Input::i().BeginRecordInputs([&keybind, keybindBeingModified](KeyCombo kc, bool final) {
            if(final) {
                keybind.keyCombo(kc);
                if(*keybindBeingModified == &keybind) {
                    *keybindBeingModified = nullptr;
                }
            }
            else
                keybind.UpdateDisplayString(kc);
        });
    }
    else if(beingModified && ImGui::Button(("Clear" + suffix).c_str(), ImVec2(windowWidth * 0.1f, 0.f))) {
        Input::i().CancelRecordInputs();
        *keybindBeingModified = nullptr;
        keybind.keyCombo({});
    }

    if(disableSet) {
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);
    }

    ImGui::SameLine();

    ImGui::PushItemWidth(windowWidth * 0.5f);

    ImGui::Text(keybind.displayName().c_str());

    ImGui::PopItemWidth();

    if(tooltip)
        UI::HelpTooltip(tooltip);
}
thread_local i32 s_max_timeline_value;
thread_local f32 s_timeline_text_width;
thread_local i32 s_timeline_element_count;

bool ImGuiBeginTimeline(const char* str_id, i32 max_value, f32 text_width, i32 number_elements) {
    using namespace ImGui;
    s_max_timeline_value = max_value + 2;
    s_timeline_text_width = text_width;
    s_timeline_element_count = number_elements;
    return ImGui::BeginChild(str_id, ImVec2(0, (1 + number_elements) * GetTextLineHeightWithSpacing()));
}

static constexpr f32 TIMELINE_RADIUS = 8.f;

ImTimelineResult ImGuiTimelineEvent(const char* str_id, const char* display_name, ImTimelineRange& values, bool selected) {
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

    for(i32 i = 0; i < 2; ++i) {
        if(values[i] < 0 || values[i] > s_max_timeline_value)
            continue;

        f32 offsetX = 1.f + (2 * i - 1) * 0.5f;

        ImVec2 pos = cursor_pos;
        pos.x += win->Size.x * f32(values[i] + offsetX) / f32(s_max_timeline_value) + TIMELINE_RADIUS;
        pos.y += TIMELINE_RADIUS;

        SetCursorScreenPos(pos - ImVec2(TIMELINE_RADIUS, TIMELINE_RADIUS));
        PushID(i);
        InvisibleButton(str_id, ImVec2(2 * TIMELINE_RADIUS, 2 * TIMELINE_RADIUS));
        if(IsItemActive() || IsItemHovered()) {
            ImGui::SetTooltip("%d", values[i]);
            ImVec2 a(pos.x, GetWindowContentRegionMin().y + win->Pos.y);
            ImVec2 b(pos.x, GetWindowContentRegionMax().y + win->Pos.y);
            win->DrawList->AddLine(a, b, line_color);
        }
        if(IsItemActive() && IsMouseDragging(ImGuiMouseButton_Left)) {
            values[i] = i32((GetIO().MousePos.x - cursor_pos.x) / win->Size.x * f32(s_max_timeline_value) - offsetX);
            res.changed = true;
        }
        PopID();
        win->DrawList->AddCircleFilled(pos, TIMELINE_RADIUS, IsItemActive() || IsItemHovered() ? active_color : inactive_color);
    }

    ImVec2 start = cursor_pos;
    start.x += win->Size.x * std::max(0.f, values[0] + 0.5f) / f32(s_max_timeline_value) + 2 * TIMELINE_RADIUS;
    start.y += TIMELINE_RADIUS * 0.5f;
    ImVec2 end = cursor_pos;
    end.x += win->Size.x * std::max(0.f, values[1] + 1.5f) / f32(s_max_timeline_value);
    end.y += TIMELINE_RADIUS * 1.5f;

    PushID(-1);
    SetCursorScreenPos(start);
    InvisibleButton(str_id, end - start);
    if(IsItemActivated())
        GetStateStorage()->SetFloat(
            id, GetIO().MouseClickedPos[0].x - (cursor_pos.x + win->Size.x * f32(values[0] + 0.5f) / f32(s_max_timeline_value)));
    if(IsItemActive() && IsMouseDragging(0)) {
        f32 offset = GetStateStorage()->GetFloat(id);

        i32 dist = values[1] - values[0];
        values[0] = i32((GetIO().MousePos.x - offset - cursor_pos.x) / win->Size.x * f32(s_max_timeline_value));
        values[1] = values[0] + dist;
        res.changed = true;
    }
    PopID();

    SetCursorScreenPos(cursor_pos + ImVec2(-s_timeline_text_width, GetTextLineHeightWithSpacing()));

    win->DrawList->AddRectFilled(start, end, IsItemActive() || IsItemHovered() ? active_color : inactive_color);

    if(values[0] > values[1])
        std::swap(values[0], values[1]);
    for(i32 i = 0; i < 2; i++) {
        if(values[i] > s_max_timeline_value)
            values[i] = s_max_timeline_value;
        else if(values[i] < 0)
            values[i] = 0;
    }

    return res;
}

void ImGuiEndTimeline(i32 line_count, i32* lines, ImVec2* mouseTop, i32* mouseNumber) {
    using namespace ImGui;
    ImGuiWindow* win = GetCurrentWindow();

    ImU32 color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Button]);
    ImU32 line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Border]);
    ImU32 text_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Text]);
    f32 rounding = GImGui->Style.ScrollbarRounding;
    ImVec2 start(win->DC.CursorPos.x + s_timeline_text_width, win->DC.CursorPos.y);
    ImVec2 end(GetWindowContentRegionMax().x + win->Pos.x, start.y + GetTextLineHeightWithSpacing());

    win->DrawList->AddRectFilled(start, end, color, rounding);

    const ImVec2 text_offset(0, GetTextLineHeightWithSpacing());
    for(i32 i = 0; i < line_count; ++i) {
        ImVec2 a = GetWindowContentRegionMin() + win->Pos + ImVec2(TIMELINE_RADIUS + s_timeline_text_width, 0);
        a.x += lines ? f32(lines[i] + 1.f) / f32(s_max_timeline_value) * UI::GetAvailableSpace().x
                     : f32(i) * UI::GetAvailableSpace().x / f32(line_count);
        ImVec2 b = a;
        b.y = start.y;
        win->DrawList->AddLine(a, b, line_color);
        char tmp[256];
        ImFormatString(tmp, sizeof(tmp), "%d", lines ? lines[i] : i * s_max_timeline_value / line_count);
        win->DrawList->AddText(b, text_color, tmp);
    }

    if(win->Rect().Contains(GetMousePos())) {
        f32 ratio = UI::GetAvailableSpace().x / s_max_timeline_value;
        f32 offset = win->Pos.x + TIMELINE_RADIUS + s_timeline_text_width;
        i32 num = std::round((ImGui::GetMousePos().x - offset) / ratio);
        f32 x = num * ratio + offset;

        ImVec2 a(x, win->Pos.y);
        ImVec2 b(x, start.y);

        win->DrawList->AddLine(a, b, line_color, 2.f);

        if(mouseTop)
            *mouseTop = a;

        if(mouseNumber)
            *mouseNumber = num;
    }

    EndChild();
}

namespace UI
{
Scoped::Font::Font(UI::Font f, f32 scale)
    : Stack { [f] {
        switch(f) {
        default:
        case UI::Font::Default:
            return GetBaseCore().font();
        case UI::Font::Bold:
            return GetBaseCore().fontBold();
        case UI::Font::Italic:
            return GetBaseCore().fontItalic();
        case UI::Font::Monospace:
            return GetBaseCore().fontMono();
        }
    }() }, scale_ { scale } { }

bool Detail::CloseButton::operator()(const char* id, f32 scale, bool includeScrollbars) const {
    Scoped::Font _(Font::Default, scale);
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - size() -
                         (includeScrollbars ? (ImGui::GetScrollX() + ImGui::GetStyle().ScrollbarSize) : 0.f));

    return ImGui::Button(std::format("{}##{}", ICON_FA_TIMES, id).c_str());
}

f32 Detail::CloseButton::size() const {
    GW2_ASSERT(ImGui::GetCurrentContext()->Font == GetBaseCore().fontBold() ||
        ImGui::GetCurrentContext()->Font == GetBaseCore().font());
    return ImGui::CalcTextSize(ICON_FA_TIMES).x + ImGui::GetStyle().ItemSpacing.x + 1.f;
}

void Title(std::string_view text, f32 scale) {
    ImGui::Dummy({ 0, ImGui::GetStyle().ItemSpacing.y * 2 });
    SCOPE(Font(Font::Bold, scale)) {
        ImGui::TextUnformatted(text.data(), text.data() + text.size());
    }
    ImGui::Separator();
    ImGui::Spacing();
}

void Detail::HelpTooltip::operator()(std::string_view text, f32 scale, bool includeScrollbars) const {
    ImGui::SameLine();
    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - size() -
        (includeScrollbars ? (ImGui::GetScrollX() + ImGui::GetStyle().ScrollbarSize) : 0.f));

    SCOPE(Font(Font::Default)) {
        ImGui::TextDisabled(reinterpret_cast<const char*>(ICON_FA_QUESTION_CIRCLE));
    }

    if (ImGui::IsItemHovered()) {
        SCOPE(Tooltip()) SCOPE(FontScale(scale)) SCOPE(TextWrapPos(ImGui::GetFontSize() * 35.0f)) {
            ImGui::TextUnformatted(text.data(), text.data() + text.size());
        }
    }
}

f32 Detail::HelpTooltip::size() const {
    SCOPED(Font(Font::Default, 1.f));
    return ImGui::CalcTextSize(ICON_FA_QUESTION_CIRCLE).x + ImGui::GetStyle().ItemSpacing.x + 1.f;
}
} // namespace GW2Clarity::UI