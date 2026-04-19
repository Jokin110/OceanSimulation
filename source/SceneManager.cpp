#include "SceneManager.h"
#include "OceanComputeManager.h"
#include <fstream>

SceneManager* SceneManager::m_Instance = nullptr;

SceneManager::SceneManager()
{
	for (int i = 0; i < m_OceanSurfaceSideCount * m_OceanSurfaceSideCount; i++)
	{
		m_Ocean[i] = new OceanSurface("Ocean Surface", L"assets/shaders/OceanSurfaceVS.hlsl", L"assets/shaders/PixelShader.hlsl", L"assets/shaders/OceanSurfaceHS.hlsl", L"assets/shaders/OceanSurfaceDS.hlsl", D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

		m_Ocean[i]->SetPosition(Vector3((i % m_OceanSurfaceSideCount - (m_OceanSurfaceSideCount - 1) / 2) * (OceanComputeManager::GetInstance().GetOceanPatchSize()[0] - 0.0f), 0.0f, (i / m_OceanSurfaceSideCount - (m_OceanSurfaceSideCount - 1) / 2) * (OceanComputeManager::GetInstance().GetOceanPatchSize()[0]	 - 0.0f)));
	}
}

SceneManager::~SceneManager()
{
	if (m_Instance)
	{
		delete m_Instance;
		m_Instance = nullptr;
	}
}

bool SceneManager::Initialize()
{
	if (!m_Instance)
	{
		m_Instance = new SceneManager;

		m_Instance->m_PixelShaderSettings = {};
		m_Instance->m_PixelShaderSettings.m_FoamColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
		m_Instance->m_PixelShaderSettings.m_FoamBias = 0.3f;
		m_Instance->m_PixelShaderSettings.m_DecayFactor = 0.98f;
		m_Instance->m_PixelShaderSettings.m_FoamAddition = 1.0f;
		m_Instance->m_PixelShaderSettings.m_LightColor = XMFLOAT3(0.53f, 0.81f, 0.92f);
		m_Instance->m_PixelShaderSettings.m_AmbientLightIntensity = 0.25f;
		m_Instance->m_PixelShaderSettings.m_LightDirection = XMFLOAT3(0.0f, -0.5f, -1.0f);
		m_Instance->m_PixelShaderSettings.m_SpecularColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
		m_Instance->m_PixelShaderSettings.m_FogColor = XMFLOAT3(1.0f, 1.0f, 1.0f);
		m_Instance->m_PixelShaderSettings.m_FogDistance = 800.0f;

		m_Instance->m_PixelShaderSettings.m_UpwellingColor = XMFLOAT3(0.1f, 0.3f, 0.4f);
		m_Instance->m_PixelShaderSettings.m_Snell = 1.33f;
		m_Instance->m_PixelShaderSettings.m_AirColor = XMFLOAT3(0.1f, 0.1f, 0.1f);
		m_Instance->m_PixelShaderSettings.m_kDiffuse = 0.01f;

		std::ifstream inFile("OceanSettings.bin", std::ios::binary);
		if (inFile.is_open())
		{
			inFile.read(reinterpret_cast<char*>(&m_Instance->m_PixelShaderSettings), sizeof(m_Instance->m_PixelShaderSettings));
			inFile.close();
		}

		return true;
	}

	return false;
}

void SceneManager::Start()
{
	for (int i = 0; i < 25; i++)
	{
		m_Ocean[i]->UpdatePixelShaderBuffer(m_PixelShaderSettings);
	}
}

void SceneManager::Update()
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_AlwaysAutoResize;
	ImGui::Begin("Ocean Surface Rendering Settings", nullptr, windowFlags);

	ImGui::ColorEdit3("Foam Color", (float*)&m_PixelShaderSettings.m_FoamColor);
	ImGui::SliderFloat("Foam Bias", &m_PixelShaderSettings.m_FoamBias, 0.0f, 1.0f);
	ImGui::SliderFloat("Decay Factor", &m_PixelShaderSettings.m_DecayFactor, 0.0f, 1.0f);
	ImGui::SliderFloat("Foam Addition", &m_PixelShaderSettings.m_FoamAddition, 0.0f, 5.0f);
	ImGui::ColorEdit3("Light Color", (float*)&m_PixelShaderSettings.m_LightColor);
	ImGui::SliderFloat("Ambient Light Intensity", &m_PixelShaderSettings.m_AmbientLightIntensity, 0.0f, 1.0f);
	ImGui::SliderFloat3("Light Direction", (float*)&m_PixelShaderSettings.m_LightDirection, -1.0f, 1.0f);
	ImGui::ColorEdit3("Specular Color", (float*)&m_PixelShaderSettings.m_SpecularColor);
	ImGui::ColorEdit3("Fog Color", (float*)&m_PixelShaderSettings.m_FogColor);
	ImGui::SliderFloat("Fog Distance", &m_PixelShaderSettings.m_FogDistance, 0.0f, 2000.0f);
	ImGui::ColorEdit3("Upwelling Color", (float*)&m_PixelShaderSettings.m_UpwellingColor);
	ImGui::SliderFloat("Snell's Index", &m_PixelShaderSettings.m_Snell, 1.0f, 2.0f);
	ImGui::ColorEdit3("Air Color", (float*)&m_PixelShaderSettings.m_AirColor);
	ImGui::SliderFloat("Diffuse Coefficient", &m_PixelShaderSettings.m_kDiffuse, 0.0f, 1.0f);

	if (ImGui::Button("Apply Changes"))
	{
		for (int i = 0; i < 25; i++)
		{
			m_Ocean[i]->UpdatePixelShaderBuffer(m_PixelShaderSettings);
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Save Settings"))
	{
		std::ofstream outFile("OceanSettings.bin", std::ios::binary);
		if (outFile.is_open())
		{
			outFile.write(reinterpret_cast<const char*>(&m_PixelShaderSettings), sizeof(m_PixelShaderSettings));
			outFile.close();
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Load Settings"))
	{
		std::ifstream inFile("OceanSettings.bin", std::ios::binary);
		if (inFile.is_open())
		{
			inFile.read(reinterpret_cast<char*>(&m_PixelShaderSettings), sizeof(m_PixelShaderSettings));
			inFile.close();

			for (int i = 0; i < 25; i++)
			{
				m_Ocean[i]->UpdatePixelShaderBuffer(m_PixelShaderSettings);
			}
		}
	}

	ImGui::End();
}

void SceneManager::RegenerateMeshes()
{
	for (int i = 0; i < m_OceanSurfaceSideCount * m_OceanSurfaceSideCount; i++)
	{
		m_Ocean[i]->RegenerateMeshAndPos(Vector3((i % m_OceanSurfaceSideCount - (m_OceanSurfaceSideCount - 1) / 2) * (OceanComputeManager::GetInstance().GetOceanPatchSize()[0] - 0.0f), 0.0f, (i / m_OceanSurfaceSideCount - (m_OceanSurfaceSideCount - 1) / 2) * (OceanComputeManager::GetInstance().GetOceanPatchSize()[0] - 0.0f)));
	}
}