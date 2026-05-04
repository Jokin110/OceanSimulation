#pragma once

#include "Object.h"

#include <DirectXMath.h>

using namespace DirectX;

// Define the vertex data structure
struct VertexData
{
	XMFLOAT3 m_Position;
	XMFLOAT3 m_Color;
};

// Define the structure of the constant buffers for the vertex shader
struct VertexShaderConstantBufferData
{
	
};

// Define the structure of the constant buffers for the hull shader
struct HullShaderConstantBufferData
{
	XMMATRIX m_WorldMatrix;

	XMFLOAT3 m_CameraPosition;

	float m_MinDistance;
	float m_MaxDistance;
	int m_TessFactorExponent;

	XMFLOAT2 m_Padding;
};

// Define the structure of the constant buffers for the domain shader
struct DomainShaderConstantBufferData
{
	XMMATRIX m_WorldMatrix;
	XMMATRIX m_ViewProjectionMatrix;

	XMFLOAT4 m_PatchSizes; // Size of the ocean patch in world units

	XMFLOAT3 m_CameraPosition;

	float m_Padding;
};

// Define the structure of the constant buffers for the pixel shader
struct PixelShaderConstantBufferData
{
	XMFLOAT3 m_FoamColor;
	float m_FoamBias;
	XMFLOAT3 m_LightColor;
	float m_AmbientLightIntensity;
	XMFLOAT3 m_LightDirection;
	float m_DecayFactor;
	XMFLOAT3 m_SpecularColor;
	float m_FoamAddition;
	XMFLOAT3 m_FogColor;
	float m_FogDistance;

	XMFLOAT3 m_UpwellingColor;
	float m_Snell;
	XMFLOAT3 m_AirColor;
	float m_kDiffuse;
};

class OceanSurface : public Object<VertexData, VertexShaderConstantBufferData, PixelShaderConstantBufferData, HullShaderConstantBufferData, DomainShaderConstantBufferData>
{
public:
	OceanSurface(string name, wstring vertexShaderFile, wstring pixelShaderFile, wstring hullShaderFile, wstring domainShaderFile, D3D11_PRIMITIVE_TOPOLOGY topology) : Object(name, vertexShaderFile, pixelShaderFile, hullShaderFile, domainShaderFile, topology) {}
	~OceanSurface();

	bool Initialize() override;
	void Start() override;
	void Update() override;

	ID3D11ShaderResourceView* const* GetDomainShaderSRVs() override;
	ID3D11ShaderResourceView* const* GetPixelShaderSRVs() override;

	bool RegenerateMeshAndPos(Vector3 position);

	void UpdatePixelShaderBuffer(const PixelShaderConstantBufferData& pixelShaderBufferData);

protected:
	UINT GetVertexInputLayout(D3D11_INPUT_ELEMENT_DESC*& inputLayout) override;

	void GenerateMesh() override;
};

