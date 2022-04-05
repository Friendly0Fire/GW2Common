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

ImVec4 operator/(const ImVec4& v, float f);

inline ImVec4 ConvertVector(const fVector4& val) {
	return { val.x, val.y, val.z, val.w };
}

inline ImVec2 ConvertVector(const fVector2& val) {
	return { val.x, val.y };
}

inline ImVec2 ToImGui(const glm::vec2& val) {
	return { val.x, val.y };
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

void ImGuiTitle(const char * text);
float ImGuiHelpTooltipSize();
void ImGuiHelpTooltip(const char* desc);

class ImGuiDisabler
{
	static bool disabled_;
public:
	ImGuiDisabler(bool disable, float alpha = 0.6f);
	~ImGuiDisabler();
};

inline float ImGuiGetWindowContentRegionWidth() { return ImGui::GetWindowContentRegionMax().x - ImGui::GetWindowContentRegionMin().x; }