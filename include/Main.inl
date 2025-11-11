#include <dxgi.h>
#include <gw2load/api.h>

std::ofstream g_logStream;
HMODULE       g_hModule;

static constexpr u64 GetAddonVersion()
{
    static std::array<u64, 4> ver = { GIT_VER };
    return (ver[0] << 16 * 3) + (ver[1] << 16 * 2) + (ver[2] << 16 * 1) + (ver[3] << 16 * 0);
}

extern "C"
{
    __declspec(dllexport) unsigned int GW2Load_GetAddonAPIVersion()
    {
        return GW2Load_CurrentAddonAPIVersion;
    }

    __declspec(dllexport) bool GW2Load_OnLoad(HMODULE gw2loadHandle, IDXGISwapChain* swapChain, ID3D11Device* device, ID3D11DeviceContext* context)
    {
        GW2Load_API          api(gw2loadHandle);
        DXGI_SWAP_CHAIN_DESC desc;
        swapChain->GetDesc(&desc);
        auto logName = std::format("addons/_logs/{}.log", ToLower(AddonName));
        g_logStream = std::ofstream(logName.c_str());
        BaseCore::Init(g_hModule, api);
        GetBaseCore().PostCreateSwapChain(desc.OutputWindow, device, swapChain);

        api.RegisterCallback(GW2Load_HookedFunction::Present, 0, GW2Load_CallbackPoint::BeforeCall, [](IDXGISwapChain* swapChain) { GetBaseCore().Draw(); });

        api.RegisterCallback(GW2Load_HookedFunction::ResizeBuffers, 0, GW2Load_CallbackPoint::BeforeCall,
            [](IDXGISwapChain* swapChain, unsigned int width, unsigned int height, DXGI_FORMAT format) { GetBaseCore().PreResizeSwapChain(); });

        api.RegisterCallback(GW2Load_HookedFunction::ResizeBuffers, 0, GW2Load_CallbackPoint::AfterCall,
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