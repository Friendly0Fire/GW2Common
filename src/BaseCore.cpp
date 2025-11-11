#include "BaseCore.h"

#include <CommCtrl.h>
#include <IconFontCppHeaders/IconsFontAwesome5.h>
#include <d3d11_4.h>
#include <imgui/backends/imgui_impl_dx11.h>
#include <imgui/backends/imgui_impl_win32.h>
#include <imgui/imgui.h>
#include <shellapi.h>

#include "GFXSettings.h"
#include "Graphics.h"
#include "ImGuiPopup.h"
#include "ShaderManager.h"
#include "UpdateCheck.h"
#include <baseresource.h>

extern "C" __declspec(dllexport) void BaseCore_MockInit() {
    GetBaseCore().MockInit();
}

LONG WINAPI GW2TopLevelFilter(struct _EXCEPTION_POINTERS* pExceptionInfo);
extern LPTOP_LEVEL_EXCEPTION_FILTER previousTopLevelExceptionFilter;
extern void* vectoredExceptionHandlerHandle;

void BaseCore::Init(HMODULE dll, GW2Load_API& api) {
    LogInfo("This is {} {}", AddonName, AddonVersionString);

    auto osVer = GetOSVersion();
    if(osVer.dwOSVersionInfoSize == sizeof(RTL_OSVERSIONINFOW))
        LogInfo("Operating system version {}.{}.{}", osVer.dwMajorVersion, osVer.dwMinorVersion, osVer.dwBuildNumber);
    else
        LogWarn("Could not identify operating system version.");

    LogInfo("CPU is {}", GetCpuInfo());

    // Install our own exception handler to automatically log minidumps.
    vectoredExceptionHandlerHandle = AddVectoredExceptionHandler(GetCommandLineArg(L"xvehfirst") == L"1" ? 1 : 0, GW2TopLevelFilter);
    previousTopLevelExceptionFilter = SetUnhandledExceptionFilter(GW2TopLevelFilter);

    GetBaseCore().InternalInit(dll, api);
}

void BaseCore::Shutdown() {
    GetBaseCore().InternalShutdown();

    SetUnhandledExceptionFilter(previousTopLevelExceptionFilter);
    RemoveVectoredExceptionHandler(vectoredExceptionHandlerHandle);

    g_singletonManagerInstance.Shutdown();
}

void BaseCore::OnInputLanguageChange() {
    if(!active_)
        return;

    LogInfo("Input language change detected, reloading...");
    SettingsMenu::i().OnInputLanguageChange();

    languageChangeEvent_();
}

UINT BaseCore::GetDpiForWindow(HWND hwnd) {
    if(getDpiForWindow_)
        return getDpiForWindow_(hwnd);
    else
        return 96;
}

void BaseCore::InternalInit(HMODULE dll, GW2Load_API& api) {
    dllModule_ = dll;

    {
        wchar_t fn[MAX_PATH];
        GetModuleFileName(dllModule_, fn, MAX_PATH);

        addonDirectory_ = std::filesystem::path(fn).remove_filename();
    }

    user32_ = LoadLibrary(L"User32.dll");
    if(user32_)
        getDpiForWindow_ = (GetDpiForWindow_t)GetProcAddress(user32_, "GetDpiForWindow");

#ifdef _DEBUG
    LogInfo("Installing CRT report hook...");
    i32 rv = _CrtSetReportHook2(_CRT_RPTHOOK_INSTALL, CRTReportHook);
    if(rv < 0)
        LogWarn("CRT report hook install failed: {}", rv);
#endif

    imguiContext_ = ImGui::CreateContext();
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    ImGui::GetIO().ConfigInputTrickleEventQueue = false;

    InnerInternalInit(api);
    UpdateCheck::init(GetGithubRepoSubUrl());
}

