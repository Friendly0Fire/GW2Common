#include <ImGuiImplDX11.h>
#include <imgui/backends/imgui_impl_dx11.cpp>

ImGuiBlendStateOverride::ImGuiBlendStateOverride(ID3D11BlendState* bs)
{
	auto* data = ImGui_ImplDX11_GetBackendData();
	originalBlendState_ = data->pBlendState;
	data->pBlendState = bs;
}

ImGuiBlendStateOverride::~ImGuiBlendStateOverride()
{
	auto* data = ImGui_ImplDX11_GetBackendData();
	data->pBlendState = originalBlendState_;
}
