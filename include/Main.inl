#include <gw2load/api.h>

std::ofstream g_logStream;
HMODULE       g_hModule;

extern "C"
{
    __declspec(dllexport) bool GW2Load_GetAddonDescription(GW2Load_AddonDescription* desc)
    {
        desc->descriptionVersion = GW2Load_CurrentAddonDescriptionVersion;
        const auto& ver = GetAddonVersion();
        desc->majorAddonVersion = ver.major;
        desc->minorAddonVersion = ver.minor;
        desc->patchAddonVersion = ver.patch;
        desc->name = GetAddonName();

        return true;
    }

    __declspec(dllexport) bool GW2Load_OnLoad(GW2Load_API* api, IDXGISwapChain* swapChain, ID3D11Device* device, ID3D11DeviceContext* context)
    {
        DXGI_SWAP_CHAIN_DESC desc;
        swapChain->GetDesc(&desc);
        auto logName = std::format("addons/_logs/{}.log", ToLower(GetAddonName()));
        g_logStream = std::ofstream(logName.c_str());
        BaseCore::Init(g_hModule, api);
        GetBaseCore().PostCreateSwapChain(desc.OutputWindow, device, swapChain);

        api->RegisterCallback(GW2Load_HookedFunction::Present, 0, GW2Load_CallbackPoint::BeforeCall, [](IDXGISwapChain* swapChain) { GetBaseCore().Draw(); });

        api->RegisterCallback(GW2Load_HookedFunction::ResizeBuffers, 0, GW2Load_CallbackPoint::BeforeCall,
            [](IDXGISwapChain* swapChain, unsigned int width, unsigned int height, DXGI_FORMAT format) { GetBaseCore().PreResizeSwapChain(); });

        api->RegisterCallback(GW2Load_HookedFunction::ResizeBuffers, 0, GW2Load_CallbackPoint::AfterCall,
            [](IDXGISwapChain* swapChain, unsigned int width, unsigned int height, DXGI_FORMAT format) { GetBaseCore().PostResizeSwapChain(width, height); });

        return true;
    }

    __declspec(dllexport) void GW2Load_OnClose()
    {
        BaseCore::Shutdown();
    }
}


bool WINAPI DllMain(HMODULE hModule, DWORD fdwReason, LPVOID lpReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        g_hModule = hModule;
        break;
    default:;
    }

    return true;
}