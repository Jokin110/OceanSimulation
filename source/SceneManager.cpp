#include "SceneManager.h"
#include "OceanComputeManager.h"
#include <fstream>

SceneManager* SceneManager::m_Instance = nullptr;

SceneManager::SceneManager()
{
	m_PixelShaderSettings = {};
	m_SunSettings = {};

	for (int i = 0; i < OCEAN_SURFACE_SIDE_COUNT * OCEAN_SURFACE_SIDE_COUNT; i++)
	{
		m_Ocean[i] = new OceanSurface("Ocean Surface", L"assets/shaders/renderPipeline/OceanSurfaceVS.hlsl", L"assets/shaders/renderPipeline/OceanSurfacePS.hlsl", L"assets/shaders/renderPipeline/OceanSurfaceHS.hlsl", L"assets/shaders/renderPipeline/OceanSurfaceDS.hlsl", D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

		m_Ocean[i]->SetPosition(Vector3((i % OCEAN_SURFACE_SIDE_COUNT - (OCEAN_SURFACE_SIDE_COUNT - 1) / 2) * (OceanComputeManager::GetInstance().GetOceanPatchSize()[0] - 0.0f), 0.0f, (i / OCEAN_SURFACE_SIDE_COUNT - (OCEAN_SURFACE_SIDE_COUNT - 1) / 2) * (OceanComputeManager::GetInstance().GetOceanPatchSize()[0] - 0.0f)));
	}

	m_SkyBox = new SkyBox("SkyBox");

	m_FogPostProcessEffect = new FogPostprocessEffect("Fog Effect", L"assets/shaders/postprocessEffects/Fog.hlsl");
}

SceneManager::~SceneManager()
{
	for (int i = 0; i < OCEAN_SURFACE_SIDE_COUNT * OCEAN_SURFACE_SIDE_COUNT; i++)
	{
		m_Ocean[i] = nullptr;
	}

	m_SkyBox = nullptr;
	m_FogPostProcessEffect = nullptr;

	if (m_Instance)
	{
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

		m_Instance->m_PixelShaderSettings.m_K1 = 1.0f;
		m_Instance->m_PixelShaderSettings.m_K2 = 1.0f;
		m_Instance->m_PixelShaderSettings.m_K3 = 1.0f;
		m_Instance->m_PixelShaderSettings.m_K4 = 1.0f;

		m_Instance->m_PixelShaderSettings.m_WaterScatterColor = XMFLOAT3(0.0f, 0.3f, 0.5f);
		m_Instance->m_PixelShaderSettings.m_AirBubblesColor = XMFLOAT3(0.8f, 0.9f, 1.0f);
		m_Instance->m_PixelShaderSettings.m_DensityOfAirBubblesSpreadInWater = 0.5f;

		m_Instance->m_PixelShaderSettings.m_FoamRoughnessMultiplier = 0.2f;
		m_Instance->m_PixelShaderSettings.m_TextureResolution = OceanComputeManager::GetInstance().GetOceanTextureSize();

		std::ifstream inFile("OceanSettings.bin", std::ios::binary);
		if (inFile.is_open())
		{
			inFile.read(reinterpret_cast<char*>(&m_Instance->m_PixelShaderSettings), sizeof(m_Instance->m_PixelShaderSettings));
			inFile.close();
		}

		m_Instance->m_SunSettings.m_SunColor = XMFLOAT3(1.0f, 0.7f, 0.1f);
		m_Instance->m_SunSettings.m_SunExponent = 2.0f;
		m_Instance->m_SunSettings.m_SunBias = 0.7f;

		std::ifstream sunSettings("SunSettings.bin", std::ios::binary);
		if (sunSettings.is_open())
		{
			sunSettings.read(reinterpret_cast<char*>(&m_Instance->m_SunSettings), sizeof(m_Instance->m_SunSettings));
			sunSettings.close();
		}

		return true;
	}

	return false;
}

void SceneManager::Start()
{
	for (int i = 0; i < OCEAN_SURFACE_SIDE_COUNT * OCEAN_SURFACE_SIDE_COUNT; i++)
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
	ImGui::SliderFloat("Foam Roughness", &m_PixelShaderSettings.m_FoamRoughnessMultiplier, 0.0f, 5.0f);
	ImGui::ColorEdit3("Light Color", (float*)&m_PixelShaderSettings.m_LightColor);
	ImGui::SliderFloat("Ambient Light Intensity", &m_PixelShaderSettings.m_AmbientLightIntensity, 0.0f, 1.0f);
	ImGui::SliderFloat3("Light Direction", (float*)&m_PixelShaderSettings.m_LightDirection, -1.0f, 1.0f);
	ImGui::ColorEdit3("Specular Color", (float*)&m_PixelShaderSettings.m_SpecularColor);
	ImGui::SliderFloat("Snell's Index", &m_PixelShaderSettings.m_Snell, 1.0f, 2.0f);
	ImGui::SliderFloat("K1", &m_PixelShaderSettings.m_K1, 0.0f, 5.0f);
	ImGui::SliderFloat("K2", &m_PixelShaderSettings.m_K2, 0.0f, 5.0f);
	ImGui::SliderFloat("K3", &m_PixelShaderSettings.m_K3, 0.0f, 5.0f);
	ImGui::SliderFloat("K4", &m_PixelShaderSettings.m_K4, 0.0f, 5.0f);
	ImGui::ColorEdit3("Water Scatter Color", (float*)&m_PixelShaderSettings.m_WaterScatterColor);
	ImGui::ColorEdit3("Air Bubbles Color", (float*)&m_PixelShaderSettings.m_AirBubblesColor);
	ImGui::SliderFloat("Density of Air Bubbles Spread in Water", &m_PixelShaderSettings.m_DensityOfAirBubblesSpreadInWater, 0.0f, 1.0f);

	if (ImGui::Button("Apply Changes"))
	{
		for (int i = 0; i < OCEAN_SURFACE_SIDE_COUNT * OCEAN_SURFACE_SIDE_COUNT; i++)
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

			for (int i = 0; i < OCEAN_SURFACE_SIDE_COUNT * OCEAN_SURFACE_SIDE_COUNT; i++)
			{
				m_Ocean[i]->UpdatePixelShaderBuffer(m_PixelShaderSettings);
			}
		}
	}

	ImGui::End();

	ImGui::Begin("Sun Settings", nullptr, windowFlags);

	ImGui::ColorEdit3("Sun Color", (float*)&m_SunSettings.m_SunColor);
	ImGui::SliderFloat("Sun Exponent", &m_SunSettings.m_SunExponent, 0.0f, 50.0f);
	ImGui::SliderFloat("Sun Bias", &m_SunSettings.m_SunBias, 0.0f, 1.0f, "%.4f");

	if (ImGui::Button("Save Settings"))
	{
		std::ofstream outFile("SunSettings.bin", std::ios::binary);
		if (outFile.is_open())
		{
			outFile.write(reinterpret_cast<const char*>(&m_SunSettings), sizeof(m_SunSettings));
			outFile.close();
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Load Settings"))
	{
		std::ifstream inFile("SunSettings.bin", std::ios::binary);
		if (inFile.is_open())
		{
			inFile.read(reinterpret_cast<char*>(&m_SunSettings), sizeof(m_SunSettings));
			inFile.close();
		}
	}

	ImGui::End();

	m_PixelShaderSettings.m_TextureResolution = OceanComputeManager::GetInstance().GetOceanTextureSize();

	for (int i = 0; i < OCEAN_SURFACE_SIDE_COUNT * OCEAN_SURFACE_SIDE_COUNT; i++)
	{
		m_Ocean[i]->UpdatePixelShaderBuffer(m_PixelShaderSettings);
	}
}

bool SceneManager::RegenerateMeshes()
{
	for (int i = 0; i < OCEAN_SURFACE_SIDE_COUNT * OCEAN_SURFACE_SIDE_COUNT; i++)
	{
		if (!m_Ocean[i]->RegenerateMeshAndPos(Vector3((i % OCEAN_SURFACE_SIDE_COUNT - (OCEAN_SURFACE_SIDE_COUNT - 1) / 2) * (OceanComputeManager::GetInstance().GetOceanPatchSize()[0] - 0.0f), 0.0f, (i / OCEAN_SURFACE_SIDE_COUNT - (OCEAN_SURFACE_SIDE_COUNT - 1) / 2) * (OceanComputeManager::GetInstance().GetOceanPatchSize()[0] - 0.0f)))) return false;
		
		m_Ocean[i]->UpdatePixelShaderBuffer(m_PixelShaderSettings);
	}

	return true;
}