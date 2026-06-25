#include "SceneManager.h"
#include "imgui.h"
#include <fstream>
#include <windows.h>

#include "CameraManager.h"
#include "OceanComputeManager.h"

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

		m_Instance->m_PixelShaderSettings.m_Cascade0FoamWeight = 1.0f;
		m_Instance->m_PixelShaderSettings.m_Cascade1FoamWeight = 1.0f;
		m_Instance->m_PixelShaderSettings.m_Cascade2FoamWeight = 1.0f;
		m_Instance->m_PixelShaderSettings.m_Cascade3FoamWeight = 1.0f;

		ifstream inFile("OceanSettings.bin", ios::binary);
		if (inFile.is_open())
		{
			inFile.read(reinterpret_cast<char*>(&m_Instance->m_PixelShaderSettings), sizeof(m_Instance->m_PixelShaderSettings));
			inFile.close();
		}

		m_Instance->m_SunSettings.m_SunColor = XMFLOAT3(1.0f, 0.7f, 0.1f);
		m_Instance->m_SunSettings.m_SunExponent = 2.0f;
		m_Instance->m_SunSettings.m_SunBias = 0.7f;

		ifstream sunSettings("SunSettings.bin", ios::binary);
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
	LoadSceneNames();

	LoadSceneSettings();

	for (int i = 0; i < OCEAN_SURFACE_SIDE_COUNT * OCEAN_SURFACE_SIDE_COUNT; i++)
	{
		m_Ocean[i]->UpdatePixelShaderBuffer(m_PixelShaderSettings);
	}
}

void SceneManager::Update()
{
	m_PixelShaderSettings.m_TextureResolution = OceanComputeManager::GetInstance().GetOceanTextureSize();

	for (int i = 0; i < OCEAN_SURFACE_SIDE_COUNT * OCEAN_SURFACE_SIDE_COUNT; i++)
	{
		m_Ocean[i]->UpdatePixelShaderBuffer(m_PixelShaderSettings);
	}
}

