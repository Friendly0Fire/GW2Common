#include <imgui/backends/imgui_impl_dx11.cpp>

ID3D11BlendState* g_pBlendStateOriginal = g_pBlendState;

void ImGui_ImplDX11_OverrideBlendState(ID3D11BlendState* blendState)
{
	g_pBlendState = blendState ? blendState : g_pBlendStateOriginal;
}