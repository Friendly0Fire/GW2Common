#include "ShaderManager.h"

#include <fstream>
#include <sstream>

#include <d3dcompiler.h>

#include "FileSystem.h"
#include "Utility.h"

class ShaderInclude final : public ID3DInclude
{
    std::unordered_map<LPCVOID, std::vector<byte>> openFiles_;

public:
    virtual ~ShaderInclude() = default;
    ShaderInclude() = default;

    HRESULT COM_DECLSPEC_NOTHROW Open([[maybe_unused]] D3D_INCLUDE_TYPE includeType, LPCSTR pFileName, [[maybe_unused]] LPCVOID pParentData,
                                      LPCVOID* ppData, UINT* pBytes) override {
        LogDebug("Opening shader include '{}'...", pFileName);
        auto file = ShaderManager::i().shadersZip_->getEntry(pFileName);
        if(file.isNull()) {
            LogDebug("Include '{}' not found in archive!", pFileName);
            return E_INVALIDARG;
        }

        auto data = FileSystem::ReadFile(file);
        *ppData = data.data();
        *pBytes = UINT(data.size());

        openFiles_[data.data()] = std::move(data);
        return S_OK;

        return E_FAIL;
    }

    HRESULT COM_DECLSPEC_NOTHROW Close(LPCVOID pData) override {
        openFiles_.erase(pData);
        return S_OK;
    }
};

ShaderManager::ShaderManager(ComPtr<ID3D11Device>& device, u32 shaderResourceID, HMODULE shaderResourceModule,
                             const std::filesystem::path& shadersPath)
    : device_(device), shaderResourceID_(shaderResourceID), shaderResourceModule_(shaderResourceModule), shadersPath_(shadersPath) {
    CheckHotReload();
}

ShaderManager::~ShaderManager()
{
    if (shadersZip_)
    {
        ZipArchive::free(shadersZip_);
        shadersZip_ = nullptr;
    }
}

void ShaderManager::SetShaders(ID3D11DeviceContext* ctx, ShaderId vs, ShaderId ps) {
    ctx->PSSetShader(std::get<ComPtr<ID3D11PixelShader>>(shaders_[ps.id].shader).Get(), nullptr, 0);
    ctx->VSSetShader(std::get<ComPtr<ID3D11VertexShader>>(shaders_[vs.id].shader).Get(), nullptr, 0);
}

ShaderId ShaderManager::GetShader(const std::wstring& filename, D3D11_SHADER_VERSION_TYPE st, const std::string& entrypoint,
                                  std::optional<std::vector<std::string>> macros) {
    LogDebug(L"Looking for shader {}:{} (type #{})", filename, utf8_decode(entrypoint), i32(st));

    for(u32 i = 0; i < shaders_.size(); i++) {
        auto& sd = shaders_[i];
        if(sd.filename == filename && sd.entrypoint == entrypoint && (macros && sd.macros == *macros || !macros && sd.macros.empty()))
            return { i };
    }

    auto shader = CompileShader(filename, st, entrypoint, macros);
    u32 id = u32(shaders_.size());
    shaders_.push_back({
        .shader = shader,
        .filename = filename,
        .st = st,
        .entrypoint = entrypoint,
    });

    if(macros)
        std::swap(*macros, shaders_.back().macros);

    return { id };
}

void ShaderManager::ReloadAll() {
#ifdef HOT_RELOAD_SHADERS
    if(!IsDebuggerPresent())
        return;

    for(auto& sd : shaders_)
        sd.shader = CompileShader(sd.filename, sd.st, sd.entrypoint, sd.macros);
#endif
}

[[nodiscard]] std::string ShaderManager::LoadShaderFile(const std::wstring& filename) {
    LoadShadersArchive();

    if(hotReloadFolderExists_ && FileSystem::Exists(GetShaderFilename(filename))) {
        LogDebug(L"Hot reloading shader file {}", filename);
        std::ifstream file(GetShaderFilename(filename));
        auto vec = FileSystem::ReadFile(file);
        return std::string(reinterpret_cast<char*>(vec.data()), vec.size());
    } else {
        LogDebug(L"Looking for shader {} in archive", filename);
        auto file = shadersZip_->getEntry(EncodeShaderFilename(filename));
        if(file.isNull())
            LogError(L"Shader file {} not found in archive!", filename);
        GW2_ASSERT(!file.isNull());

        return FileSystem::ReadFileAsText(file);
    }
}

ComPtr<ID3D11Buffer> ShaderManager::MakeConstantBuffer(size_t dataSize, const void* data) {
    LogDebug("Creating constant buffer of {} bytes (initial data: {})", dataSize, data ? "yes" : "no");

    dataSize = RoundUp(dataSize, 16);
    D3D11_BUFFER_DESC desc { .ByteWidth = u32(dataSize),
                             .Usage = D3D11_USAGE_DYNAMIC,
                             .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
                             .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
                             .MiscFlags = 0,
                             .StructureByteStride = 0 };
    D3D11_SUBRESOURCE_DATA idata { .pSysMem = data, .SysMemPitch = u32(dataSize), .SysMemSlicePitch = 0 };
    ComPtr<ID3D11Buffer> buf;
    GW2_CHECKED_HRESULT(device_->CreateBuffer(&desc, data ? &idata : nullptr, buf.GetAddressOf()));

    return buf;
}

