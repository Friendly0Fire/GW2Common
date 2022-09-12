#pragma once
#include <Common.h>
#include <imgui.h>
#include <ConfigurationOption.h>
#include <ImGuiImplDX11.h>
#include <Keybind.h>

ImVec2 operator*(const ImVec2& a, const ImVec2& b);
ImVec2 operator*(const ImVec2& a, float b);
ImVec2 operator/(const ImVec2& a, const ImVec2& b);
ImVec2 operator/(const ImVec2& a, float b);
ImVec2 operator-(const ImVec2& a, const ImVec2& b);
ImVec2 operator+(const ImVec2& a, const ImVec2& b);
ImVec2 operator*=(ImVec2& a, const ImVec2& b);
ImVec2 operator*=(ImVec2& a, float b);

ImVec4 operator*(const ImVec4& a, const ImVec4& b);
ImVec4 operator*(const ImVec4& a, float b);
ImVec4 operator/(const ImVec4& v, float f);
ImVec4 operator*=(ImVec4& a, float b);
ImVec4 operator*=(ImVec4& a, const ImVec4& b);

inline ImVec4 ConvertVector(const fVector4& val) {
	return { val.x, val.y, val.z, val.w };
}

inline ImVec2 ConvertVector(const fVector2& val) {
	return { val.x, val.y };
}


template<typename T, size_t N>
auto ToImGui(const glm::vec<N, T>& vec) {
	static_assert(N == 2 || N == 4);
	if constexpr(N == 2)
		return ImVec2(float(vec.x), float(vec.y));
	else
		return ImVec4(float(vec.x), float(vec.y), float(vec.z), float(vec.w));
}

template<typename T, size_t N, glm::qualifier Q, int... Es>
auto ToImGui(const glm::detail::_swizzle<N, T, Q, Es...>& vec) {
	return ToImGui(glm::vec<N, T>(vec));
}

template<typename T = float>
auto FromImGui(const ImVec2& vec) {
	return glm::vec<2, T>(T(vec.x), T(vec.y));
}

template<typename T = float>
auto FromImGui(const ImVec4& vec) {
	return glm::vec<4, T>(T(vec.x), T(vec.y), T(vec.z), T(vec.w));
}

void ImGuiKeybindInput(Keybind& keybind, Keybind** keybindBeingModified, const char* tooltip);

template<typename F, typename T, typename... Args>
bool ImGuiConfigurationWrapper(F fct, const char* name, ConfigurationOption<T>& value, Args&&... args)
{
	if(fct(name, &value.value(), std::forward<Args>(args)...)) {
		value.ForceSave();
		return true;
	}

	return false;
}

template<typename F, typename T, typename... Args>
bool ImGuiConfigurationWrapper(F fct, ConfigurationOption<T>& value, Args&&... args)
{
	if(fct(value.displayName().c_str(), &value.value(), std::forward<Args>(args)...)) {
		value.ForceSave();
		return true;
	}

	return false;
}

inline bool ImGuiInputIntFormat(const char* label, int* v, const char* format, int step = 0, int step_fast = 0, ImGuiInputTextFlags flags = 0)
{
	return ImGui::InputScalar(label, ImGuiDataType_S32, (void*)v, (void*)(step > 0 ? &step : NULL), (void*)(step_fast > 0 ? &step_fast : NULL), format, flags);
}

void ImGuiTitle(const char * text, float scale = 1.f);
float ImGuiHelpTooltipSize();
enum class ImGuiHelpTooltipElementType
{
    DEFAULT = 0,
    BULLET = 1,
};
void ImGuiHelpTooltip(std::initializer_list<std::pair<ImGuiHelpTooltipElementType, const char*>> desc, float scale = 1.f, bool includeScrollbars = true);
inline void ImGuiHelpTooltip(const char* desc, float scale = 1.f, bool includeScrollbars = true)
{
    ImGuiHelpTooltip({ { ImGuiHelpTooltipElementType::DEFAULT, desc } }, scale, includeScrollbars);
}

float ImGuiCloseSize();
bool ImGuiClose(const char* id, float scale = 1.f, bool includeScrollbars = true);

class ImGuiDisabler
{
	static bool disabled_;
public:
	ImGuiDisabler(bool disable, float alpha = 0.6f);
	~ImGuiDisabler();

    [[nodiscard]] bool disabled() const { return disabled_; }
};

inline float ImGuiGetWindowContentRegionWidth() { return ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x; }