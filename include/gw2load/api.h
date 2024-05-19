#pragma once

/*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

/*
************************
* GW2Load API Overview *
************************
*
* 1. Folder structure
*
* Throughout this document, "." will refer to Guild Wars 2's root installation path.
* GW2Load will look for addons in subfolders of "./addons". DLLs found directly in the addons folder will be ignored.
* Subfolders starting with "." or "_" characters will also be skipped.
* Inside the subfolders, all DLLs will be checked for GW2Load exports. This is done *without* executing any code in the DLL.
*
* 2. Requirements
*
* The primary requirement to be recognized as an addon is a valid VERSIONINFO block with, at minimum:
*   - A valid ProductName (1st chance) or FileDescription (2nd chance) string field, which will be interpreted as the addon's name
*   - A valid PRODUCTVERSION (1st chance) or FILEVERSION (2nd chance) numerical field, which will be interpreted as the addon's version
* Additionally, ProductVersion (1st chance) or FileVersion (2nd chance) string fields will also be inspected and used for display (e.g., in logs),
* otherwise the version will fall back to the numerical field value (xx.yy.zz.ww).
*
* Only one export is required to be recognized as an addon:
*   unsigned int GW2Load_GetAddonAPIVersion();
*       Always return GW2Load_CurrentAddonAPIVersion in your implementation or zero to prevent loading.
*       Do NOT do any initialization in GW2Load_GetAddonAPIVersion!
*
* Additional exports will be detected at this time as well:
*   bool GW2Load_OnLoad(GW2Load_API* api, IDXGISwapChain* swapChain, ID3D11Device* device, ID3D11DeviceContext* context);
*   bool GW2Load_OnLoadLauncher(GW2Load_API* api);
*       The return value for either of these will be checked and the addon will be unloaded if it is false.
*   void GW2Load_OnClose();
*
* For advanced use only:
*   bool GW2Load_OnAddonDescriptionVersionOutdated(unsigned int loaderVersion);
*       This will only be called if GW2Load's addon description version is *older* than the addon's,
*       allowing the addon to adjust its behavior for the outdated loader. The loader will handle backwards compatibility automatically
*       (e.g., an addon using version 1 being loaded by a loader with version 2).
*
*   using GW2Load_UpdateCallback = void(*)(void* data, unsigned int sizeInBytes, bool dataIsFileName);
*   void GW2Load_UpdateCheck(GW2Load_UpdateAPI* api);
*       If the export is defined, UpdateCheck will be called *before* GetAddonDescription to alllow the addon the opportunity to self-update.
*       The provided callback may be called by the addon to signal to the loader that an update is pending.
*       The buffer provided by the addon will be copied by the loader so the addon can free the buffer immediately after the callback returns.
*       Two interpretations of the data buffer are available:
*         - If dataIsfileName is true, then data is cast as a C-string and interpreted as a path relative to the current DLL's location.
*           The addon will be unloaded and replaced with the new file, then reloaded.
*         - If dataIsfileName is false, then data is assumed to contain the new DLL's binary data in full.
*           The addon will be unloaded and overwritten by the new data, then reloaded.
*       These calls are asynchronous: if UpdateCheck exists, a thread will be spawned to perform the update check for each addon in parallel.
*       All UpdateCheck threads will be killed at most two seconds after the launcher is closed and the affected addons will be unloaded.
*
* Once API capabilities have been determined, the DLL will be unloaded.
* After all DLLs have been processed, they will be reloaded properly (via LoadLibrary) at the earliest opportunity.
* This is also when GetAddonDescription and UpdateCheck (if defined) will be invoked.
*
* Do *NOT* run initialization in DllMain. Use one of the OnLoad events instead.
* Similarly, it is recommended to run cleanup operations in the OnClose event.
*
* N.B. Remember to declare all of your exports extern "C"!
*
*/

inline static constexpr unsigned int GW2Load_AddonAPIVersionMagicFlag = 0xF0CF0000;
inline static constexpr unsigned int GW2Load_CurrentAddonAPIVersion = GW2Load_AddonAPIVersionMagicFlag | 1;

enum class GW2Load_HookedFunction : unsigned int
{
    Undefined = 0, // Reserved, do not use
    Present, // void Present(IDXGISwapChain* swapChain);
    ResizeBuffers, // void ResizeBuffers(IDXGISwapChain* swapChain, unsigned int width, unsigned int height, DXGI_FORMAT format);

    Count
};

enum class GW2Load_CallbackPoint : unsigned int
{
    Undefined = 0, // Reserved, do not use
    BeforeCall,
    AfterCall,

    Count
};

using GW2Load_GenericCallback = void(__cdecl*)(); // Used as a placeholder for the actual function signature, see enum
using GW2Load_PresentCallback = void(__cdecl*)(IDXGISwapChain* swapChain);
using GW2Load_ResizeBuffersCallback = void(__cdecl*)(IDXGISwapChain* swapChain, unsigned int width, unsigned int height, DXGI_FORMAT format);

using GW2Load_RegisterCallback = void(__cdecl*)(GW2Load_HookedFunction func, int priority, GW2Load_CallbackPoint callbackPoint, GW2Load_GenericCallback callback);
using GW2Load_UpdateCallback = void(__cdecl*)(void* data, unsigned int sizeInBytes, bool dataIsFileName);

struct GW2Load_API
{
#ifdef __cplusplus
    template<typename F>
    auto RegisterCallback(GW2Load_HookedFunction func, int priority, GW2Load_CallbackPoint callbackPoint, F callback)
    {
        GW2Load_GenericCallback cb = reinterpret_cast<GW2Load_GenericCallback>(+callback);
        registerCallback(func, priority, callbackPoint, cb);
    }
protected:
#endif
    GW2Load_RegisterCallback registerCallback;
};

struct GW2Load_UpdateAPI
{
    GW2Load_UpdateCallback updateCallback;
};

using GW2Load_GetAddonAPIVersion_t = unsigned int(__cdecl*)();
using GW2Load_OnLoad_t = bool(__cdecl*)(GW2Load_API* api, struct IDXGISwapChain* swapChain, struct ID3D11Device* device, struct ID3D11DeviceContext* context);
using GW2Load_OnLoadLauncher_t = bool(__cdecl*)(GW2Load_API* api);
using GW2Load_OnClose_t = void(__cdecl*)();
using GW2Load_OnAddonDescriptionVersionOutdated_t = bool(__cdecl*)(unsigned int loaderVersion);
using GW2Load_UpdateCheck_t = void(__cdecl*)(GW2Load_UpdateAPI* api);

/*
* 3. Standalone API
*
* GW2Load also offers a simple standalone C API to enumerate compatible addons in a directory and return basic information about them.
* This API can be accessed as if the loader were a standard library.
*
*/

#ifdef GW2LOAD
#define GW2LOAD_EXPORT __declspec(dllexport)
#else
#define GW2LOAD_EXPORT __declspec(dllimport)
#endif

struct GW2Load_EnumeratedAddon
{
    const char* path;
    const char* name;
    unsigned short majorAddonVersion = 0;
    unsigned short minorAddonVersion = 0;
    unsigned short patchAddonVersion = 0;
    unsigned short fixAddonVersion = 0;
};

extern "C"
{
    // If allowDisabled is true, addons ending in a .dll.disabled extension will also be returned
    GW2LOAD_EXPORT GW2Load_EnumeratedAddon* GW2Load_GetAddonsInDirectory(const char* directory, unsigned int* count, bool allowDisabled);
}