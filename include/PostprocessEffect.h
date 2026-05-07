#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <string>

#include "PostprocessEffectManager.h"
#include "D3D11Application.h"

using namespace DirectX;
using namespace std;

class IPostprocessEffect
{
public:
	virtual ~IPostprocessEffect() = default;

	virtual bool Initialize() = 0;
	virtual void Start() = 0;
	virtual void Update() = 0;
	virtual void Render() = 0;

	virtual void ReleaseResources() = 0;
	
	virtual ID3D11PixelShader*& GetPixelShader() = 0;

	virtual ID3D11SamplerState*& GetSamplerState() = 0;

	virtual ID3D11Buffer*& GetPixelShaderConstantBuffers() = 0;

	virtual ID3D11ShaderResourceView* const* GetPixelShaderSRVs() = 0;

	virtual UINT GetPixelShaderSRVCount() = 0;
};

template<typename PixelShaderConstantBufferData>
class PostprocessEffect : IPostprocessEffect
{
public:
	PostprocessEffect();
	PostprocessEffect(string name, wstring pixelShaderFilePath);
	~PostprocessEffect();

	virtual bool Initialize();
	virtual void Start();
	virtual void Update();
	virtual void Render();

	ID3D11PixelShader*& GetPixelShader() { return m_d3dPixelShader; }

	ID3D11SamplerState*& GetSamplerState() { return m_d3dSamplerState; }

	ID3D11Buffer*& GetPixelShaderConstantBuffers() { return m_d3dPixelShaderConstantBuffers; }

	virtual ID3D11ShaderResourceView* const* GetPixelShaderSRVs();

	UINT GetPixelShaderSRVCount() { return m_PixelShaderSRVCount; }

protected:
	string m_Name;

	wstring m_PixelShaderFile;

	void ReleaseResources();

	void GetPixelShaderConstantBufferDesc(D3D11_BUFFER_DESC& pixelShaderBufferDesc);

	ID3D11SamplerState* m_d3dSamplerState = nullptr;

	PixelShaderConstantBufferData m_PixelShaderConstantBufferData;

	UINT m_PixelShaderSRVCount = 0;

private:
	ID3D11PixelShader* m_d3dPixelShader = nullptr;

	ID3D11Buffer* m_d3dPixelShaderConstantBuffers = nullptr;

	template<typename T>
	inline void SafeRelease(T& ptr)
	{
		if (ptr != NULL)
		{
			ptr->Release();
			ptr = NULL;
		}
	}
};

template<typename PixelShaderConstantBufferData> PostprocessEffect<PixelShaderConstantBufferData>::PostprocessEffect()
{
	m_Name = "New Postprocess Effect";

	m_PixelShaderFile = L"assets/shaders/postprocessEffects/DefaultPostprocessEffect.hlsl";

	m_d3dPixelShader = nullptr;
	m_d3dPixelShaderConstantBuffers = nullptr;

	m_d3dSamplerState = nullptr;

	m_PixelShaderConstantBufferData = {};

	m_PixelShaderSRVCount = 0;

	PostprocessEffectManager::GetInstance().AddEffectToList(this);
}

template<typename PixelShaderConstantBufferData> 
PostprocessEffect<PixelShaderConstantBufferData>::PostprocessEffect(string name, wstring pixelShaderFilePath) : PostprocessEffect()
{
	m_Name = name;
	m_PixelShaderFile = pixelShaderFilePath;
}

template<typename PixelShaderConstantBufferData> 
PostprocessEffect<PixelShaderConstantBufferData>::~PostprocessEffect()
{
	ReleaseResources();
}

template<typename PixelShaderConstantBufferData> 
void PostprocessEffect<PixelShaderConstantBufferData>::ReleaseResources()
{
	SafeRelease(m_d3dPixelShader);
	SafeRelease(m_d3dPixelShaderConstantBuffers);

	SafeRelease(m_d3dSamplerState);
}

template<typename PixelShaderConstantBufferData>
bool PostprocessEffect<PixelShaderConstantBufferData>::Initialize()
{
#if _DEBUG
	wstring pathPrefix = L"";
#else
	wstring pathPrefix = L"../../";
#endif

	m_d3dPixelShader = D3D11Application::GetInstance().CreatePixelShader(pathPrefix + m_PixelShaderFile);

	if (m_d3dPixelShader == nullptr)
	{
		return false;
	}

	D3D11_BUFFER_DESC pixelShaderBufferDesc = {};
	GetPixelShaderConstantBufferDesc(pixelShaderBufferDesc);

	if (pixelShaderBufferDesc.ByteWidth > 1)
	{
		if (FAILED(D3D11Application::GetInstance().GetDevice()->CreateBuffer(
			&pixelShaderBufferDesc,
			nullptr,
			&m_d3dPixelShaderConstantBuffers)))
		{
			cout << "D3D11: Failed to create per-object pixel shader buffer\n";
			return false;
		}
	}

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0.0f;
	samplerDesc.BorderColor[1] = 0.0f;
	samplerDesc.BorderColor[2] = 0.0f;
	samplerDesc.BorderColor[3] = 0.0f;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	if (FAILED(D3D11Application::GetInstance().GetDevice()->CreateSamplerState(
		&samplerDesc,
		&m_d3dSamplerState)))
	{
		cout << "D3D11: Failed to create sampler state\n";
		return false;
	}

	return true;
}

template<typename PixelShaderConstantBufferData>
void PostprocessEffect<PixelShaderConstantBufferData>::Start()
{

}

template<typename PixelShaderConstantBufferData>
void PostprocessEffect<PixelShaderConstantBufferData>::Update()
{

}

template<typename PixelShaderConstantBufferData>
void PostprocessEffect<PixelShaderConstantBufferData>::Render()
{
	if (m_d3dPixelShaderConstantBuffers)
		D3D11Application::GetInstance().GetDeviceContext()->UpdateSubresource(m_d3dPixelShaderConstantBuffers, 0, nullptr, &m_PixelShaderConstantBufferData, 0, 0);
}

template<typename PixelShaderConstantBufferData>
void PostprocessEffect<PixelShaderConstantBufferData>::GetPixelShaderConstantBufferDesc(D3D11_BUFFER_DESC& constantBufferDesc)
{
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	UINT rawSize = sizeof(PixelShaderConstantBufferData);

	if (rawSize <= 1)
	{
		constantBufferDesc.ByteWidth = 0;
		return;
	}

	constantBufferDesc.ByteWidth = (rawSize + 15) & ~15;

	constantBufferDesc.CPUAccessFlags = 0;
	constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;
}

template<typename PixelShaderConstantBufferData>
ID3D11ShaderResourceView* const* PostprocessEffect<PixelShaderConstantBufferData>::GetPixelShaderSRVs()
{
	return nullptr;
}