void HandleFailedShaderCompile(HRESULT hr, ID3DBlob* errors) {
    if(SUCCEEDED(hr))
        return;

    LogError(L"Compilation failed: 0x{:x}", u32(hr));

    if(errors) {
        const char* errorsText = static_cast<const char*>(errors->GetBufferPointer());

        LogError("Compilation errors:\n{}", errorsText);
    }

#ifndef _DEBUG
    if(hr == 0x88760b59)
        CriticalMessageBox(
            L"Fatal error: outdated shader compiler. Please install Windows Update KB4019990 or upgrade to a more modern operating "
            L"system.");
    else
        CriticalMessageBox(
            L"Fatal error: shader compilation failed! Error code was 0x%X. Please report this to "
            L"https://github.com/Friendly0Fire/GW2Common/issues",
            hr);
#endif

    // Break to fix errors
    GW2_ASSERT(errors == nullptr);
}

[[nodiscard]] ShaderManager::AnyShaderComPtr ShaderManager::CompileShader(const std::wstring& filename, D3D11_SHADER_VERSION_TYPE st,
                                                                          const std::string& entrypoint,
                                                                          std::optional<std::vector<std::string>> macros) {
    LogDebug(L"Compiling shader {}:{} (type #{})", filename, utf8_decode(entrypoint), i32(st));

    ComPtr<ID3DBlob> blob = nullptr;
    while(blob == nullptr) {
        auto shaderContents = LoadShaderFile(filename);
        std::string filePath = EncodeShaderFilename(filename);
        auto* includePtr = GetIncludeManager();

        std::vector<D3D_SHADER_MACRO> d3dMacros;
        d3dMacros.reserve((macros ? macros->size() : 0) + 1);
        if(macros) {
            for(auto& m : *macros) {
                size_t split = m.find('=');

                if(split != std::string::npos) {
                    m[split] = '\0';
                    d3dMacros.emplace_back(m.c_str(), m.c_str() + split + 1);
                }
                else
                    d3dMacros.emplace_back(m.c_str(), "1");
            }
        }
        d3dMacros.emplace_back(nullptr, nullptr);

        ComPtr<ID3DBlob> errors = nullptr;
        const auto hr =
            D3DCompile(shaderContents.data(), shaderContents.size(), filePath.c_str(), d3dMacros.data(), includePtr, entrypoint.c_str(),
                       st == D3D11_SHVER_PIXEL_SHADER ? "ps_4_0" : "vs_4_0", 0, 0, blob.GetAddressOf(), errors.GetAddressOf());

        HandleFailedShaderCompile(hr, errors.Get());
        errors.Reset();
    }

    if(st == D3D11_SHVER_PIXEL_SHADER) {
        ComPtr<ID3D11PixelShader> ps;
        GW2_CHECKED_HRESULT(device_->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, ps.GetAddressOf()));
        return ps;
    }
    else {
        ComPtr<ID3D11VertexShader> vs;
        GW2_CHECKED_HRESULT(device_->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, vs.GetAddressOf()));
        return vs;
    }
}

void ShaderManager::LoadShadersArchive() {
    if(shadersZip_ || !shaderResourceID_)
        return;

    LogDebug("Loading shader archive...");
    const auto data = LoadResource(shaderResourceModule_, shaderResourceID_);

    auto* const iss = new std::istringstream(std::string(reinterpret_cast<char*>(data.data()), data.size()), std::ios_base::binary);

    shadersZip_ = ZipArchive::fromBuffer(data.data(), static_cast<u32>(data.size_bytes()));

    shaderIncludeManager_ = std::make_unique<ShaderInclude>();
}

std::wstring ShaderManager::GetShaderFilename(const std::wstring& filename) const {
#ifdef HOT_RELOAD_SHADERS
    if(hotReloadFolderExists_)
        return (shadersPath_ / filename).wstring();
#endif
    return filename;
}

ID3DInclude* ShaderManager::GetIncludeManager() const {
#ifdef HOT_RELOAD_SHADERS
    if(hotReloadFolderExists_)
        return D3D_COMPILE_STANDARD_FILE_INCLUDE;
#endif
    return shaderIncludeManager_.get();
}

void ShaderManager::CheckHotReload() {
#ifdef HOT_RELOAD_SHADERS
    hotReloadFolderExists_ = std::filesystem::exists(shadersPath_);
#endif
}

void ConstantBufferBase::Upload(ID3D11DeviceContext* ctx, void* data, size_t size) {
    D3D11_MAPPED_SUBRESOURCE map;
    ctx->Map(buf.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &map);
    memcpy_s(map.pData, size, data, size);
    ctx->Unmap(buf.Get(), 0);
}