void SceneManager::UpdateUI()
{
	ImGuiWindowFlags windowFlags = ImGuiWindowFlags_AlwaysAutoResize;
	ImGui::Begin("Ocean Surface Rendering Settings", nullptr, windowFlags);

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

	ImGui::Separator();

	ImGui::ColorEdit3("Foam Color", (float*)&m_PixelShaderSettings.m_FoamColor);
	ImGui::SliderFloat("Foam Bias", &m_PixelShaderSettings.m_FoamBias, 0.0f, 1.0f);
	ImGui::SliderFloat("Foam Decay Factor", &m_PixelShaderSettings.m_DecayFactor, 0.0f, 1.0f);
	ImGui::SliderFloat("Foam Addition", &m_PixelShaderSettings.m_FoamAddition, 0.0f, 5.0f);
	ImGui::SliderFloat("Foam Roughness", &m_PixelShaderSettings.m_FoamRoughnessMultiplier, 0.0f, 5.0f);
	ImGui::SliderFloat("Foam Cascade 0 Weight", &m_PixelShaderSettings.m_Cascade0FoamWeight, 0.0f, 5.0f);
	ImGui::SliderFloat("Foam Cascade 1 Weight", &m_PixelShaderSettings.m_Cascade1FoamWeight, 0.0f, 5.0f);
	ImGui::SliderFloat("Foam Cascade 2 Weight", &m_PixelShaderSettings.m_Cascade2FoamWeight, 0.0f, 5.0f);
	ImGui::SliderFloat("Foam Cascade 3 Weight", &m_PixelShaderSettings.m_Cascade3FoamWeight, 0.0f, 5.0f);

	ImGui::Separator();

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
		ofstream outFile("OceanSettings.bin", ios::binary);
		if (outFile.is_open())
		{
			outFile.write(reinterpret_cast<const char*>(&m_PixelShaderSettings), sizeof(m_PixelShaderSettings));
			outFile.close();
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Load Settings"))
	{
		ifstream inFile("OceanSettings.bin", ios::binary);
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
		ofstream outFile("SunSettings.bin", ios::binary);
		if (outFile.is_open())
		{
			outFile.write(reinterpret_cast<const char*>(&m_SunSettings), sizeof(m_SunSettings));
			outFile.close();
		}
	}

	ImGui::SameLine();

	if (ImGui::Button("Load Settings"))
	{
		ifstream inFile("SunSettings.bin", ios::binary);
		if (inFile.is_open())
		{
			inFile.read(reinterpret_cast<char*>(&m_SunSettings), sizeof(m_SunSettings));
			inFile.close();
		}
	}

	ImGui::End();

	ImGui::Begin("Scene Settings", nullptr, windowFlags);

	// 1. Scene Selection Dropdown
	// Get the name of the currently selected scene for the combo box preview
	const char* comboPreview = m_SceneFolders.empty() ? "No scenes found" : m_SceneFolders[m_SelectedSceneIndex].c_str();

	if (ImGui::BeginCombo("Select Scene", comboPreview))
	{
		for (int i = 0; i < m_SceneFolders.size(); ++i)
		{
			const bool isSelected = (m_SelectedSceneIndex == i);

			if (ImGui::Selectable(m_SceneFolders[i].c_str(), isSelected))
			{
				m_SelectedSceneIndex = i;

				// Optional: Auto-fill the save input box with the selected scene name
				strncpy_s(m_SceneNameInput, sizeof(m_SceneNameInput), m_SceneFolders[i].c_str(), _TRUNCATE);
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (isSelected)
			{
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndCombo();
	}

	// 2. Load Button
	// Disable the load button if there are no scenes to load
	if (m_SceneFolders.empty()) ImGui::BeginDisabled();

	if (ImGui::Button("Load Selected Scene"))
	{
		LoadSceneSettings();
	}

	if (m_SceneFolders.empty()) ImGui::EndDisabled();

	ImGui::Spacing();
	ImGui::Separator();
	ImGui::Spacing();

	// 3. Save Scene Text Input
	ImGui::InputText("Scene Name", m_SceneNameInput, IM_ARRAYSIZE(m_SceneNameInput));

	// 4. Save Button
	// Disable the save button if the text input is empty
	bool isInputEmpty = (m_SceneNameInput[0] == '\0');
	if (isInputEmpty) ImGui::BeginDisabled();

	if (ImGui::Button("Save Scene"))
	{
		SaveSceneSettings();

		LoadSceneNames();
	}

	if (isInputEmpty) ImGui::EndDisabled();

	ImGui::End();
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

void SceneManager::SaveSettings(string parentPath)
{
	ofstream oceanFile(parentPath + "OceanSettings.bin", ios::binary);
	if (oceanFile.is_open())
	{
		oceanFile.write(reinterpret_cast<const char*>(&m_PixelShaderSettings), sizeof(m_PixelShaderSettings));
		oceanFile.close();
	}

	ofstream sunFile(parentPath + "SunSettings.bin", ios::binary);
	if (sunFile.is_open())
	{
		sunFile.write(reinterpret_cast<const char*>(&m_SunSettings), sizeof(m_SunSettings));
		sunFile.close();
	}
}

void SceneManager::LoadSettings(string parentPath)
{
	ifstream oceanFile(parentPath + "OceanSettings.bin", ios::binary);
	if (oceanFile.is_open())
	{
		oceanFile.read(reinterpret_cast<char*>(&m_PixelShaderSettings), sizeof(m_PixelShaderSettings));
		oceanFile.close();

		for (int i = 0; i < OCEAN_SURFACE_SIDE_COUNT * OCEAN_SURFACE_SIDE_COUNT; i++)
		{
			m_Ocean[i]->UpdatePixelShaderBuffer(m_PixelShaderSettings);
		}
	}

	ifstream sunFile(parentPath + "SunSettings.bin", ios::binary);
	if (sunFile.is_open())
	{
		sunFile.read(reinterpret_cast<char*>(&m_SunSettings), sizeof(m_SunSettings));
		sunFile.close();
	}
}

void SceneManager::LoadSceneNames()
{
	// 1. Clear the existing list so we don't duplicate entries
	m_SceneFolders.clear();

	// 2. Format the search path for the Windows API
	string searchPath = m_SceneSettingsSavePath;

	// Ensure there is a trailing slash before adding the wildcard
	if (!searchPath.empty() && searchPath.back() != '/' && searchPath.back() != '\\')
	{
		searchPath += "/";
	}

	// Create the directory if it doesn't exist to prevent first-boot crashes
	CreateDirectoryA(m_SceneSettingsSavePath.c_str(), NULL);

	// The Windows API requires a wildcard to search INSIDE the folder
	searchPath += "*";

	// 3. Query the Operating System
	WIN32_FIND_DATAA findFileData;
	HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findFileData);

	if (hFind == INVALID_HANDLE_VALUE)
	{
		// Directory is empty or inaccessible
		m_SelectedSceneIndex = 0;
		return;
	}

	// 4. Loop through everything found in the directory
	do
	{
		// Check if the item is actually a directory (not a file like a .txt or .dds)
		if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			string folderName = findFileData.cFileName;

			// Ignore the hidden Windows navigation folders "." and ".."
			if (folderName != "." && folderName != "..")
			{
				m_SceneFolders.push_back(folderName);
			}
		}
	} while (FindNextFileA(hFind, &findFileData) != 0);

	// 5. Always close the handle to prevent memory leaks!
	FindClose(hFind);

	// 6. UI Safety: Prevent out-of-bounds selection if folders were deleted
	if (m_SelectedSceneIndex >= m_SceneFolders.size())
	{
		m_SelectedSceneIndex = 0;
	}
}

void SceneManager::SaveSceneSettings()
{
	string newFolderToCreate = m_SceneSettingsSavePath + "/" + string(m_SceneNameInput) + "/";

	CreateDirectoryA(newFolderToCreate.c_str(), NULL);

	SaveSettings(newFolderToCreate);
	CameraManager::GetInstance().SaveSettings(newFolderToCreate);
	m_SkyBox->SaveSettings(newFolderToCreate);
	m_FogPostProcessEffect->SaveSettings(newFolderToCreate);
	OceanComputeManager::GetInstance().SaveSettings(newFolderToCreate);
}

void SceneManager::LoadSceneSettings()
{
	string folderToLoad = "";

	if (m_SelectedSceneIndex >= 0 && m_SelectedSceneIndex < m_SceneFolders.size())
 		folderToLoad = m_SceneSettingsSavePath + "/" + m_SceneFolders[m_SelectedSceneIndex] + "/";

	LoadSettings(folderToLoad);
	CameraManager::GetInstance().LoadSettings(folderToLoad);
	m_SkyBox->LoadSettings(folderToLoad);
	m_FogPostProcessEffect->LoadSettings(folderToLoad);
	OceanComputeManager::GetInstance().LoadSettings(folderToLoad);
}