void BaseCore::InternalShutdown() {
    InnerShutdown();

#ifdef _DEBUG
    _CrtSetReportHook2(_CRT_RPTHOOK_REMOVE, CRTReportHook);
#endif

    if(subclassed_) {
        subclassed_ = false;
        RemoveWindowSubclass(gameWindow_, &WndProc, 0);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    device_.Reset();
    context_.Reset();
    swc_.Reset();
    backBufferRTV_.Reset();
    annotations_.Reset();
    active_ = false;

    gameWindow_ = nullptr;
    getDpiForWindow_ = nullptr;
    if(user32_)
        FreeLibrary(user32_);
    user32_ = nullptr;
}

void BaseCore::OnFocusLost() { Input::i().OnFocusLost(); }

void BaseCore::OnFocus() {
    ShaderManager::i().ReloadAll();

    Input::i().OnFocus();
}

LRESULT CALLBACK BaseCore::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    auto& core = GetBaseCore();

    if(msg == WM_KILLFOCUS)
        core.OnFocusLost();
    else if(msg == WM_SETFOCUS)
        core.OnFocus();
    else if(auto rval = core.OnInput(msg, wParam, lParam); rval)
        return *rval;
    else if(Input::i().OnInput(msg, wParam, lParam))
        return 0;

    // Whatever's left should be sent to the game
    return DefSubclassProc(hWnd, msg, wParam, lParam);
}

void BaseCore::PreResizeSwapChain() { backBufferRTV_.Reset(); }

void BaseCore::PostResizeSwapChain(u32 w, u32 h) {
    if(!active_)
        return;

    screenWidth_ = w;
    screenHeight_ = h;

    ComPtr<ID3D11Texture2D> backbuffer;
    swc_->GetBuffer(0, IID_PPV_ARGS(backbuffer.GetAddressOf()));
    device_->CreateRenderTargetView(backbuffer.Get(), nullptr, backBufferRTV_.ReleaseAndGetAddressOf());
}

HHOOK g_callWndProcHook;
LRESULT CALLBACK CallWndProcHook(i32 nCode, WPARAM wParam, LPARAM lParam) {
    auto hwnd = GetBaseCore().gameWindow();
    if(nCode == HC_ACTION && hwnd && ((CWPSTRUCT*)lParam)->hwnd == hwnd) {
        auto success = SetWindowSubclass(hwnd, BaseCore::WndProc, 0, 0);
        GW2_ASSERT(success != 0);

        UnhookWindowsHookEx(g_callWndProcHook);
        g_callWndProcHook = nullptr;
    }
    return CallNextHookEx(0, nCode, wParam, lParam);
}

