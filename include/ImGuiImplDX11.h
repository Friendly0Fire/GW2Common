#pragma once

struct ID3D11BlendState;

class ImGuiBlendStateOverride
{
	ID3D11BlendState* originalBlendState_;
public:
	ImGuiBlendStateOverride(ID3D11BlendState* bs);

	ImGuiBlendStateOverride(ImGuiBlendStateOverride&&) = delete;
	ImGuiBlendStateOverride(const ImGuiBlendStateOverride&) = delete;
	ImGuiBlendStateOverride& operator=(const ImGuiBlendStateOverride&) = delete;
	ImGuiBlendStateOverride& operator=(ImGuiBlendStateOverride&&) = delete;

	~ImGuiBlendStateOverride();
};