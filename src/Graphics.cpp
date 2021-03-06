#include <Graphics.h>
#include <DirectXTK/DDSTextureLoader.h>
#include <renderdoc_app.h>
#include <Utility.h>

std::pair<ComPtr<ID3D11Resource>, ComPtr<ID3D11ShaderResourceView>> CreateResourceFromResource(ID3D11Device* pDev, HMODULE hModule, unsigned uResource)
{
	const auto resourceSpan = LoadResource(hModule, uResource);
	if (resourceSpan.data() == nullptr)
		return { nullptr, nullptr };

	ComPtr<ID3D11Resource> res;
	ComPtr<ID3D11ShaderResourceView> srv;

	auto hr = DirectX::CreateDDSTextureFromMemory(pDev, resourceSpan.data(), resourceSpan.size_bytes(), &res, &srv);
	GW2_ASSERT(SUCCEEDED(hr));

	return { res, srv };
}

void DrawScreenQuad(ID3D11DeviceContext* ctx)
{
    ctx->IASetVertexBuffers(0, 0, NULL, NULL, NULL);
    ctx->IASetInputLayout(NULL);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    ctx->Draw(4, 0);
}

void BackupD3D11State(ID3D11DeviceContext* ctx, StateBackupD3D11& old)
{
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

void RestoreD3D11State(ID3D11DeviceContext* ctx, const StateBackupD3D11& old)
{
    ctx->RSSetScissorRects(old.ScissorRectsCount, old.ScissorRects);
    ctx->RSSetViewports(old.ViewportsCount, old.Viewports);
    ctx->RSSetState(old.RS); if (old.RS) old.RS->Release();
    ctx->OMSetBlendState(old.BlendState, old.BlendFactor, old.SampleMask); if (old.BlendState) old.BlendState->Release();
    ctx->OMSetDepthStencilState(old.DepthStencilState, old.StencilRef); if (old.DepthStencilState) old.DepthStencilState->Release();
    ctx->PSSetShaderResources(0, 1, &old.PSShaderResource); if (old.PSShaderResource) old.PSShaderResource->Release();
    ctx->PSSetSamplers(0, 1, &old.PSSampler); if (old.PSSampler) old.PSSampler->Release();
    ctx->PSSetShader(old.PS, old.PSInstances, old.PSInstancesCount); if (old.PS) old.PS->Release();
    for (UINT i = 0; i < old.PSInstancesCount; i++) if (old.PSInstances[i]) old.PSInstances[i]->Release();
    ctx->VSSetShader(old.VS, old.VSInstances, old.VSInstancesCount); if (old.VS) old.VS->Release();
    ctx->VSSetConstantBuffers(0, 1, &old.VSConstantBuffer); if (old.VSConstantBuffer) old.VSConstantBuffer->Release();
    ctx->GSSetShader(old.GS, old.GSInstances, old.GSInstancesCount); if (old.GS) old.GS->Release();
    for (UINT i = 0; i < old.VSInstancesCount; i++) if (old.VSInstances[i]) old.VSInstances[i]->Release();
    ctx->IASetPrimitiveTopology(old.PrimitiveTopology);
    ctx->IASetIndexBuffer(old.IndexBuffer, old.IndexBufferFormat, old.IndexBufferOffset); if (old.IndexBuffer) old.IndexBuffer->Release();
    ctx->IASetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride, &old.VertexBufferOffset); if (old.VertexBuffer) old.VertexBuffer->Release();
    ctx->IASetInputLayout(old.InputLayout); if (old.InputLayout) old.InputLayout->Release();

    ctx->OMSetRenderTargets(D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT, old.RenderTargets, old.DepthStencil); if (old.DepthStencil) old.DepthStencil->Release();
    for (int i = 0; i < D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
        if (old.RenderTargets[i])
            old.RenderTargets[i]->Release();
}

RENDERDOC_API_1_5_0* RenderDocCapture::rdoc_ = nullptr;
ComPtr<ID3D11Device> RenderDocCapture::dev_;

void RenderDocCapture::Init(ComPtr<ID3D11Device>& dev)
{
#if _DEBUG
    if (HMODULE mod = GetModuleHandleA("renderdoc.dll"))
    {
        pRENDERDOC_GetAPI RENDERDOC_GetAPI =
            (pRENDERDOC_GetAPI)GetProcAddress(mod, "RENDERDOC_GetAPI");
        int ret = RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_5_0, (void**)&rdoc_);
        if (ret != 1)
            rdoc_ = nullptr;

        if (rdoc_)
            dev_ = dev;
    }
#endif
}

RenderDocCapture::RenderDocCapture()
{
    if (!rdoc_)
        return;
    LogDebug("Beginning RenderDoc frame capture...");

    rdoc_->StartFrameCapture(dev_.Get(), nullptr);
}

RenderDocCapture::~RenderDocCapture()
{
    if (!rdoc_)
        return;
    LogDebug("Ending RenderDoc frame capture...");

    rdoc_->EndFrameCapture(dev_.Get(), nullptr);
}