void BaseCore::PostCreateSwapChain(HWND hwnd, ID3D11Device* device, IDXGISwapChain* swc) {
    swapChainInitialized_ = true;
    ComPtr<IDXGIDevice> dxgiDevice;
    swc->GetDevice(IID_PPV_ARGS(&dxgiDevice));
    if(dxgiDevice) {
        ComPtr<IDXGIAdapter> adapter;
        dxgiDevice->GetAdapter(&adapter);

        if(adapter) {
            DXGI_ADAPTER_DESC desc;
            adapter->GetDesc(&desc);

            LogInfo(L"Graphics adapter is {}", desc.Description);
        }
    }
    gameWindow_ = hwnd;

    if(!subclassed_) {
        subclassed_ = true;
        // Hook CallWndProc to call SetWindowClass in the correct thread
        g_callWndProcHook = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProcHook, 0, GetWindowThreadProcessId(hwnd, 0));
    }

    device_ = device;
    device_->GetImmediateContext(&context_);
    swc_ = swc;

    RenderDocCapture::Init(device_);

    context_->QueryInterface(annotations_.ReleaseAndGetAddressOf());

    ComPtr<ID3D11Texture2D> backbuffer;
    swc_->GetBuffer(0, IID_PPV_ARGS(backbuffer.GetAddressOf()));
    device_->CreateRenderTargetView(backbuffer.Get(), nullptr, backBufferRTV_.GetAddressOf());

    DXGI_SWAP_CHAIN_DESC desc;
    swc_->GetDesc(&desc);

    screenWidth_ = desc.BufferDesc.Width;
    screenHeight_ = desc.BufferDesc.Height;

    firstFrame_ = true;

    ShaderManager::init(device_, GetShaderArchiveID(), dllModule_, GetShaderDirectory());

    UpdateCheck::i().CheckForUpdates();

    InnerInitPreImGui();

    // Init ImGui
    auto& imio = ImGui::GetIO();
    imio.IniFilename = nullptr;
    imio.IniSavingRate = 1.0f;
    auto fontCfg = ImFontConfig();
    fontCfg.FontDataOwnedByAtlas = false;

    constexpr f32 fontSize = 25.f;
    constexpr f32 fontSizeMono = 18.f;
    constexpr f32 fontSizeIcon = fontSize * 2.f / 3.f;

    const auto iconData = LoadResource(dllModule_, IDR_FONT_ICON);
    auto appendIcons = [&] {
        if(iconData.data()) {
            auto iconFontCfg = fontCfg;
            iconFontCfg.GlyphMinAdvanceX = fontSizeIcon;
            iconFontCfg.MergeMode = true;
            iconFontCfg.PixelSnapH = true;
            static const ImWchar iconRange[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
            imio.Fonts->AddFontFromMemoryTTF(iconData.data(), i32(iconData.size_bytes()), fontSizeIcon, &iconFontCfg, iconRange);
        }
    };

    if(const auto data = LoadResource(dllModule_, IDR_FONT); data.data()) {
        font_ = imio.Fonts->AddFontFromMemoryTTF(data.data(), i32(data.size_bytes()), fontSize, &fontCfg);
        appendIcons();
    }
    if(const auto data = LoadResource(dllModule_, IDR_FONT_BLACK); data.data()) {
        fontBold_ = imio.Fonts->AddFontFromMemoryTTF(data.data(), i32(data.size_bytes()), fontSize, &fontCfg);
        appendIcons();
    }
    if(const auto data = LoadResource(dllModule_, IDR_FONT_ITALIC); data.data())
        fontItalic_ = imio.Fonts->AddFontFromMemoryTTF(data.data(), i32(data.size_bytes()), fontSize, &fontCfg);
    if(const auto data = LoadResource(dllModule_, IDR_FONT_MONO); data.data())
        fontMono_ = imio.Fonts->AddFontFromMemoryTTF(data.data(), i32(data.size_bytes()), fontSizeMono, &fontCfg);
    
    if(font_)
        imio.FontDefault = font_;
    else
        imio.Fonts->AddFontDefault();

    InnerInitPreFontImGui();

    ImGui_ImplWin32_Init(gameWindow_);
    ImGui_ImplDX11_Init(device_.Get(), context_.Get());

    InnerInitPostImGui();

#ifdef _DEBUG
    ID3D11Debug* d3dDebug = nullptr;
    if(SUCCEEDED(device_->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug))) {
        ID3D11InfoQueue* d3dInfoQueue = nullptr;
        if(SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue))) {
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
            d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
            // d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_WARNING, true);

            D3D11_MESSAGE_ID hide[] = {
                D3D11_MESSAGE_ID_DEVICE_DRAW_RENDERTARGETVIEW_NOT_SET,
            };

            D3D11_INFO_QUEUE_FILTER filter = {};
            filter.DenyList.NumIDs = UINT(std::size(hide));
            filter.DenyList.pIDList = hide;
            d3dInfoQueue->AddStorageFilterEntries(&filter);
            d3dInfoQueue->Release();
        }
        d3dDebug->Release();
    }
#endif
}

