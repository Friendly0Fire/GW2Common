#include "Graphics.h"

#include <FileSystem.h>
#include <DirectXTK/DDSTextureLoader.h>
#include <DirectXTK/WICTextureLoader.h>
#include <renderdoc_app.h>

#include "Utility.h"

RenderTarget MakeRenderTarget(ComPtr<ID3D11Device>& dev, u32 width, u32 height, DXGI_FORMAT fmt, UINT mips, bool generateMips) {
    RenderTarget rt;
    rt.width = width;
    rt.height = height;
    D3D11_TEXTURE2D_DESC desc;
    desc.Format = fmt;
    desc.Width = width;
    desc.Height = height;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    desc.MipLevels = mips;
    desc.ArraySize = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = generateMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    GW2_CHECKED_HRESULT(dev->CreateTexture2D(&desc, nullptr, &rt.texture));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = fmt;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = -1;
    srvDesc.Texture2D.MostDetailedMip = 0;
    GW2_CHECKED_HRESULT(dev->CreateShaderResourceView(rt.texture.Get(), &srvDesc, &rt.srv));

    D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
    rtvDesc.Format = fmt;
    rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rtvDesc.Texture2D.MipSlice = 0;
    GW2_CHECKED_HRESULT(dev->CreateRenderTargetView(rt.texture.Get(), &rtvDesc, &rt.rtv));

    return rt;
}

template<typename T>
T MakeTexture(ComPtr<ID3D11Device>& dev, u32 width, u32 height, u32 depth, DXGI_FORMAT fmt, UINT mips, bool generateMips) {
    constexpr bool is1D = std::is_same_v<T, Texture1D>;
    constexpr bool is2D = std::is_same_v<T, Texture2D>;
    constexpr bool is3D = std::is_same_v<T, Texture3D>;

    T tex;
    tex.width = width;
    if constexpr(is2D)
        tex.height = height;
    if constexpr (is3D)
        tex.depth = depth;

    std::conditional_t<is1D, D3D11_TEXTURE1D_DESC, std::conditional_t<is2D, D3D11_TEXTURE2D_DESC, D3D11_TEXTURE3D_DESC>> desc;
    desc.Format = fmt;
    desc.Width = width;
    if constexpr(!is1D)
        desc.Height = height;
    if constexpr(is3D)
        desc.Depth = depth;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.MipLevels = mips;
    if constexpr(!is3D)
        desc.ArraySize = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.CPUAccessFlags = 0;
    desc.MiscFlags = generateMips ? D3D11_RESOURCE_MISC_GENERATE_MIPS : 0;
    if constexpr(is2D) {
        desc.SampleDesc.Count = 1;
        desc.SampleDesc.Quality = 0;
    }
    if constexpr(is1D)
        GW2_CHECKED_HRESULT(dev->CreateTexture1D(&desc, nullptr, &tex.texture));
    else if constexpr(is2D)
        GW2_CHECKED_HRESULT(dev->CreateTexture2D(&desc, nullptr, &tex.texture));
    else
        GW2_CHECKED_HRESULT(dev->CreateTexture3D(&desc, nullptr, &tex.texture));

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    srvDesc.Format = fmt;
    if constexpr(is1D)
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
    else if constexpr(is2D)
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    else
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
    srvDesc.Texture2D.MipLevels = -1;
    srvDesc.Texture2D.MostDetailedMip = 0;
    GW2_CHECKED_HRESULT(dev->CreateShaderResourceView(tex.texture.Get(), &srvDesc, &tex.srv));

    return tex;
}

template Texture1D MakeTexture<Texture1D>(ComPtr<ID3D11Device>& dev, u32 width, u32 height, u32 depth, DXGI_FORMAT fmt, UINT mips,
                                                bool generateMips);
template Texture2D MakeTexture<Texture2D>(ComPtr<ID3D11Device>& dev, u32 width, u32 height, u32 depth, DXGI_FORMAT fmt, UINT mips,
                                                bool generateMips);
template Texture3D MakeTexture<Texture3D>(ComPtr<ID3D11Device>& dev, u32 width, u32 height, u32 depth, DXGI_FORMAT fmt, UINT mips,
                                                bool generateMips);

