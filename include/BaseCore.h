#pragma once
#include <wrl/client.h>
#include <Event.h>
#include <imgui.h>
#include <neargye/semver.hpp>
#include <mutex>

using Microsoft::WRL::ComPtr;

struct IDXGISwapChain;

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3DUserDefinedAnnotation;
struct ID3D11RenderTargetView;

struct ImFont;
struct ImGuiContext;

class BaseCore
{
public:
	static void Init(HMODULE dll);
	static void Shutdown();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

	void OnInjectorCreated();

	void OnInputLanguageChange();
	
	void DisplayErrorPopup(const char* message);

	[[nodiscard]] UINT GetDpiForWindow(HWND hwnd);

	virtual ~BaseCore();

	void Draw();

	void Update();

	[[nodiscard]] auto gameWindow() const { return gameWindow_; }
	[[nodiscard]] auto dllModule() const { return dllModule_; }
	[[nodiscard]] auto screenWidth() const { return screenWidth_; }
	[[nodiscard]] auto screenHeight() const { return screenHeight_; }

	[[nodiscard]] auto* font() const { return font_; }
	[[nodiscard]] auto* fontBold() const { return fontBold_; }
	[[nodiscard]] auto* fontBlack() const { return fontBlack_; }
	[[nodiscard]] auto* fontItalic() const { return fontItalic_; }
	[[nodiscard]] auto* fontIcon() const { return fontIcon_; }
	[[nodiscard]] auto* fontMono() const { return fontMono_; }

	[[nodiscard]] auto device() const { return device_; }

	[[nodiscard]] auto& languageChangeEvent() { return languageChangeEvent_.Downcast(); }

protected:
	virtual void InnerDraw() {}
	virtual void InnerUpdate() {}
	virtual void InnerFrequentUpdate() {}
	virtual void InnerInfrequentUpdate() {}
	virtual void InnerOnFocus() {}
	virtual void InnerOnFocusLost() {}
	virtual void InnerInitPreImGui() {}
	virtual void InnerInitPreFontImGui() {}
	virtual void InnerInitPostImGui() {}
	virtual void InnerShutdown() {}
	virtual void InnerInternalInit() {}
	[[nodiscard]] virtual unsigned int GetShaderArchiveID() const = 0;
	[[nodiscard]] virtual const wchar_t* GetShaderDirectory() const = 0;
	[[nodiscard]] virtual const wchar_t* GetGithubRepoSubUrl() const = 0;

	void InternalInit(HMODULE dll);
	void InternalShutdown();
	void OnFocusLost();
	void OnFocus();

	void PostCreateSwapChain(HWND hwnd, ID3D11Device* device, IDXGISwapChain* swc);
	void PreResizeSwapChain();
	void PostResizeSwapChain(unsigned int w, unsigned int h);
	bool CheckForConflictingModule(const char* name, const char* message);

	HWND gameWindow_ = nullptr;
	HMODULE dllModule_ = nullptr;
	unsigned int screenWidth_ = 0, screenHeight_ = 0;
	bool firstFrame_ = true;

	ComPtr<ID3D11Device> device_ = nullptr;
	ComPtr<ID3D11DeviceContext> context_ = nullptr;
	ComPtr<IDXGISwapChain> swc_ = nullptr;

	ComPtr<ID3DUserDefinedAnnotation> annotations_;

	ComPtr<ID3D11RenderTargetView> backBufferRTV_;

	ImFont* font_ = nullptr, * fontBold_ = nullptr, * fontBlack_ = nullptr, * fontItalic_ = nullptr, * fontDraw_ = nullptr, * fontIcon_ = nullptr, * fontMono_ = nullptr;

	using LanguageChangeEvent = Event<void()>;

	LanguageChangeEvent languageChangeEvent_;

	ImGuiContext* imguiContext_ = nullptr;

	using GetDpiForWindow_t = UINT(WINAPI*)(HWND hwnd);
	HMODULE user32_ = nullptr;
	GetDpiForWindow_t getDpiForWindow_ = nullptr;

	unsigned int tickSkip_ = 0;
	const unsigned int TickSkipCount = 10;
	unsigned int longTickSkip_ = 0;
	const unsigned int LongTickSkipCount = 600;
	bool active_ = true;
	bool subclassed_ = false;
	
	ImGuiID                   errorPopupID_  = 0;
	std::string				  errorPopupMessage_;
	std::string				  errorPopupTitle_;

	friend class Direct3D11Loader;
};
BaseCore& GetBaseCore();