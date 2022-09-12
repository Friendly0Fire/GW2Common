#pragma once

#include <Common.h>
#include <d3dcompiler.h>
#include <Utility.h>
#include <span>
#include <map>
#include <variant>
#include <ZipArchive.h>
#include <d3d11.h>

class ShaderId
{
public:
	ShaderId() : id(std::numeric_limits<uint>::max()) {}

private:
	ShaderId(uint i) : id(i) {}
	uint id;

	friend class ShaderManager;
};

class ConstantBufferBase
{
protected:
	ConstantBufferBase(ComPtr<ID3D11Buffer> buf) : buf(buf) {}
	ComPtr<ID3D11Buffer> buf;

	friend class ShaderManager;

	void Upload(ID3D11DeviceContext* ctx, void* data, size_t size);

public:
	bool IsValid() const { return buf != nullptr; }
	const ComPtr<ID3D11Buffer>& buffer() const { return buf; }
};

template<typename T>
class ConstantBuffer : public ConstantBufferBase
{
protected:
    ConstantBuffer(ComPtr<ID3D11Buffer> buf) : ConstantBufferBase(buf) {}
	ConstantBuffer(ComPtr<ID3D11Buffer> buf, T&& data) : ConstantBufferBase(buf), data(data) {}

	friend class ShaderManager;

	T data;

public:
    ~ConstantBuffer()
	{
	    LogDebug("Constant buffer of type {} destroyed", typeid(T).name());
	}

	ConstantBuffer() = default;

	ConstantBuffer(const ConstantBuffer&) = delete;
	ConstantBuffer(ConstantBuffer&&) = default;
	ConstantBuffer& operator=(const ConstantBuffer&) = delete;
	ConstantBuffer& operator=(ConstantBuffer&&) = default;

	T* operator->() { return &data; }
	void Update(ID3D11DeviceContext* ctx)
	{
		Upload(ctx, &data, sizeof(T));
	}
};
template<typename T>
using ConstantBufferSPtr = std::shared_ptr<ConstantBuffer<T>>;
template<typename T>
using ConstantBufferWPtr = std::weak_ptr<ConstantBuffer<T>>;

class ShaderManager : public Singleton<ShaderManager>
{
    template<typename T>
    class ConstantBufferFwd : public ConstantBuffer<T>
    {
    public:
        template<typename ...Args>
        ConstantBufferFwd(Args&& ...args) : ConstantBuffer<T>(std::forward<Args>(args)...) {}
    };

public:
	using AnyShaderComPtr = std::variant<ComPtr<ID3D11VertexShader>, ComPtr<ID3D11PixelShader>>;

    ShaderManager(ComPtr<ID3D11Device>& device, uint shaderResourceID, HMODULE shaderResourceModule, const std::filesystem::path& shadersPath);

	void SetShaders(ID3D11DeviceContext* ctx, ShaderId vs, ShaderId ps);
	ShaderId GetShader(const std::wstring& filename, D3D11_SHADER_VERSION_TYPE st, const std::string& entrypoint);

	template<typename T>
	ConstantBufferSPtr<T> MakeConstantBuffer(std::optional<T> data = std::nullopt)
	{
		auto buf = MakeConstantBuffer(sizeof(T), data.has_value() ? &data.value() : nullptr);
        if(data.has_value())
		    return std::make_shared<ConstantBufferFwd<T>>(buf, std::move(data.value()));
        else
		    return std::make_shared<ConstantBufferFwd<T>>(buf);
	}

	template<typename... Args>
	void SetConstantBuffers(ID3D11DeviceContext* ctx, Args& ...cbs)
	{
		auto getbuf = [](auto& cb) { return cb.buf.Get(); };
		ID3D11Buffer* cbPtrs[] = { getbuf(cbs)... };
		ctx->VSSetConstantBuffers(0, sizeof...(cbs), cbPtrs);
		ctx->PSSetConstantBuffers(0, sizeof...(cbs), cbPtrs);
	}

	void ReloadAll();

protected:
	[[nodiscard]] std::string LoadShaderFile(const std::wstring& filename);

	ComPtr<ID3D11Buffer> MakeConstantBuffer(size_t dataSize, const void* data);

	void LoadShadersArchive();

	[[nodiscard]] AnyShaderComPtr CompileShader(const std::wstring& filename, D3D11_SHADER_VERSION_TYPE st, const std::string& entrypoint);

	[[nodiscard]] std::wstring GetShaderFilename(const std::wstring& filename) const;
    [[nodiscard]] std::string EncodeShaderFilename(const std::wstring& filename) const {
        return utf8_encode(GetShaderFilename(filename));
    }
    [[nodiscard]] ID3DInclude* GetIncludeManager() const;
	void CheckHotReload();

	bool hotReloadFolderExists_ = false;

	struct ShaderData
	{
		AnyShaderComPtr shader;

		std::wstring filename;
		D3D11_SHADER_VERSION_TYPE st;
		std::string entrypoint;
	};
	std::vector<ShaderData> shaders_;

	ZipArchive::Ptr shadersZip_;
	std::unique_ptr<ID3DInclude> shaderIncludeManager_;

	ComPtr<ID3D11Device> device_;

	HMODULE shaderResourceModule_ = 0;
	uint shaderResourceID_ = 0;
	std::filesystem::path shadersPath_;

	friend class ShaderInclude;
};