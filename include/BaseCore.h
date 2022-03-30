#pragma once
#include <wrl/client.h>
#include <Event.h>

using Microsoft::WRL::ComPtr;

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ImFont;

class BaseCore
{
public:
	auto gameWindow() const { return gameWindow_; }
	auto dllModule() const { return dllModule_; }
	auto baseWndProc() const { return baseWndProc_; }
	auto screenWidth() const { return screenWidth_; }
	auto screenHeight() const { return screenHeight_; }

	auto font() const { return font_; }
	auto fontBlack() const { return fontBlack_; }
	auto fontItalic() const { return fontItalic_; }
	auto fontIcon() const { return fontIcon_; }
	auto fontMono() const { return fontMono_; }

	auto device() const { return device_; }

	auto& languageChangeEvent() { return languageChangeEvent_.Downcast(); }

protected:
	HWND gameWindow_ = nullptr;
	HMODULE dllModule_ = nullptr;
	WNDPROC baseWndProc_ = nullptr;
	unsigned int screenWidth_ = 0, screenHeight_ = 0;

	ComPtr<ID3D11Device> device_;
	ComPtr<ID3D11DeviceContext> context_;
	ComPtr<IDXGISwapChain> swc_;

	ImFont* font_ = nullptr, * fontBlack_ = nullptr, * fontItalic_ = nullptr, * fontDraw_ = nullptr, * fontIcon_ = nullptr, * fontMono_ = nullptr;

	using LanguageChangeEvent = Event<void()>;

	LanguageChangeEvent languageChangeEvent_;
};
BaseCore& GetBaseCore();