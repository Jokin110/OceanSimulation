#include "FogPostprocessEffect.h"
#include "D3D11Application.h"
#include "SceneManager.h"
#include "CameraManager.h"
#include "imgui.h"
#include <fstream>

FogPostprocessEffect::FogPostprocessEffect(string name, wstring pixelShaderFilePath) : PostprocessEffect(name, pixelShaderFilePath)
{
    m_PixelShaderSRVCount = 1;

    m_PixelShaderSRV = new ID3D11ShaderResourceView * [m_PixelShaderSRVCount];
}

FogPostprocessEffect::~FogPostprocessEffect()
{
    ReleaseResources();
}

bool FogPostprocessEffect::Initialize()
{
    if (!PostprocessEffect::Initialize())
		return false;

	if (m_d3dSamplerState)
	{
		m_d3dSamplerState->Release();
		m_d3dSamplerState = nullptr;
	}

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
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

	m_PixelShaderConstantBufferData.m_FogColor = XMFLOAT3(1.0f, 1.0f, 1.0f); 
	m_PixelShaderConstantBufferData.m_FogDensity = 0.08f;
	m_PixelShaderConstantBufferData.m_HeightFalloff = 0.02f;
	m_PixelShaderConstantBufferData.m_LightScatteringIntensity = 1.0f;
	m_PixelShaderConstantBufferData.m_FogFactorExponent = 1.0f;

	std::ifstream inFile("FogSettings.bin", std::ios::binary);
	if (inFile.is_open())
	{
		inFile.read(reinterpret_cast<char*>(&m_PixelShaderConstantBufferData), sizeof(m_PixelShaderConstantBufferData));
		inFile.close();
	}

	return true;
}

void FogPostprocessEffect::ReleaseResources()
{
    PostprocessEffect::ReleaseResources();

    if (m_PixelShaderSRV) { delete[] m_PixelShaderSRV; m_PixelShaderSRV = nullptr; };
}

void FogPostprocessEffect::Start()
{
	
}

void FogPostprocessEffect::Update()
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_AlwaysAutoResize;
	ImGui::Begin("Fog Settings", nullptr, windowFlags);

	ImGui::ColorEdit3("Fog Color", (float*)&m_PixelShaderConstantBufferData.m_FogColor);
	ImGui::SliderFloat("Fog Density", &m_PixelShaderConstantBufferData.m_FogDensity, 0.0f, 0.1f, "%.4f");
	ImGui::SliderFloat("Height Falloff", &m_PixelShaderConstantBufferData.m_HeightFalloff, 0.0f, 0.1f, "%.4f");
	ImGui::SliderFloat("Light Scattering Intensity", &m_PixelShaderConstantBufferData.m_LightScatteringIntensity, 0.0f, 50.0f, "%.4f");
	ImGui::SliderFloat("Fog Factor Exponent", &m_PixelShaderConstantBufferData.m_FogFactorExponent, 0.1f, 5.0f, "%.4f");

	if (ImGui::Button("Save Settings"))
	{
		std::ofstream outFile("FogSettings.bin", std::ios::binary);
		if (outFile.is_open())
		{
			outFile.write(reinterpret_cast<const char*>(&m_PixelShaderConstantBufferData), sizeof(m_PixelShaderConstantBufferData));
			outFile.close();
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Load Settings"))
	{
		std::ifstream inFile("FogSettings.bin", std::ios::binary);
		if (inFile.is_open())
		{
			inFile.read(reinterpret_cast<char*>(&m_PixelShaderConstantBufferData), sizeof(m_PixelShaderConstantBufferData));
			inFile.close();
		}
	}

	ImGui::End();
}

void FogPostprocessEffect::Render()
{
	XMMATRIX viewProjectionMatrix = CameraManager::GetInstance().GetViewMatrix() * CameraManager::GetInstance().GetProjectionMatrix();
	m_PixelShaderConstantBufferData.m_InverseViewProjectionMatrix = XMMatrixInverse(nullptr, viewProjectionMatrix);
	m_PixelShaderConstantBufferData.m_CameraPosition = CameraManager::GetInstance().GetCameraPosition();
	m_PixelShaderConstantBufferData.m_LightDirection = SceneManager::GetInstance().GetLightDirection();
	m_PixelShaderConstantBufferData.m_LightColor = SceneManager::GetInstance().GetLightColor();

    PostprocessEffect::Render();
}

ID3D11ShaderResourceView* const* FogPostprocessEffect::GetPixelShaderSRVs()
{
    m_PixelShaderSRV[0] = D3D11Application::GetInstance().GetDepthStencilSRV();

    return m_PixelShaderSRV;
}