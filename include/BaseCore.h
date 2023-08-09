#pragma once
#include "Common.h"
#include "Event.h"

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

    virtual ~BaseCore() = default;

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
    [[nodiscard]] auto context() const { return context_; }

    [[nodiscard]] auto& languageChangeEvent() { return languageChangeEvent_.Downcast(); }

    ComPtr<ID3D11RenderTargetView>& backBufferRTV() { return backBufferRTV_; }

    [[nodiscard]] bool swapChainInitialized() const { return swapChainInitialized_; }

    [[nodiscard]] const std::filesystem::path& addonDirectory() const { return addonDirectory_; }

protected:
    virtual void InnerDraw() { }
    virtual void InnerUpdate() { }
    virtual void InnerFrequentUpdate() { }
    virtual void InnerInfrequentUpdate() { }
    virtual void InnerOnFocus() { }
    virtual void InnerOnFocusLost() { }
    virtual void InnerInitPreImGui() { }
    virtual void InnerInitPreFontImGui() { }
    virtual void InnerInitPostImGui() { }
    virtual void InnerShutdown() { }
    virtual void InnerInternalInit() { }
    [[nodiscard]] virtual u32 GetShaderArchiveID() const = 0;
    [[nodiscard]] virtual const wchar_t* GetShaderDirectory() const = 0;
    [[nodiscard]] virtual const wchar_t* GetGithubRepoSubUrl() const = 0;

    virtual std::optional<LRESULT> OnInput(UINT msg, WPARAM& wParam, LPARAM& lParam) { return std::nullopt; }

    void InternalInit(HMODULE dll);
    void InternalShutdown();
    void OnFocusLost();
    void OnFocus();

    void PostCreateSwapChain(HWND hwnd, ID3D11Device* device, IDXGISwapChain* swc);
    virtual void PreResizeSwapChain();
    virtual void PostResizeSwapChain(u32 w, u32 h);
    bool CheckForConflictingModule(const char* name, const char* message);

    HWND gameWindow_ = nullptr;
    HMODULE dllModule_ = nullptr;
    u32 screenWidth_ = 0, screenHeight_ = 0;
    bool firstFrame_ = true;
    std::filesystem::path addonDirectory_;

    ComPtr<ID3D11Device> device_ = nullptr;
    ComPtr<ID3D11DeviceContext> context_ = nullptr;
    ComPtr<IDXGISwapChain> swc_ = nullptr;

    ComPtr<ID3DUserDefinedAnnotation> annotations_;

    ComPtr<ID3D11RenderTargetView> backBufferRTV_;

    ImFont *font_ = nullptr, *fontBold_ = nullptr, *fontBlack_ = nullptr, *fontItalic_ = nullptr, *fontDraw_ = nullptr,
           *fontIcon_ = nullptr, *fontMono_ = nullptr;

    using LanguageChangeEvent = Event<void()>;

    LanguageChangeEvent languageChangeEvent_;

    ImGuiContext* imguiContext_ = nullptr;

    using GetDpiForWindow_t = UINT(WINAPI*)(HWND hwnd);
    HMODULE user32_ = nullptr;
    GetDpiForWindow_t getDpiForWindow_ = nullptr;

    u32 tickSkip_ = 0;
    const u32 TickSkipCount = 10;
    u32 longTickSkip_ = 0;
    const u32 LongTickSkipCount = 600;
    bool active_ = true;
    bool subclassed_ = false;

    u32 errorPopupID_ = 0;
    std::vector<std::string> errorPopupMessages_;
    std::string errorPopupTitle_;

    std::atomic<bool> swapChainInitialized_ = false;

    friend class Direct3D11Loader;
};
BaseCore& GetBaseCore();
