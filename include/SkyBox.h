#pragma once
#include "Object.h"

#include <DirectXMath.h>

using namespace DirectX;

// Define the vertex data structure
struct VertexDataSkyBox
{
	XMFLOAT3 position;
	XMFLOAT2 texcoord;
};

// Define the structure of the constant buffers for the vertex shader
struct PerObjectConstantBufferDataSkyBox
{
	XMMATRIX m_WorldViewProjectioMatrix;
};

// Define the structure of the constant buffers for the pixel shader
struct PerObjectPixelShaderBufferDataSkyBox
{
	
};

class SkyBox : public Object<VertexDataSkyBox, PerObjectConstantBufferDataSkyBox, PerObjectPixelShaderBufferDataSkyBox>
{
public:
	SkyBox() : Object() {}
	SkyBox(string name) : Object(name) {}
	SkyBox(string name, wstring vertexShaderFile, wstring pixelShaderFile) : Object(name, vertexShaderFile, pixelShaderFile) {}

	bool Initialize() override;
	void Start() override;
	void Update() override;

protected:
	UINT GetVertexInputLayout(D3D11_INPUT_ELEMENT_DESC*& inputLayout) override;

	void GenerateMesh() override;
};

