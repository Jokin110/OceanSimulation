#pragma once
#include "Object.h"
#include "Texture2D.h"

#include <DirectXMath.h>

using namespace DirectX;

const int SKYBOX_TEXTURE_COUNT = 6;

// Define the vertex data structure
struct VertexDataSkyBox
{
	XMFLOAT3 m_Position;
};

// Define the structure of the constant buffers for the vertex shader
struct PerObjectVertexShaderConstantBufferDataSkyBox
{
	XMMATRIX m_WorldViewProjectionMatrix;
};

// Define the structure of the constant buffers for the hull shader
struct PerObjectHullShaderConstantBufferDataSkyBox
{

};

// Define the structure of the constant buffers for the domain shader
struct PerObjectDomainShaderConstantBufferDataSkyBox
{

};

// Define the structure of the constant buffers for the pixel shader
struct PerObjectPixelShaderConstantBufferDataSkyBox
{

};

class SkyBox : public Object<VertexDataSkyBox, PerObjectVertexShaderConstantBufferDataSkyBox, PerObjectPixelShaderConstantBufferDataSkyBox, PerObjectHullShaderConstantBufferDataSkyBox, PerObjectDomainShaderConstantBufferDataSkyBox>
{
public:
	SkyBox(string name);
	~SkyBox();

	bool Initialize() override;
	void Start() override;
	void Update() override;

	ID3D11ShaderResourceView* const* GetPixelShaderSRVs() override;

protected:
	UINT GetVertexInputLayout(D3D11_INPUT_ELEMENT_DESC*& inputLayout) override;

	void GenerateMesh() override;

private:
	wstring m_VertexShaderFilePath = L"assets/shaders/renderPipeline/SkyBoxVS.hlsl";
	wstring m_PixelShaderFilePath = L"assets/shaders/renderPipeline/SkyBoxPS.hlsl";

	string m_SkyBoxTextureFilePath[SKYBOX_TEXTURE_COUNT] = {"images/Daylight Box_Back.bmp", "images/Daylight Box_Front.bmp", "images/Daylight Box_Left.bmp", "images/Daylight Box_Right.bmp", "images/Daylight Box_Top.bmp", "images/Daylight Box_Bottom.bmp"};

	Texture2D* m_SkyBoxTexture = nullptr;
};

