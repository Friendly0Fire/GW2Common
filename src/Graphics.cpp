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

void BackupD3D11State(ID3D11DeviceContext* ctx, StateBackupD3D11& old) {
    old.ScissorRectsCount = old.ViewportsCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    ctx->RSGetScissorRects(&old.ScissorRectsCount, old.ScissorRects);
    ctx->RSGetViewports(&old.ViewportsCount, old.Viewports);
    ctx->RSGetState(&old.RS);
    ctx->OMGetBlendState(&old.BlendState, old.BlendFactor, &old.SampleMask);
    ctx->OMGetDepthStencilState(&old.DepthStencilState, &old.StencilRef);
    ctx->PSGetShaderResources(0, 1, &old.PSShaderResource);
    ctx->PSGetSamplers(0, 1, &old.PSSampler);
    old.PSInstancesCount = old.VSInstancesCount = old.GSInstancesCount = 256;
    ctx->PSGetShader(&old.PS, old.PSInstances, &old.PSInstancesCount);
    ctx->VSGetShader(&old.VS, old.VSInstances, &old.VSInstancesCount);
    ctx->VSGetConstantBuffers(0, 1, &old.VSConstantBuffer);
    ctx->GSGetShader(&old.GS, old.GSInstances, &old.GSInstancesCount);

    ctx->IAGetPrimitiveTopology(&old.PrimitiveTopology);
    ctx->IAGetIndexBuffer(&old.IndexBuffer, &old.IndexBufferFormat, &old.IndexBufferOffset);
    ctx->IAGetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride, &old.VertexBufferOffset);
    ctx->IAGetInputLayout(&old.InputLayout);

    ctx->OMGetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, old.RenderTargets, &old.DepthStencil);
}

void RestoreD3D11State(ID3D11DeviceContext* ctx, const StateBackupD3D11& old) {
    ctx->RSSetScissorRects(old.ScissorRectsCount, old.ScissorRects);
    ctx->RSSetViewports(old.ViewportsCount, old.Viewports);
    ctx->RSSetState(old.RS);
    if(old.RS)
        old.RS->Release();
    ctx->OMSetBlendState(old.BlendState, old.BlendFactor, old.SampleMask);
    if(old.BlendState)
        old.BlendState->Release();
    ctx->OMSetDepthStencilState(old.DepthStencilState, old.StencilRef);
    if(old.DepthStencilState)
        old.DepthStencilState->Release();
    ctx->PSSetShaderResources(0, 1, &old.PSShaderResource);
    if(old.PSShaderResource)
        old.PSShaderResource->Release();
    ctx->PSSetSamplers(0, 1, &old.PSSampler);
    if(old.PSSampler)
        old.PSSampler->Release();
    ctx->PSSetShader(old.PS, old.PSInstances, old.PSInstancesCount);
    if(old.PS)
        old.PS->Release();
    for(UINT i = 0; i < old.PSInstancesCount; i++)
        if(old.PSInstances[i])
            old.PSInstances[i]->Release();
    ctx->VSSetShader(old.VS, old.VSInstances, old.VSInstancesCount);
    if(old.VS)
        old.VS->Release();
    ctx->VSSetConstantBuffers(0, 1, &old.VSConstantBuffer);
    if(old.VSConstantBuffer)
        old.VSConstantBuffer->Release();
    ctx->GSSetShader(old.GS, old.GSInstances, old.GSInstancesCount);
    if(old.GS)
        old.GS->Release();
    for(UINT i = 0; i < old.VSInstancesCount; i++)
        if(old.VSInstances[i])
            old.VSInstances[i]->Release();
    ctx->IASetPrimitiveTopology(old.PrimitiveTopology);
    ctx->IASetIndexBuffer(old.IndexBuffer, old.IndexBufferFormat, old.IndexBufferOffset);
    if(old.IndexBuffer)
        old.IndexBuffer->Release();
    ctx->IASetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride, &old.VertexBufferOffset);
    if(old.VertexBuffer)
        old.VertexBuffer->Release();
    ctx->IASetInputLayout(old.InputLayout);
    if(old.InputLayout)
        old.InputLayout->Release();

    ctx->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, old.RenderTargets, old.DepthStencil);
    if(old.DepthStencil)
        old.DepthStencil->Release();
    for(i32 i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
        if(old.RenderTargets[i])
            old.RenderTargets[i]->Release();
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