std::pair<ComPtr<ID3D11Resource>, ComPtr<ID3D11ShaderResourceView>> CreateResourceFromFile(ID3D11Device* pDev, ID3D11DeviceContext* pCtx, const std::filesystem::path& path) {
    ComPtr<ID3D11Resource> res;
    ComPtr<ID3D11ShaderResourceView> srv;

    const auto data = FileSystem::ReadFile(path);
    GW2_ASSERT(!data.empty());

    if(path.extension() == ".dds") {
        auto hr = DirectX::CreateDDSTextureFromMemoryEx(pDev, data.data(), data.size(), 4096, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, 0,
                                                      0, DirectX::DDS_LOADER_DEFAULT, &res, &srv);
        GW2_ASSERT(SUCCEEDED(hr));
    } else {
        auto hr = DirectX::CreateWICTextureFromMemoryEx(pDev, pCtx, data.data(), data.size(), 4096, D3D11_USAGE_DEFAULT,
                                                      D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, 0,
                                                      D3D11_RESOURCE_MISC_GENERATE_MIPS, DirectX::WIC_LOADER_DEFAULT, &res, &srv);
        GW2_ASSERT(SUCCEEDED(hr));
    }

    return { res, srv };
}

Texture1D Detail::SetTextureSize(Texture1D&& tex) {
    D3D11_TEXTURE1D_DESC desc;
    tex.texture->GetDesc(&desc);
    tex.width = desc.Width;

    return std::move(tex);
}

Texture2D Detail::SetTextureSize(Texture2D&& tex) {
    D3D11_TEXTURE2D_DESC desc;
    tex.texture->GetDesc(&desc);
    tex.width = desc.Width;
    tex.height = desc.Height;

    return std::move(tex);
}

Texture3D Detail::SetTextureSize(Texture3D&& tex) {
    D3D11_TEXTURE3D_DESC desc;
    tex.texture->GetDesc(&desc);
    tex.width = desc.Width;
    tex.height = desc.Height;
    tex.depth = desc.Depth;

    return std::move(tex);
}

std::pair<ComPtr<ID3D11Resource>, ComPtr<ID3D11ShaderResourceView>> CreateResourceFromResource(ID3D11Device* pDev, HMODULE hModule,
                                                                                               unsigned uResource) {
    const auto resourceSpan = LoadResource(hModule, uResource);
    if(resourceSpan.data() == nullptr)
        return { nullptr, nullptr };

    ComPtr<ID3D11Resource> res;
    ComPtr<ID3D11ShaderResourceView> srv;

    auto hr = DirectX::CreateDDSTextureFromMemory(pDev, resourceSpan.data(), resourceSpan.size_bytes(), &res, &srv);
    GW2_ASSERT(SUCCEEDED(hr));

    return { res, srv };
}

void DrawScreenQuad(ID3D11DeviceContext* ctx) {
    ctx->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
    ctx->IASetInputLayout(NULL);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    ctx->Draw(4, 0);
}

StateBackupD3D11::StateBackupD3D11(ID3D11DeviceContext* ctx, Config&& cfg)
    : Context(ctx) {
    UINT count = 0;
    ctx->RSGetScissorRects(&count, nullptr);
    if(count > 0) {
        ScissorRects.resize(count);
        ctx->RSGetScissorRects(&count, ScissorRects.data());
    }

    count = 0;
    ctx->RSGetViewports(&count, nullptr);
    if(count > 0) {
        Viewports.resize(count);
        ctx->RSGetViewports(&count, Viewports.data());
    }

    ctx->RSGetState(&RS);
    ctx->OMGetBlendState(&BlendState, BlendFactor.data(), &SampleMask);
    ctx->OMGetDepthStencilState(&DepthStencilState, &StencilRef);

#define BACKUP_SHADER_STAGE(Name, name) \
    { \
        if(cfg.name.constantBufferCount > 0) { \
            Name.ConstantBuffers.resize(cfg.name.constantBufferCount); \
            ctx->Name##GetConstantBuffers(0, Name.ConstantBuffers.size(), Name.ConstantBuffers.data()); \
        } \
        if(cfg.name.samplerCount > 0) { \
            Name.Samplers.resize(cfg.name.samplerCount); \
            ctx->Name##GetSamplers(0, Name.Samplers.size(), Name.Samplers.data()); \
        } \
        if(cfg.name.shaderResourceCount > 0) { \
            Name.ShaderResources.resize(cfg.name.shaderResourceCount); \
            ctx->Name##GetShaderResources(0, Name.ShaderResources.size(), Name.ShaderResources.data()); \
        } \
        count = 0; \
        ctx->Name##GetShader(&Name.Shader, nullptr, &count); \
        Name.Instances.resize(count); \
        ctx->Name##GetShader(&Name.Shader, Name.Instances.data(), &count); \
    }

    BACKUP_SHADER_STAGE(VS, vs);
    BACKUP_SHADER_STAGE(GS, gs);
    BACKUP_SHADER_STAGE(PS, ps);

#undef BACKUP_SHADER_STAGE

    ctx->IAGetPrimitiveTopology(&PrimitiveTopology);
    ctx->IAGetIndexBuffer(&IndexBuffer, &IndexBufferFormat, &IndexBufferOffset);
    VertexBuffers.resize(cfg.vertexBufferCount);
    VertexBufferStrides.resize(cfg.vertexBufferCount);
    VertexBufferOffsets.resize(cfg.vertexBufferCount);
    ctx->IAGetVertexBuffers(0, static_cast<UINT>(VertexBuffers.size()), VertexBuffers.data(), VertexBufferStrides.data(), VertexBufferOffsets.data());
    ctx->IAGetInputLayout(&InputLayout);

    ctx->OMGetRenderTargets(static_cast<UINT>(RenderTargets.size()), RenderTargets.data(), &DepthStencil);
}

