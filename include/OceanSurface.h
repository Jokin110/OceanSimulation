#pragma once

#include "Object.h"

#include <DirectXMath.h>

using namespace DirectX;

// Define the vertex data structure
struct VertexData
{
	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT3 color;
};

// Define the structure of the constant buffers for the vertex shader
struct PerObjectConstantBufferData
{
	XMMATRIX m_WorldMatrix;
	XMMATRIX m_InverseTransposeWorldMatrix;
	XMMATRIX m_ViewProjectionMatrix;
	float m_Time;
	XMFLOAT3 m_CameraPosition;
};

// Define the structure of the constant buffers for the pixel shader
struct PerObjectPixelShaderBufferData
{
	XMFLOAT3 m_FoamColor;
	float m_FoamBias;
	XMFLOAT3 m_LightColor;
	float m_AmbientLightIntensity;
	XMFLOAT3 m_LightDirection;
	float m_Padding0; // Padding to ensure 16-byte alignment
	XMFLOAT3 m_SpecularColor;
	float m_Padding1; // Padding to ensure 16-byte alignment
	XMFLOAT3 m_FogColor;
	float m_FogDistance;

	XMFLOAT3 m_UpwellingColor;
	float m_Snell;
	XMFLOAT3 m_AirColor;
	float m_kDiffuse;
};

class OceanSurface : public Object<VertexData, PerObjectConstantBufferData, PerObjectPixelShaderBufferData>
{
public:
	OceanSurface() : Object() {}
	OceanSurface(string name) : Object(name) {}
	OceanSurface(string name, wstring vertexShaderFile, wstring pixelShaderFile) : Object(name, vertexShaderFile, pixelShaderFile) {}
	OceanSurface(string name, wstring hullShaderFile, wstring domainShaderFile, D3D11_PRIMITIVE_TOPOLOGY topology) : Object(name, hullShaderFile, domainShaderFile, topology) {}
	OceanSurface(string name, wstring vertexShaderFile, wstring pixelShaderFile, wstring hullShaderFile, wstring domainShaderFile, D3D11_PRIMITIVE_TOPOLOGY topology) : Object(name, vertexShaderFile, pixelShaderFile, hullShaderFile, domainShaderFile, topology) {}

	bool Initialize() override;
	void Start() override;
	void Update() override;

protected:
	UINT GetVertexInputLayout(D3D11_INPUT_ELEMENT_DESC*& inputLayout) override;

	void GenerateMesh() override;

};

