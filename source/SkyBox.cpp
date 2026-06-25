#include "SkyBox.h"
#include <fstream>

#include "CameraManager.h"
#include "SceneManager.h"
#include "imgui.h"

SkyBox::SkyBox(string name) : Object(name)
{
    m_VertexShaderFile = m_VertexShaderFilePath;
	m_PixelShaderFile = m_PixelShaderFilePath;

    LoadSkyboxes();

    m_PixelShaderSRVCount = 0;

    if (m_SkyBoxSettings.m_SelectedSkyboxIndex >= 0 && m_SkyBoxSettings.m_SelectedSkyboxIndex < m_SkyboxFiles.size())
    {
        m_FinalSkyBoxTexturePath = m_SkyBoxTexturesPath + "/" + m_SkyboxFiles[m_SkyBoxSettings.m_SelectedSkyboxIndex];

        m_SkyBoxTexture = new Texture2D(SKYBOX_TEXTURE_COUNT, &m_FinalSkyBoxTexturePath, true, true, true, false);

        m_PixelShaderSRVCount = SKYBOX_TEXTURE_COUNT;
    }
}

SkyBox::~SkyBox()
{
    if (m_SkyBoxTexture)
    {
        delete m_SkyBoxTexture;
        m_SkyBoxTexture = nullptr;
    }
}

bool SkyBox::Initialize()
{
    bool result = Object::Initialize();

    m_Initialized = false;

    result = result && m_SkyBoxTexture->Initialize();

    if (m_d3dSamplerState)
    {
        m_d3dSamplerState->Release();
        m_d3dSamplerState = nullptr;
    }

    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
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

    m_Initialized = true;

	return result;
}

void SkyBox::Start()
{
	Object::Start();

    m_Scale *= 1400.0f;
}

void SkyBox::Update()
{
	m_Position = CameraManager::GetInstance().GetCameraPosition();
    m_Position.y = m_SkyBoxSettings.m_SkyboxYValue;

	XMMATRIX scaleMatrix = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYaw(XMConvertToRadians(m_Rotation.x), XMConvertToRadians(m_Rotation.y), XMConvertToRadians(m_Rotation.z));
	XMMATRIX translationMatrix = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);

	XMMATRIX worldMatrix = scaleMatrix * rotationMatrix * translationMatrix;

	m_VertexShaderConstantBufferData = {};
    m_VertexShaderConstantBufferData.m_WorldMatrix = worldMatrix;
    m_VertexShaderConstantBufferData.m_ViewProjectionMatrix = XMMatrixMultiply(CameraManager::GetInstance().GetViewMatrix(), CameraManager::GetInstance().GetProjectionMatrix());

    m_PixelShaderConstantBufferData.m_SunDir = SceneManager::GetInstance().GetLightDirection();
    m_PixelShaderConstantBufferData.m_SunColor = SceneManager::GetInstance().GetSunColor();
    m_PixelShaderConstantBufferData.m_SunExponent = SceneManager::GetInstance().GetSunExponent();
    m_PixelShaderConstantBufferData.m_SunBias = SceneManager::GetInstance().GetSunBias();

	Object::Update();
}

