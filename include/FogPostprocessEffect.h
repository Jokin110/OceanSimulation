#pragma once

#include "PostprocessEffect.h"

#include "DirectXMath.h"

using namespace DirectX;

// Define the structure of the constant buffers for the pixel shader
struct PixelShaderConstantBufferDataFogPostprocessEffect
{
	XMMATRIX m_InverseViewProjectionMatrix;
	XMFLOAT3 m_CameraPosition; 
	float m_FogDensity;
	XMFLOAT3 m_FogColor;
	float m_HeightFalloff;
	XMFLOAT3 m_LightDirection;
	float m_FogFactorExponent;
	XMFLOAT3 m_LightColor;
	float m_LightScatteringIntensity;
};

class FogPostprocessEffect : public PostprocessEffect<PixelShaderConstantBufferDataFogPostprocessEffect>
{
public:
	FogPostprocessEffect(string name, wstring pixelShaderFilePath);
	~FogPostprocessEffect();

	bool Initialize() override;
	void Start() override;
	void Update() override;
	void Render() override;
	ID3D11ShaderResourceView* const* GetPixelShaderSRVs() override;

protected:
	void ReleaseResources() override;

private:
	ID3D11ShaderResourceView** m_PixelShaderSRV = { nullptr };
};