void BaseCore::DisplayErrorPopup(const char* message) { errorPopupMessages_.push_back(message); }

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void BaseCore::Draw() {
    if(!active_)
        return;

    if(annotations_)
        annotations_->BeginEvent(AddonNameW.c_str());

    StateBackupD3D11 d3dstate;
    BackupD3D11State(context_.Get(), d3dstate);

    context_->OMSetRenderTargets(1, backBufferRTV_.GetAddressOf(), nullptr);

    {
        std::lock_guard guard(imguiInputMutex_);
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    // This is the closest we have to a reliable "update" function, so use it as one
    Update();

    if(firstFrame_) {
        firstFrame_ = false;
    }
    else {
        {
            std::lock_guard guard(imguiInputMutex_);
            auto& imguiInputs = Input::i().imguiInputs();
            Input::DelayedImguiInput dii;
            while (imguiInputs.try_pop(dii)) {
                ImGui_ImplWin32_WndProcHandler(gameWindow_, dii.msg, dii.wParam, dii.lParam);
            }
        }

        // Setup viewport
        D3D11_VIEWPORT vp;
        memset(&vp, 0, sizeof(D3D11_VIEWPORT));
        vp.Width = FLOAT(screenWidth_);
        vp.Height = FLOAT(screenHeight_);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.TopLeftX = vp.TopLeftY = 0;
        context_->RSSetViewports(1, &vp);

        InnerDraw();

        if(!errorPopupID_) {
            errorPopupTitle_ = std::format("{} Error", AddonName);
            errorPopupID_ = ImGui::GetID(errorPopupTitle_.c_str());
        }

        if(!errorPopupMessages_.empty() && !ImGui::IsPopupOpen(errorPopupTitle_.c_str()))
            ImGui::OpenPopup(errorPopupID_);

        ImGui::SetNextWindowPos(ImGui::GetIO().DisplaySize * 0.5f, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(screenWidth() * 0.25f, 0.f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(30.f, 30.f));
        if(ImGui::BeginPopupModal(errorPopupTitle_.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoScrollbar)) {
            ImGui::PushTextWrapPos();

            if(errorPopupMessages_.size() == 1)
                ImGui::Text("An error has occurred: %s\nAddon stability and functionality may be degraded.",
                            errorPopupMessages_[0].c_str());
            else {
                ImGui::TextUnformatted("Multiple errors have occurred:");
                for(const auto& s : errorPopupMessages_) {
                    ImGui::Bullet();
                    ImGui::TextUnformatted(s.c_str());
                }
                ImGui::TextUnformatted("Addon stability and functionality may be degraded.");
            }

            ImGui::Dummy(ImVec2(1.f, 30.f));

            constexpr ImVec2 buttonSize(100.f, 0.f);
            const f32 width = ImGui::GetWindowSize().x;
            const f32 centrePositionForButton = (width - buttonSize.x) / 2;
            ImGui::SetCursorPosX(centrePositionForButton);

            if(ImGui::Button("OK", buttonSize)) {
                errorPopupMessages_.clear();
                ImGui::CloseCurrentPopup();
            }

            ImGui::PopTextWrapPos();
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar();

        SettingsMenu::i().Draw();
        Log::i().Draw();

        if(!INIConfigurationFile::i().lastSaveError().empty() && INIConfigurationFile::i().lastSaveErrorChanged())
            ImGuiPopup("Configuration could not be saved!")
                .Position({ 0.5f, 0.45f })
                .Size({ 0.35f, 0.2f })
                .Display(
                    [&](const ImVec2&) {
                        ImGui::Text("Could not save addon configuration. Reason given was:");
                        ImGui::TextWrapped(INIConfigurationFile::i().lastSaveError().c_str());
                    },
                    []() { INIConfigurationFile::i().lastSaveErrorChanged(false); });
        if(!JSONConfigurationFile::i().lastSaveError().empty() && JSONConfigurationFile::i().lastSaveErrorChanged())
            ImGuiPopup("Configuration could not be saved!")
                .Position({ 0.5f, 0.45f })
                .Size({ 0.35f, 0.2f })
                .Display(
                    [&](const ImVec2&) {
                        ImGui::Text("Could not save addon configuration. Reason given was:");
                        ImGui::TextWrapped(JSONConfigurationFile::i().lastSaveError().c_str());
                    },
                    []() { JSONConfigurationFile::i().lastSaveErrorChanged(false); });

        if(UpdateCheck::i().updateAvailable() && !UpdateCheck::i().updateDismissed())
            ImGuiPopup("Update available!")
                .Position({ 0.5f, 0.45f })
                .Size({ 0.35f, 0.2f })
                .Display(
                    [&](const ImVec2& windowSize) {
                        ImGui::TextWrapped(std::format("A new version of {} has been released! "
                                                       "Please follow the link below to look at the changes and download the update. "
                                                       "Remember that you can always disable this version check in the settings.",
                                                       AddonName)
                                               .c_str());

                        ImGui::Spacing();
                        ImGui::SetCursorPosX(windowSize.x * 0.1f);

                        auto url = UpdateCheck::i().repoUrl(L"releases/latest");

                        if(ImGui::Button(utf8_encode(url).c_str(), ImVec2(windowSize.x * 0.8f, ImGui::GetFontSize() * 1.3f)))
                            ShellExecute(0, 0, url.c_str(), 0, 0, SW_SHOW);
                    },
                    []() { UpdateCheck::i().updateDismissed(true); });
    }

    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    RestoreD3D11State(context_.Get(), d3dstate);

    if(annotations_)
        annotations_->EndEvent();
}

void BaseCore::Update() {
    if(!active_)
        return;

    Input::i().OnUpdate();

    tickSkip_++;
    if(tickSkip_ >= TickSkipCount) {
        tickSkip_ -= TickSkipCount;
        MumbleLink::i().OnUpdate();
        InnerFrequentUpdate();
    }

    longTickSkip_++;
    if(longTickSkip_ >= LongTickSkipCount) {
        longTickSkip_ -= LongTickSkipCount;
        INIConfigurationFile::i().OnUpdate();
        GFXSettings::i().OnUpdate();
        UpdateCheck::i().CheckForUpdates();

        InnerInfrequentUpdate();
    }

    InnerUpdate();
}
