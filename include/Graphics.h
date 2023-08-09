#pragma once

#include <d3d11.h>
#include <renderdoc_app.h>

#include "Common.h"

template<typename T>
struct Texture
{
    ComPtr<T> texture;
    ComPtr<ID3D11ShaderResourceView> srv;
};
using Texture1D = Texture<ID3D11Texture1D>;
using Texture2D = Texture<ID3D11Texture2D>;
using Texture3D = Texture<ID3D11Texture3D>;

struct RenderTarget : public Texture<ID3D11Texture2D>
{
    ComPtr<ID3D11RenderTargetView> rtv;

    RenderTarget& operator=(const Texture2D& tex) {
        texture = tex.texture;
        srv = tex.srv;

        return *this;
    }
};

RenderTarget MakeRenderTarget(ComPtr<ID3D11Device>& dev, u32 width, u32 height, DXGI_FORMAT fmt, UINT mips = 1,
                              bool generateMips = false);
template<typename T>
Texture<T> MakeTexture(ComPtr<ID3D11Device>& dev, u32 width, u32 height, u32 depth, DXGI_FORMAT fmt, UINT mips = 1,
                       bool generateMips = false);

struct DepthStencil : public Texture<ID3D11Texture2D>
{
    ComPtr<ID3D11DepthStencilView> rtv;
};

std::pair<ComPtr<ID3D11Resource>, ComPtr<ID3D11ShaderResourceView>> CreateResourceFromResource(ID3D11Device* pDev, HMODULE hModule,
                                                                                               unsigned uResource);
std::pair<ComPtr<ID3D11Resource>, ComPtr<ID3D11ShaderResourceView>> CreateResourceFromFile(ID3D11Device* pDev, ID3D11DeviceContext* pCtx,
                                                                                           const std::filesystem::path& path);

template<typename T = ID3D11Texture2D>
Texture<T> CreateTextureFromResource(ID3D11Device* pDev, HMODULE hModule, unsigned uResource) {
    auto [res, srv] = CreateResourceFromResource(pDev, hModule, uResource);

    ComPtr<T> tex;
    res->QueryInterface(tex.GetAddressOf());
    GW2_ASSERT(tex != nullptr);

    return { tex, srv };
}

template<typename T = ID3D11Texture2D>
Texture<T> CreateTextureFromFile(ID3D11Device* pDev, ID3D11DeviceContext* pCtx, const std::filesystem::path& path) {
    auto [res, srv] = CreateResourceFromFile(pDev, pCtx, path);

    ComPtr<T> tex;
    res->QueryInterface(tex.GetAddressOf());
    GW2_ASSERT(tex != nullptr);

    return { tex, srv };
}

void DrawScreenQuad(ID3D11DeviceContext* ctx);

struct StateBackupD3D11
{
    UINT ScissorRectsCount, ViewportsCount;
    D3D11_RECT ScissorRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    D3D11_VIEWPORT Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    ID3D11RasterizerState* RS;
    ID3D11BlendState* BlendState;
    FLOAT BlendFactor[4];
    UINT SampleMask;
    UINT StencilRef;
    ID3D11DepthStencilState* DepthStencilState;
    ID3D11ShaderResourceView* PSShaderResource;
    ID3D11SamplerState* PSSampler;
    ID3D11PixelShader* PS;
    ID3D11VertexShader* VS;
    ID3D11GeometryShader* GS;
    UINT PSInstancesCount, VSInstancesCount, GSInstancesCount;
    ID3D11ClassInstance *PSInstances[256], *VSInstances[256], *GSInstances[256]; // 256 is max according to PSSetShader documentation
    D3D11_PRIMITIVE_TOPOLOGY PrimitiveTopology;
    ID3D11Buffer *IndexBuffer, *VertexBuffer, *VSConstantBuffer;
    UINT IndexBufferOffset, VertexBufferStride, VertexBufferOffset;
    DXGI_FORMAT IndexBufferFormat;
    ID3D11InputLayout* InputLayout;
    ID3D11RenderTargetView* RenderTargets[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];
    ID3D11DepthStencilView* DepthStencil;
};

void BackupD3D11State(ID3D11DeviceContext* ctx, StateBackupD3D11& old);
void RestoreD3D11State(ID3D11DeviceContext* ctx, const StateBackupD3D11& old);

struct RenderDocCapture
{
    static void Init(ComPtr<ID3D11Device>& dev);
    RenderDocCapture();
    ~RenderDocCapture();

private:
    static RENDERDOC_API_1_5_0* rdoc_;
    static ComPtr<ID3D11Device> dev_;
};

#if _DEBUG
#define RDOC_CAPTURE() RenderDocCapture capture##__COUNTER__
#else
#define RDOC_CAPTURE()
#endif