StateBackupD3D11::~StateBackupD3D11() {
    Context->RSSetScissorRects(static_cast<UINT>(ScissorRects.size()), ScissorRects.data());
    Context->RSSetViewports(static_cast<UINT>(Viewports.size()), Viewports.data());

    Context->RSSetState(RS.Get());
    Context->OMSetBlendState(BlendState.Get(), BlendFactor.data(), SampleMask);
    Context->OMSetDepthStencilState(DepthStencilState.Get(), StencilRef);

#define RESTORE_SHADER_STAGE(Name) \
    { \
        if(!Name.ConstantBuffers.empty()) \
            Context->Name##SetConstantBuffers(0, static_cast<UINT>(Name.ConstantBuffers.size()), Name.ConstantBuffers.data()); \
        if(!Name.Samplers.empty()) \
            Context->Name##SetSamplers(0, static_cast<UINT>(Name.Samplers.size()), Name.Samplers.data()); \
        if(!Name.ShaderResources.empty()) \
            Context->Name##SetShaderResources(0, static_cast<UINT>(Name.ShaderResources.size()), Name.ShaderResources.data()); \
        Context->Name##SetShader(Name.Shader.Get(), Name.Instances.data(), static_cast<UINT>(Name.Instances.size())); \
        for(auto* cb : Name.ConstantBuffers) \
            if(cb) cb->Release(); \
        for(auto* s : Name.Samplers) \
            if(s) s->Release(); \
        for(auto* sr : Name.ShaderResources) \
            if(sr) sr->Release(); \
        for(auto* i : Name.Instances) \
            if(i) i->Release(); \
    }

    RESTORE_SHADER_STAGE(VS);
    RESTORE_SHADER_STAGE(GS);
    RESTORE_SHADER_STAGE(PS);

#undef RESTORE_SHADER_STAGE

    Context->IASetPrimitiveTopology(PrimitiveTopology);
    Context->IASetIndexBuffer(IndexBuffer.Get(), IndexBufferFormat, IndexBufferOffset);
    Context->IASetVertexBuffers(0, static_cast<UINT>(VertexBuffers.size()), VertexBuffers.data(), VertexBufferStrides.data(), VertexBufferOffsets.data());
    Context->IASetInputLayout(InputLayout.Get());

    for(auto* vb : VertexBuffers)
        if(vb) vb->Release();

    Context->OMSetRenderTargets(static_cast<UINT>(RenderTargets.size()), RenderTargets.data(), DepthStencil.Get());

    for(auto* rt : RenderTargets)
        if(rt) rt->Release();
}

RENDERDOC_API_1_5_0* RenderDocCapture::rdoc_ = nullptr;
ComPtr<ID3D11Device> RenderDocCapture::dev_;

void RenderDocCapture::Init(ComPtr<ID3D11Device>& dev) {
#if _DEBUG
    if(HMODULE mod = GetModuleHandleA("renderdoc.dll")) {
        pRENDERDOC_GetAPI RENDERDOC_GetAPI = (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
        i32 ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_5_0, (void**)&rdoc_);
        if(ret != 1)
            rdoc_ = nullptr;

        if(rdoc_)
            dev_ = dev;
    }
#endif
}

RenderDocCapture::RenderDocCapture() {
    if(!rdoc_)
        return;
    LogDebug("Beginning RenderDoc frame capture...");

    rdoc_->StartFrameCapture(dev_.Get(), nullptr);
}

RenderDocCapture::~RenderDocCapture() {
    if(!rdoc_)
        return;
    LogDebug("Ending RenderDoc frame capture...");

    rdoc_->EndFrameCapture(dev_.Get(), nullptr);
}
