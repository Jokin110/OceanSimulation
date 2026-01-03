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

class OceanSurface : public Object<VertexData, PerObjectConstantBufferData>
{
public:
	OceanSurface() : Object() {}
	OceanSurface(string name) : Object(name) {}
	OceanSurface(string name, wstring vertexShaderFile, wstring pixelShaderFile) : Object(name, vertexShaderFile, pixelShaderFile) {}

	bool Initialize() override;
	void Start() override;
	void Update() override;

protected:
	UINT GetVertexInputLayout(D3D11_INPUT_ELEMENT_DESC*& inputLayout) override;

	void GenerateMesh() override;

};