void SkyBox::UpdateUI()
{
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_AlwaysAutoResize;

    ImGui::Begin("Skybox Settings", nullptr, windowFlags);

    const char* comboPreview = m_SkyboxFiles.empty() ? "No skyboxes found" : m_SkyboxFiles[m_SkyBoxSettings.m_SelectedSkyboxIndex].c_str();

    if (ImGui::BeginCombo("Select Skybox", comboPreview))
    {
        for (int i = 0; i < m_SkyboxFiles.size(); ++i)
        {
            const bool isSelected = (m_SkyBoxSettings.m_SelectedSkyboxIndex == i);

            if (ImGui::Selectable(m_SkyboxFiles[i].c_str(), isSelected))
            {
                m_SkyBoxSettings.m_SelectedSkyboxIndex = i;
            }

            if (isSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    ImGui::DragFloat("Skybox Y Value", &m_SkyBoxSettings.m_SkyboxYValue, 1.0f, -500.0f, 500.0f, "%.1f");

    if (m_SkyBoxSettings.m_SelectedSkyboxIndex < 0 || m_SkyBoxSettings.m_SelectedSkyboxIndex >= m_SkyboxFiles.size()) ImGui::BeginDisabled();

    if (ImGui::Button("Apply Changes"))
    {
        ApplySkyboxChanges();
    }

    ImGui::SameLine();

    if (ImGui::Button("Save Settings"))
    {
        SaveSettings("");
    }

    if (m_SkyBoxSettings.m_SelectedSkyboxIndex < 0 || m_SkyBoxSettings.m_SelectedSkyboxIndex >= m_SkyboxFiles.size()) ImGui::EndDisabled();

    ImGui::SameLine();

    if (ImGui::Button("Load Settings"))
    {
        LoadSettings("");
    }

    ImGui::End();
}

UINT SkyBox::GetVertexInputLayout(D3D11_INPUT_ELEMENT_DESC*& inputLayout)
{
	static D3D11_INPUT_ELEMENT_DESC vertexInputLayout[] =
	{
		{
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			offsetof(VertexDataSkyBox, m_Position),
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		}
	};

	inputLayout = vertexInputLayout;

	return _countof(vertexInputLayout);
}

void SkyBox::GenerateMesh()
{
    m_Vertices.clear();
    m_Indices.clear();

    // Define the 8 unique corners of the cube
    m_Vertices.push_back({ XMFLOAT3(-1.0f,  1.0f, -1.0f) }); // 0: Front Top Left
    m_Vertices.push_back({ XMFLOAT3(1.0f,  1.0f, -1.0f) }); // 1: Front Top Right
    m_Vertices.push_back({ XMFLOAT3(-1.0f, -1.0f, -1.0f) }); // 2: Front Bot Left
    m_Vertices.push_back({ XMFLOAT3(1.0f, -1.0f, -1.0f) }); // 3: Front Bot Right
    m_Vertices.push_back({ XMFLOAT3(-1.0f,  1.0f,  1.0f) }); // 4: Back Top Left
    m_Vertices.push_back({ XMFLOAT3(1.0f,  1.0f,  1.0f) }); // 5: Back Top Right
    m_Vertices.push_back({ XMFLOAT3(-1.0f, -1.0f,  1.0f) }); // 6: Back Bot Left
    m_Vertices.push_back({ XMFLOAT3(1.0f, -1.0f,  1.0f) }); // 7: Back Bot Right

    // Helper lambda to generate inward-facing triangles given the 4 corners of a face
    // Corners must be passed in visual "reading order": TopLeft, TopRight, BotLeft, BotRight
    auto AddFaceIndices = [&](int tl, int tr, int bl, int br)
        {
            // Triangle 1
            m_Indices.push_back(tl);
            m_Indices.push_back(bl);
            m_Indices.push_back(tr);

            // Triangle 2
            m_Indices.push_back(bl);
            m_Indices.push_back(br);
            m_Indices.push_back(tr);
        };

    // +Z Face (Back)
    AddFaceIndices(5, 4, 7, 6);

    // -Z Face (Front)
    AddFaceIndices(0, 1, 2, 3);

    // +X Face (Right)
    AddFaceIndices(1, 5, 3, 7);

    // -X Face (Left)
    AddFaceIndices(4, 0, 6, 2);

    // +Y Face (Top)
    AddFaceIndices(4, 5, 0, 1);

    // -Y Face (Bottom)
    AddFaceIndices(2, 3, 6, 7);
}

ID3D11ShaderResourceView* const* SkyBox::GetPixelShaderSRVs()
{
    return m_SkyBoxTexture->GetTextureSRVs();
}

void SkyBox::LoadSkyboxes()
{
    // 1. Clear the existing list so we don't duplicate entries
    m_SkyboxFiles.clear();

    // 2. Format the search path for the Windows API
    std::string searchPath = m_SkyBoxTexturesPath;

    // Ensure there is a trailing slash before adding the wildcard
    if (!searchPath.empty() && searchPath.back() != '/' && searchPath.back() != '\\')
    {
        searchPath += "/";
    }

    // Create the directory if it doesn't exist to prevent first-boot crashes
    CreateDirectoryA(m_SkyBoxTexturesPath.c_str(), NULL);

    // The Windows API requires a wildcard to search INSIDE the folder
    searchPath += "*.dds";

    // 3. Query the Operating System
    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findFileData);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        // Directory is empty or inaccessible
        m_SkyBoxSettings.m_SelectedSkyboxIndex = 0;
        return;
    }

    // 4. Loop through everything found in the directory
    do
    {
        m_SkyboxFiles.push_back(findFileData.cFileName);
    } while (FindNextFileA(hFind, &findFileData) != 0);

    // 5. Always close the handle to prevent memory leaks!
    FindClose(hFind);

    // 6. UI Safety: Prevent out-of-bounds selection if folders were deleted
    if (m_SkyBoxSettings.m_SelectedSkyboxIndex >= m_SkyboxFiles.size())
    {
        m_SkyBoxSettings.m_SelectedSkyboxIndex = 0;
    }
}

void SkyBox::SaveSettings(string parentPath)
{
    ofstream outFile(parentPath + "SkyboxSettings.bin", std::ios::binary);

    if (outFile.is_open())
    {
        outFile.write(reinterpret_cast<const char*>(&m_SkyBoxSettings), sizeof(m_SkyBoxSettings));
        outFile.close();
    }
}

void SkyBox::LoadSettings(string parentPath)
{
    ifstream inFile(parentPath + "SkyboxSettings.bin", ios::binary);

    if (inFile.is_open())
    {
        inFile.read(reinterpret_cast<char*>(&m_SkyBoxSettings), sizeof(m_SkyBoxSettings));
        inFile.close();

        ApplySkyboxChanges();
    }
}

void SkyBox::ApplySkyboxChanges()
{
    if (m_SkyBoxSettings.m_SelectedSkyboxIndex >= 0 && m_SkyBoxSettings.m_SelectedSkyboxIndex < m_SkyboxFiles.size())
    {
        if (m_SkyBoxTexture) delete m_SkyBoxTexture; m_SkyBoxTexture = nullptr;

        m_FinalSkyBoxTexturePath = m_SkyBoxTexturesPath + "/" + m_SkyboxFiles[m_SkyBoxSettings.m_SelectedSkyboxIndex];

        m_SkyBoxTexture = new Texture2D(SKYBOX_TEXTURE_COUNT, &m_FinalSkyBoxTexturePath, true, true, true, false);

        if (!m_SkyBoxTexture->Initialize())
        {
            cout << "Failed to initialize skybox texture" << endl;
        }
    }
}