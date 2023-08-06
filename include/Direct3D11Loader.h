#pragma once

#include "Common.h"
#include "Singleton.h"

struct gw2al_core_vtable;
struct D3D9_wrapper;
struct ID3D11Device;
struct IDXGISwapChain;

class Direct3D11Loader : public Singleton<Direct3D11Loader>
{
public:
	virtual void PrePresentSwapChain();
	virtual void PostCreateSwapChain(HWND hwnd, ID3D11Device* dev, IDXGISwapChain* swc);
	virtual void PreResizeSwapChain();
	virtual void PostResizeSwapChain(uint w, uint h);

	void Init(gw2al_core_vtable* gAPI);

	virtual void ExtraInit(gw2al_core_vtable* gAPI, D3D9_wrapper* d3d9_wrap) {}

protected:
	bool isFrameDrawn_ = false;
	bool isDirect3DHooked_ = false;
};
