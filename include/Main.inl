#include <gw2load/api.h>

std::ofstream g_logStream;
HMODULE       g_hModule;

const char* GetAddonName()
{
    return ADDON_NAME;
}

const wchar_t* GetAddonNameW()
{
    return TEXT(ADDON_NAME);
}

const char* GetAddonVersionString()
{
    return GIT_VER_STR;
}

u64 GetAddonVersion()
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

    __declspec(dllexport) bool GW2Load_OnLoad(GW2Load_API* api, IDXGISwapChain* swapChain, ID3D11Device* device, ID3D11DeviceContext* context)
    {
        DXGI_SWAP_CHAIN_DESC desc;
        swapChain->GetDesc(&desc);
        auto logName = std::format("addons/_logs/{}.log", ToLower(ADDON_NAME));
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