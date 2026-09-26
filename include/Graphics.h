#pragma once

#include <d3d11.h>
#include <renderdoc_app.h>

#include "Common.h"

template<typename T>
struct Texture
{
    using D3DType = T;
    ComPtr<T> texture;
    ComPtr<ID3D11ShaderResourceView> srv;
    u32 width;
};

struct Texture1D : Texture<ID3D11Texture1D> { };
struct Texture2D : Texture<ID3D11Texture2D>
{
    u32 height;
};
struct Texture3D : Texture<ID3D11Texture3D>
{
    u32 height, depth;
};

struct RenderTarget : public Texture2D
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
T MakeTexture(ComPtr<ID3D11Device>& dev, u32 width, u32 height, u32 depth, DXGI_FORMAT fmt, UINT mips = 1,
                       bool generateMips = false);

struct DepthStencil : public Texture<ID3D11Texture2D>
{
    ComPtr<ID3D11DepthStencilView> rtv;
};

std::pair<ComPtr<ID3D11Resource>, ComPtr<ID3D11ShaderResourceView>> CreateResourceFromResource(ID3D11Device* pDev, HMODULE hModule,
                                                                                               unsigned uResource);
std::pair<ComPtr<ID3D11Resource>, ComPtr<ID3D11ShaderResourceView>> CreateResourceFromFile(ID3D11Device* pDev, ID3D11DeviceContext* pCtx,
                                                                                           const std::filesystem::path& path);

namespace Detail
{
Texture1D SetTextureSize(Texture1D&& tex);
Texture2D SetTextureSize(Texture2D&& tex);
Texture3D SetTextureSize(Texture3D&& tex);
}

template<typename T = Texture2D>
T CreateTextureFromResource(ID3D11Device* pDev, HMODULE hModule, unsigned uResource) {
    auto [res, srv] = CreateResourceFromResource(pDev, hModule, uResource);
    GW2_ASSERT(res != nullptr);

    ComPtr<typename T::D3DType> tex;
    res->QueryInterface(tex.GetAddressOf());
    GW2_ASSERT(tex != nullptr);

    return Detail::SetTextureSize({ tex, srv });
}

template<typename T = Texture2D>
T CreateTextureFromFile(ID3D11Device* pDev, ID3D11DeviceContext* pCtx, const std::filesystem::path& path) {
    auto [res, srv] = CreateResourceFromFile(pDev, pCtx, path);

    ComPtr<typename T::D3DType> tex;
    res->QueryInterface(tex.GetAddressOf());
    GW2_ASSERT(tex != nullptr);

    return Detail::SetTextureSize({ tex, srv });
}

void DrawScreenQuad(ID3D11DeviceContext* ctx);

struct StateBackupD3D11
{
    struct Config {
        struct ShaderStage {
            size_t shaderResourceCount = D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT;
            size_t samplerCount = D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT;
            size_t constantBufferCount = D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT;
        } vs, gs, ps;
        size_t vertexBufferCount = D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;
    };

    StateBackupD3D11(ID3D11DeviceContext* ctx, Config&& cfg);
    ~StateBackupD3D11();
    ComPtr<ID3D11DeviceContext> Context;

    std::vector<D3D11_RECT> ScissorRects;
    std::vector<D3D11_VIEWPORT> Viewports;
    ComPtr<ID3D11RasterizerState> RS;
    ComPtr<ID3D11BlendState> BlendState;
    std::array<FLOAT, 4> BlendFactor;
    UINT SampleMask;
    UINT StencilRef;
    ComPtr<ID3D11DepthStencilState> DepthStencilState;

    template<typename Shader>
    struct ShaderStage {
        ComPtr<Shader> Shader;
        std::vector<ID3D11ShaderResourceView*> ShaderResources;
        std::vector<ID3D11SamplerState*> Samplers;
        std::vector<ID3D11Buffer*> ConstantBuffers;
        std::vector<ID3D11ClassInstance*> Instances;
    };
    ShaderStage<ID3D11VertexShader> VS;
    ShaderStage<ID3D11GeometryShader> GS;
    ShaderStage<ID3D11PixelShader> PS;

    D3D11_PRIMITIVE_TOPOLOGY PrimitiveTopology;
    std::vector<ID3D11Buffer*> VertexBuffers;
    std::vector<UINT> VertexBufferStrides, VertexBufferOffsets;
    UINT IndexBufferOffset;
    ComPtr<ID3D11Buffer> IndexBuffer;
    DXGI_FORMAT IndexBufferFormat;
    ComPtr<ID3D11InputLayout> InputLayout;
    std::array<ID3D11RenderTargetView*, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> RenderTargets;
    ComPtr<ID3D11DepthStencilView> DepthStencil;
};

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
