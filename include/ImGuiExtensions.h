#pragma once
#include <imgui.h>

#include "Common.h"
#include "ConfigurationOption.h"
#include "ImGuiImplDX11.h"
#include "Keybind.h"

ImVec2 operator*(const ImVec2& a, const ImVec2& b);
ImVec2 operator*(const ImVec2& a, f32 b);
ImVec2 operator/(const ImVec2& a, const ImVec2& b);
ImVec2 operator/(const ImVec2& a, f32 b);
ImVec2 operator-(const ImVec2& a, const ImVec2& b);
ImVec2 operator+(const ImVec2& a, const ImVec2& b);
ImVec2 operator*=(ImVec2& a, const ImVec2& b);
ImVec2 operator*=(ImVec2& a, f32 b);

ImVec4 operator*(const ImVec4& a, const ImVec4& b);
ImVec4 operator*(const ImVec4& a, f32 b);
ImVec4 operator/(const ImVec4& v, f32 f);
ImVec4 operator*=(ImVec4& a, f32 b);
ImVec4 operator*=(ImVec4& a, const ImVec4& b);

inline ImVec4 ConvertVector(const vec4& val) { return { val.x, val.y, val.z, val.w }; }

inline ImVec2 ConvertVector(const vec2& val) { return { val.x, val.y }; }

template<typename T, size_t N>
auto ToImGui(const glm::vec<N, T>& vec) {
    static_assert(N == 2 || N == 4);
    if constexpr(N == 2)
        return ImVec2(f32(vec.x), f32(vec.y));
    else
        return ImVec4(f32(vec.x), f32(vec.y), f32(vec.z), f32(vec.w));
}

template<typename T, size_t N, glm::qualifier Q, i32... Es>
auto ToImGui(const glm::detail::_swizzle<N, T, Q, Es...>& vec) {
    return ToImGui(glm::vec<N, T>(vec));
}

template<typename T = f32>
auto FromImGui(const ImVec2& vec) {
    return glm::vec<2, T>(T(vec.x), T(vec.y));
}

template<typename T = f32>
auto FromImGui(const ImVec4& vec) {
    return glm::vec<4, T>(T(vec.x), T(vec.y), T(vec.z), T(vec.w));
}

void ImGuiKeybindInput(Keybind& keybind, Keybind** keybindBeingModified, const char* tooltip);

template<typename F, typename T, typename... Args>
bool ImGuiConfigurationWrapper(F fct, const char* name, ConfigurationOption<T>& value, Args&&... args) {
    if(fct(name, &value.value(), std::forward<Args>(args)...)) {
        value.ForceSave();
        return true;
    }

    return false;
}

template<typename F, typename T, typename... Args>
bool ImGuiConfigurationWrapper(F fct, ConfigurationOption<T>& value, Args&&... args) {
    if(fct(value.displayName().c_str(), &value.value(), std::forward<Args>(args)...)) {
        value.ForceSave();
        return true;
    }

    return false;
}

inline bool ImGuiInputIntFormat(const char* label, i32* v, const char* format, i32 step = 0, i32 step_fast = 0,
                                ImGuiInputTextFlags flags = 0) {
    return ImGui::InputScalar(label, ImGuiDataType_S32, (void*)v, (void*)(step > 0 ? &step : NULL),
                              (void*)(step_fast > 0 ? &step_fast : NULL), format, flags);
}

void ImGuiTitle(const char* text, f32 scale = 1.f);
f32 ImGuiHelpTooltipSize();
enum class ImGuiHelpTooltipElementType
{
    DEFAULT = 0,
    BULLET = 1,
};
void ImGuiHelpTooltip(std::initializer_list<std::pair<ImGuiHelpTooltipElementType, const char*>> desc, f32 scale = 1.f,
                      bool includeScrollbars = true);
inline void ImGuiHelpTooltip(const char* desc, f32 scale = 1.f, bool includeScrollbars = true) {
    ImGuiHelpTooltip({ { ImGuiHelpTooltipElementType::DEFAULT, desc } }, scale, includeScrollbars);
}

f32 ImGuiCloseSize();
bool ImGuiClose(const char* id, f32 scale = 1.f, bool includeScrollbars = true);

class ImGuiDisabler
{
    static f32 alpha_s;
    static bool disabled_s;
    bool active_;

public:
    ImGuiDisabler(bool active, f32 alpha = 0.6f);
    ~ImGuiDisabler();

    void Disable();
    void Enable();

    [[nodiscard]] bool disabled() const { return disabled_s; }
};

inline f32 ImGuiGetWindowContentRegionWidth() { return ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x; }

struct ImTimelineRange
{
    ImTimelineRange() = default;
    ImTimelineRange(i32 a, i32 b) {
        values[0] = a;
        values[1] = b;
    }
    ImTimelineRange(const std::pair<i32, i32>& p) {
        values[0] = p.first;
        values[1] = p.second;
    }

    i32 values[2];

    auto& operator[](i32 i) { return values[i]; }
};

struct ImTimelineResult
{
    bool changed = false;
    bool selected = false;
};

bool ImGuiBeginTimeline(const char* str_id, i32 max_value, f32 text_width, i32 number_elements);
ImTimelineResult ImGuiTimelineEvent(const char* str_id, const char* display_name, ImTimelineRange& values, bool selected);
void ImGuiEndTimeline(i32 line_count, i32* lines = nullptr, ImVec2* mouseTop = nullptr, i32* mouseNumber = nullptr);
