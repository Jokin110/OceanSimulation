#include "OceanComputeManager.h"
#include "D3D11Application.h"
#include <fstream>

#include "imgui.h"
#include "TimeManager.h"
#include "FFTManager.h"
#include "SceneManager.h"

#define PI 3.14159265358979323846f

OceanComputeManager* OceanComputeManager::m_Instance = nullptr;

OceanComputeManager::OceanComputeManager()
{
    m_InitialSpectrumTexture = nullptr;
    m_XYDisplacementTexture = nullptr;
    m_XYDisplacementPingPongTexture = nullptr;
    m_ZDisplacementXXDerivativeTexture = nullptr;
    m_ZDisplacementXXDerivativePingPongTexture = nullptr;
    m_XZYXDerivativeTexture = nullptr;
    m_XZYXDerivativePingPongTexture = nullptr;
    m_YZZZDerivativeTexture = nullptr;
    m_YZZZDerivativePingPongTexture = nullptr;

    m_DisplacementTexture = nullptr;
    m_SlopeTexture = nullptr;

	m_InitialSpectrumComputeShader = nullptr;
	m_TimeEvolutionComputeShader = nullptr;
	m_DisplacementAndSlopeComputeShader = nullptr;

    m_d3dOceanSimulationSettingsBuffer = nullptr;
	m_d3dTimeEvolutionBuffer = nullptr;
	m_d3dDisplacementAndSlopeBuffer = nullptr;

    m_OceanSimulationSettingsBufferData = {};
	m_TimeEvolutionBufferData = {};
	m_DisplacementAndSlopeBufferData = {};

	RecalculateFrequencyFilters();
}

OceanComputeManager::~OceanComputeManager()
{
    if (m_InitialSpectrumComputeShader) m_InitialSpectrumComputeShader->Release();
	if (m_TimeEvolutionComputeShader) m_TimeEvolutionComputeShader->Release();
	if (m_DisplacementAndSlopeComputeShader) m_DisplacementAndSlopeComputeShader->Release();

	ReleaseTextureResources();

	if (m_Instance)
	{
		m_Instance = nullptr;
	}
}

bool OceanComputeManager::Initialize()
{
    if (!m_Instance)
    {
        m_Instance = new OceanComputeManager;

        std::ifstream cascadeSettingsFile("CascadeSettings.bin", std::ios::binary);
        if (cascadeSettingsFile.is_open())
        {
            cascadeSettingsFile.read(reinterpret_cast<char*>(&m_Instance->m_OceanSimulationCascadeSettings), sizeof(OceanSimulationCascadeSettings));
            cascadeSettingsFile.close();
        }

        if (!m_Instance->CreateTextureAndViews())
        {
			cout << "Failed to create textures and views for ocean simulation." << endl;
            return false;
		}

        if (!m_Instance->CreateBuffers())
		{
            cout << "Failed to create buffers for ocean simulation." << endl;
			return false;
		}

        if (!m_Instance->CreateComputeShaders())
        {
			cout << "Failed to create compute shaders for ocean simulation." << endl;
            return false;
		}

        std::ifstream tessellationSettingsFile("TessellationSettings.bin", std::ios::binary);
        if (tessellationSettingsFile.is_open())
        {
            tessellationSettingsFile.read(reinterpret_cast<char*>(&m_Instance->m_TessellationSettingsData), sizeof(TessellationSettingsData));
            tessellationSettingsFile.close();
        }

        m_Instance->m_DisplacementAndSlopeBufferData.m_ChoppinessFactor = 1.0f;

        m_Instance->ApplyCascadeSettings(false);

		return true;
	}

	return false;
}

void OceanComputeManager::Start()
{
    FFTManager::GetInstance().PrecomputeTwiddleFactors();

    GenerateInitialSpectrum(true);
}

void OceanComputeManager::Update()
{
	UpdateUI();
    UpdateTimeEvolutionTextures();
	UpdateFFTTextures();
	GenerateDisplacementAndSlopeFinalTextures();
}

void OceanComputeManager::InitializeOceanSimulationSettings(bool initial)
{
    if (initial)
    {
        m_OceanSimulationSettingsBufferData = {};

        m_OceanSimulationSettingsBufferData.m_DensityOfWater = 1000.0f;
        m_OceanSimulationSettingsBufferData.m_SurfaceTension = 0.074f;
        m_OceanSimulationSettingsBufferData.m_GravitationalConstant = 9.81f;
        m_OceanSimulationSettingsBufferData.m_OceanDepth = 4000.0f;
        m_OceanSimulationSettingsBufferData.m_WindDirection = 0.0f;
        m_OceanSimulationSettingsBufferData.m_AverageWindSpeed = 10.0f;
        m_OceanSimulationSettingsBufferData.m_FetchLength = 1000.0f;
        m_OceanSimulationSettingsBufferData.m_PeakEnhancementFactor = 3.3f;
        m_OceanSimulationSettingsBufferData.m_Swell = 0.0f;

        std::ifstream inFile("OceanSimulationSettings.bin", std::ios::binary);

        if (inFile.is_open())
        {
            inFile.read(reinterpret_cast<char*>(&m_OceanSimulationSettingsBufferData), sizeof(m_OceanSimulationSettingsBufferData));
            inFile.close();
        }

        m_DensityOfWater = m_OceanSimulationSettingsBufferData.m_DensityOfWater;
        m_SurfaceTension = m_OceanSimulationSettingsBufferData.m_SurfaceTension;
        m_GravitationalConstant = m_OceanSimulationSettingsBufferData.m_GravitationalConstant;
        m_OceanDepth = m_OceanSimulationSettingsBufferData.m_OceanDepth;
        m_WindDirection = m_OceanSimulationSettingsBufferData.m_WindDirection;
        m_AverageWindSpeed = m_OceanSimulationSettingsBufferData.m_AverageWindSpeed;
        m_FetchLength = m_OceanSimulationSettingsBufferData.m_FetchLength;
        m_PeakEnhancementFactor = m_OceanSimulationSettingsBufferData.m_PeakEnhancementFactor;
        m_Swell = m_OceanSimulationSettingsBufferData.m_Swell;
    }
    else
    {
		m_OceanSimulationSettingsBufferData.m_DensityOfWater = m_DensityOfWater;
        m_OceanSimulationSettingsBufferData.m_SurfaceTension = m_SurfaceTension;
        m_OceanSimulationSettingsBufferData.m_GravitationalConstant = m_GravitationalConstant;
        m_OceanSimulationSettingsBufferData.m_OceanDepth = m_OceanDepth;
        m_OceanSimulationSettingsBufferData.m_WindDirection = m_WindDirection;
        m_OceanSimulationSettingsBufferData.m_AverageWindSpeed = m_AverageWindSpeed;
        m_OceanSimulationSettingsBufferData.m_FetchLength = m_FetchLength;
        m_OceanSimulationSettingsBufferData.m_PeakEnhancementFactor = m_PeakEnhancementFactor;
		m_OceanSimulationSettingsBufferData.m_Swell = m_Swell;
    }
}

void OceanComputeManager::GenerateInitialSpectrum(bool initial)
{
    RecalculateFrequencyFilters();

    ID3D11DeviceContext* context = D3D11Application::GetInstance().GetDeviceContext();

    context->CSSetShader(m_InitialSpectrumComputeShader, nullptr, 0);

    InitializeOceanSimulationSettings(initial);

    m_OceanSimulationSettingsBufferData.m_OceanTextureSize = m_OceanSimulationCascadeSettings.m_OceanTextureSize;

    m_OceanSimulationSettingsBufferData.m_PeakFrequency = pow(22.0f * m_OceanSimulationSettingsBufferData.m_GravitationalConstant * m_OceanSimulationSettingsBufferData.m_GravitationalConstant / max(m_OceanSimulationSettingsBufferData.m_AverageWindSpeed * m_OceanSimulationSettingsBufferData.m_FetchLength, 0.01f), 1.0f / 3.0f);
    m_OceanSimulationSettingsBufferData.m_Alpha = 0.076f * pow(m_OceanSimulationSettingsBufferData.m_AverageWindSpeed * m_OceanSimulationSettingsBufferData.m_AverageWindSpeed / (m_OceanSimulationSettingsBufferData.m_GravitationalConstant * m_OceanSimulationSettingsBufferData.m_FetchLength), 0.22f);
    m_OceanSimulationSettingsBufferData.m_WindAngle = m_OceanSimulationSettingsBufferData.m_WindDirection * (PI / 180.0f); // Convert to radians

    m_OceanSimulationSettingsBufferData.m_RandomSeed = rand();

    for (int i = 0; i < CASCADE_COUNT; i++)
    {
        m_OceanSimulationSettingsBufferData.m_PatchSize = m_OceanSimulationCascadeSettings.m_OceanPatchSize[i];
        m_OceanSimulationSettingsBufferData.m_LowPassFilter = m_LowPassFilters[i];
		m_OceanSimulationSettingsBufferData.m_HighPassFilter = m_HighPassFilters[i];
        m_OceanSimulationSettingsBufferData.m_CascadeAmplitude = m_OceanSimulationCascadeSettings.m_CascadeAmplitudes[i];

        context->UpdateSubresource(m_d3dOceanSimulationSettingsBuffer, 0, nullptr, &m_OceanSimulationSettingsBufferData, 0, 0);
        context->CSSetConstantBuffers(0, 1, &m_d3dOceanSimulationSettingsBuffer);

        context->CSSetUnorderedAccessViews(0, 1, &m_InitialSpectrumTexture->GetTextureUAVs()[i], nullptr);

        UINT groupsX = m_OceanSimulationCascadeSettings.m_OceanTextureSize / 16;
        UINT groupsY = m_OceanSimulationCascadeSettings.m_OceanTextureSize / 16;
        context->Dispatch(groupsX, groupsY, 1);

        ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
        context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
    }
}

void OceanComputeManager::UpdateTimeEvolutionTextures()
{
    ID3D11DeviceContext* context = D3D11Application::GetInstance().GetDeviceContext();

    context->CSSetShader(m_TimeEvolutionComputeShader, nullptr, 0);

	m_TimeEvolutionBufferData.m_OceanTextureSize = m_OceanSimulationCascadeSettings.m_OceanTextureSize;
    m_TimeEvolutionBufferData.m_DensityOfWater = m_OceanSimulationSettingsBufferData.m_DensityOfWater;
    m_TimeEvolutionBufferData.m_SurfaceTension = m_OceanSimulationSettingsBufferData.m_SurfaceTension;
    m_TimeEvolutionBufferData.m_GravitationalConstant = m_OceanSimulationSettingsBufferData.m_GravitationalConstant;
    m_TimeEvolutionBufferData.m_OceanDepth = m_OceanSimulationSettingsBufferData.m_OceanDepth;
    m_TimeEvolutionBufferData.m_Time = TimeManager::GetInstance().GetTime();

    for (int i = 0; i < CASCADE_COUNT; i++)
    {
        m_TimeEvolutionBufferData.m_PatchSize = m_OceanSimulationCascadeSettings.m_OceanPatchSize[i];

        context->UpdateSubresource(m_d3dTimeEvolutionBuffer, 0, nullptr, &m_TimeEvolutionBufferData, 0, 0);
        context->CSSetConstantBuffers(0, 1, &m_d3dTimeEvolutionBuffer);

        context->CSSetShaderResources(0, 1, &m_InitialSpectrumTexture->GetTextureSRVs()[i]);
        ID3D11UnorderedAccessView* allUAVs[4] = { m_XYDisplacementTexture->GetTextureUAVs()[i], m_ZDisplacementXXDerivativeTexture->GetTextureUAVs()[i], m_XZYXDerivativeTexture->GetTextureUAVs()[i], m_YZZZDerivativeTexture->GetTextureUAVs()[i]};
        context->CSSetUnorderedAccessViews(0, 4, allUAVs, nullptr);

        UINT groupsX = m_OceanSimulationCascadeSettings.m_OceanTextureSize / 16;
        UINT groupsY = m_OceanSimulationCascadeSettings.m_OceanTextureSize / 16;
        context->Dispatch(groupsX, groupsY, 1);

        ID3D11UnorderedAccessView* nullUAVs[4] = { nullptr, nullptr, nullptr, nullptr };
        context->CSSetUnorderedAccessViews(0, 4, nullUAVs, nullptr);

        ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
        context->CSSetShaderResources(0, 1, nullSRV);
    }
}

void OceanComputeManager::UpdateFFTTextures()
{
    for (int i = 0; i < CASCADE_COUNT; i++)
    {
        FFTManager::GetInstance().ComputeIFFT2D(m_XYDisplacementTexture->GetTextureUAVs()[i], m_XYDisplacementPingPongTexture->GetTextureUAVs()[i], true, false, true);
        FFTManager::GetInstance().ComputeIFFT2D(m_ZDisplacementXXDerivativeTexture->GetTextureUAVs()[i], m_ZDisplacementXXDerivativePingPongTexture->GetTextureUAVs()[i], true, false, true);
        FFTManager::GetInstance().ComputeIFFT2D(m_XZYXDerivativeTexture->GetTextureUAVs()[i], m_XZYXDerivativePingPongTexture->GetTextureUAVs()[i], true, false, true);
        FFTManager::GetInstance().ComputeIFFT2D(m_YZZZDerivativeTexture->GetTextureUAVs()[i], m_YZZZDerivativePingPongTexture->GetTextureUAVs()[i], true, false, true);
    }
}

void OceanComputeManager::GenerateDisplacementAndSlopeFinalTextures()
{
    ID3D11DeviceContext* context = D3D11Application::GetInstance().GetDeviceContext();

    context->CSSetShader(m_DisplacementAndSlopeComputeShader, nullptr, 0);

	m_DisplacementAndSlopeBufferData.m_FoamBias = SceneManager::GetInstance().GetFoamBias();
	m_DisplacementAndSlopeBufferData.m_DecayFactor = SceneManager::GetInstance().GetDecayFactor();
    m_DisplacementAndSlopeBufferData.m_DeltaTime = TimeManager::GetInstance().GetDeltaTime();
    m_DisplacementAndSlopeBufferData.m_FoamAddition = SceneManager::GetInstance().GetFoamAddition();
    //m_DisplacementAndSlopeBufferData.m_ChoppinessFactor = SceneManager::GetInstance().GetChoppinessFactor();

	context->UpdateSubresource(m_d3dDisplacementAndSlopeBuffer, 0, nullptr, &m_DisplacementAndSlopeBufferData, 0, 0);
	context->CSSetConstantBuffers(0, 1, &m_d3dDisplacementAndSlopeBuffer);

    for (int i = 0; i < CASCADE_COUNT; i++)
    {
        ID3D11ShaderResourceView* allSRVs[4] = { m_XYDisplacementTexture->GetTextureSRVs()[i], m_ZDisplacementXXDerivativeTexture->GetTextureSRVs()[i], m_XZYXDerivativeTexture->GetTextureSRVs()[i], m_YZZZDerivativeTexture->GetTextureSRVs()[i] };
        context->CSSetShaderResources(0, 4, allSRVs);

        ID3D11UnorderedAccessView* allUAVs[2] = { m_DisplacementTexture->GetTextureUAVs()[i], m_SlopeTexture->GetTextureUAVs()[i] };
        context->CSSetUnorderedAccessViews(0, 2, allUAVs, nullptr);

        UINT groupsX = m_OceanSimulationCascadeSettings.m_OceanTextureSize / 16;
        UINT groupsY = m_OceanSimulationCascadeSettings.m_OceanTextureSize / 16;

        context->Dispatch(groupsX, groupsY, 1);

        ID3D11UnorderedAccessView* nullUAVs[2] = { nullptr, nullptr };
        context->CSSetUnorderedAccessViews(0, 2, nullUAVs, nullptr);
        ID3D11ShaderResourceView* nullSRVs[4] = { nullptr, nullptr, nullptr, nullptr };
        context->CSSetShaderResources(0, 4, nullSRVs);
    }
}

void OceanComputeManager::UpdateUI()
{
    UpdateSimulationSettingsUI();

    UpdateGraphicSettingsUI();

    UpdateCascadeSettingsUI();
}

void OceanComputeManager::UpdateSimulationSettingsUI()
{
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_AlwaysAutoResize;
    ImGui::Begin("Ocean Simulation Settings", nullptr, windowFlags);

    ImGui::SliderFloat("Wind Direction (degrees)", &m_WindDirection, 0.0f, 359.9f);
    ImGui::SliderFloat("Average Wind Speed (m/s)", &m_AverageWindSpeed, 0.0f, 100.0f);
    ImGui::SliderFloat("Fetch Length (m)", &m_FetchLength, 0.0f, 500000.0f);
    ImGui::SliderFloat("Peak Enhancement Factor", &m_PeakEnhancementFactor, 0.0f, 7.0f);
    ImGui::SliderFloat("Swell", &m_Swell, 0.0f, 1.0f);
    ImGui::DragFloat("Ocean Depth (m)", &m_OceanDepth, 1.0f, 0.0f, 5000.0f, "%.1f");
    ImGui::SliderFloat("Density of Water (kg/m^3)", &m_DensityOfWater, 500.0f, 2000.0f);
    ImGui::SliderFloat("Surface Tension (N/m)", &m_SurfaceTension, 0.0f, 0.2f);
    ImGui::SliderFloat("Gravitational Constant (m/s^2)", &m_GravitationalConstant, 0.0f, 20.0f);
    ImGui::SliderFloat("Choppiness Factor", &m_DisplacementAndSlopeBufferData.m_ChoppinessFactor, 0.0f, 10.0f);

    if (ImGui::Button("Apply Changes"))
    {
        GenerateInitialSpectrum(false);
    }

    ImGui::SameLine();

    if (ImGui::Button("Save Settings"))
    {
        std::ofstream outFile("OceanSimulationSettings.bin", std::ios::binary);
        if (outFile.is_open())
        {
            outFile.write(reinterpret_cast<const char*>(&m_OceanSimulationSettingsBufferData), sizeof(m_OceanSimulationSettingsBufferData));
            outFile.close();
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Load Settings"))
    {
        GenerateInitialSpectrum(true);
    }

    ImGui::End();
}

void OceanComputeManager::UpdateGraphicSettingsUI()
{
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_AlwaysAutoResize;
    ImGui::Begin("Graphics Settings", nullptr, windowFlags);

    ImGui::TextColored(ImVec4(0.53f, 0.81f, 0.92f, 1.0f), "Rendering Pipeline"); // Sky blue text
    ImGui::Separator();
    ImGui::Spacing();

    const char* rasterizerModes[] = { "Solid", "Wireframe" };

    ImGui::PushItemWidth(150.0f);

    ImGui::Combo("Fill Mode", D3D11Application::GetInstance().GetRasterizerIndex(), rasterizerModes, IM_ARRAYSIZE(rasterizerModes));

    ImGui::PopItemWidth();

    ImGui::TextColored(ImVec4(0.53f, 0.81f, 0.92f, 1.0f), "Tessellation"); // Sky blue text
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::SliderFloat("Min Distance", &m_TessellationSettingsData.m_MinTessellationDistance, 0.0f, 200.0f);
    ImGui::SliderFloat("MaxDistance", &m_TessellationSettingsData.m_MaxTessellationDistance, 200.0f, 5000.0f);
    ImGui::SliderInt("Tessellation Exponent", &m_TessellationSettingsData.m_TessellationExponent, 1, 30);

    if (ImGui::Button("Save Settings"))
    {
        std::ofstream outFile("TessellationSettings.bin", std::ios::binary);
        if (outFile.is_open())
        {
            outFile.write(reinterpret_cast<const char*>(&m_TessellationSettingsData), sizeof(TessellationSettingsData));
            outFile.close();
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Load Settings"))
    {
        std::ifstream inFile("TessellationSettings.bin", std::ios::binary);
        if (inFile.is_open())
        {
            inFile.read(reinterpret_cast<char*>(&m_TessellationSettingsData), sizeof(TessellationSettingsData));
            inFile.close();
        }
    }

    ImGui::End();
}

void OceanComputeManager::UpdateCascadeSettingsUI()
{
    ImGuiWindowFlags windowFlags = ImGuiWindowFlags_AlwaysAutoResize;
    ImGui::Begin("Cascade Settings", nullptr, windowFlags);

    ImGui::PushItemWidth(150.0f);

    ImGui::Combo("Texture Size", &m_TemporaryOceanSimulationCascadeSettings.m_SelectedTextureSizeIndex, m_TextureDropdownNames, IM_ARRAYSIZE(m_TextureDropdownNames));
    m_TemporaryOceanSimulationCascadeSettings.m_OceanTextureSize = m_TextureSizes[m_TemporaryOceanSimulationCascadeSettings.m_SelectedTextureSizeIndex];

    ImGui::PopItemWidth();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextColored(ImVec4(0.53f, 0.81f, 0.92f, 1.0f), "Cascade Parameters");
    ImGui::Spacing();

    if (ImGui::BeginTable("CascadeTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchSame))
    {
        ImGui::TableSetupColumn("Cascade");
        ImGui::TableSetupColumn("Patch Size (m)");
        ImGui::TableSetupColumn("Amplitude Scale");
        ImGui::TableHeadersRow();

        for (int i = 0; i < CASCADE_COUNT; i++)
        {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Spacing();
            ImGui::Text(" Cascade %d", i);

            ImGui::TableSetColumnIndex(1);
            ImGui::PushID(i);
            ImGui::SetNextItemWidth(-FLT_MIN); 
            ImGui::DragFloat("##PatchSize", &m_TemporaryOceanSimulationCascadeSettings.m_OceanPatchSize[i], 1.0f, 1.0f, 10000.0f, "%.1f");
            ImGui::PopID();

            ImGui::TableSetColumnIndex(2);
            ImGui::PushID(i + CASCADE_COUNT);
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::DragFloat("##Amplitude", &m_TemporaryOceanSimulationCascadeSettings.m_CascadeAmplitudes[i], 0.1f, 0.0f, 30.0f, "%.2f");
            ImGui::PopID();
        }
        ImGui::EndTable();
    }

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::DragFloat("Mesh Vertex Separation", &m_TemporaryOceanSimulationCascadeSettings.m_MeshVertexSeparation, 1.0f, 0.0f, 200.0f, "%.1f");
    ImGui::DragFloat("Frequency Filter Multiplier", &m_TemporaryOceanSimulationCascadeSettings.m_FrequencyFilterMultiplier, 0.01f, 0.0f, 1.0f, "%.3f");

    if (ImGui::Button("Apply Changes"))
    {
        ApplyCascadeSettings(true);

        if (!ReinitializeTexturesMeshesAndSpectrum())
        {
            cout << "Couldn't reinitialize textures, meshes and spectrum!";
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Save Settings"))
    {
        ApplyCascadeSettings(true);

        std::ofstream outFile("CascadeSettings.bin", std::ios::binary);
        if (outFile.is_open())
        {
            outFile.write(reinterpret_cast<const char*>(&m_OceanSimulationCascadeSettings), sizeof(OceanSimulationCascadeSettings));
            outFile.close();
        }

        if (!ReinitializeTexturesMeshesAndSpectrum())
        {
            cout << "Couldn't reinitialize textures, meshes and spectrum!";
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Load Settings"))
    {
        std::ifstream inFile("CascadeSettings.bin", std::ios::binary);
        if (inFile.is_open())
        {
            inFile.read(reinterpret_cast<char*>(&m_OceanSimulationCascadeSettings), sizeof(OceanSimulationCascadeSettings));
            inFile.close();

            if (!ReinitializeTexturesMeshesAndSpectrum())
            {
                cout << "Couldn't reinitialize textures, meshes and spectrum!";
            }
        }

        ApplyCascadeSettings(false);
    }

    ImGui::End();
}

bool OceanComputeManager::ReinitializeTexturesMeshesAndSpectrum()
{
    if (!ResizeTextures()) return false;

    if (!FFTManager::GetInstance().ResizeTextures(m_OceanSimulationCascadeSettings.m_OceanTextureSize)) return false;

    GenerateInitialSpectrum(false);

    if (!SceneManager::GetInstance().RegenerateMeshes()) return false;

    return true;
}

void OceanComputeManager::RecalculateFrequencyFilters()
{
    m_LowPassFilters[0] = 0.0f;
    m_HighPassFilters[CASCADE_COUNT - 1] = 9999999999999.0f;

    for (int i = 1; i < CASCADE_COUNT; i++)
    {
        float filter = m_OceanSimulationCascadeSettings.m_FrequencyFilterMultiplier * PI * m_OceanSimulationCascadeSettings.m_OceanTextureSize / m_OceanSimulationCascadeSettings.m_OceanPatchSize[i - 1];

        m_LowPassFilters[i] = filter;
        m_HighPassFilters[i - 1] = filter;
    }
}

bool OceanComputeManager::ResizeTextures()
{
    int textureSize = GetOceanTextureSize();

    if (!m_InitialSpectrumTexture->ResizeTexture(textureSize, textureSize)) return false;
    if (!m_XYDisplacementTexture->ResizeTexture(textureSize, textureSize)) return false;
    if (!m_ZDisplacementXXDerivativeTexture->ResizeTexture(textureSize, textureSize)) return false;
    if (!m_ZDisplacementXXDerivativePingPongTexture->ResizeTexture(textureSize, textureSize)) return false;
    if (!m_XZYXDerivativeTexture->ResizeTexture(textureSize, textureSize)) return false;
    if (!m_XZYXDerivativePingPongTexture->ResizeTexture(textureSize, textureSize)) return false;
    if (!m_YZZZDerivativeTexture->ResizeTexture(textureSize, textureSize)) return false;
    if (!m_YZZZDerivativePingPongTexture->ResizeTexture(textureSize, textureSize)) return false;

    if (!m_DisplacementTexture->ResizeTexture(textureSize, textureSize)) return false;
    if (!m_SlopeTexture->ResizeTexture(textureSize, textureSize)) return false;

    return true;
}

void OceanComputeManager::ApplyCascadeSettings(bool apply)
{
    if (apply)
    {
        m_OceanSimulationCascadeSettings.m_OceanTextureSize = m_TemporaryOceanSimulationCascadeSettings.m_OceanTextureSize;
        m_OceanSimulationCascadeSettings.m_SelectedTextureSizeIndex = m_TemporaryOceanSimulationCascadeSettings.m_SelectedTextureSizeIndex;
        m_OceanSimulationCascadeSettings.m_MeshVertexSeparation = m_TemporaryOceanSimulationCascadeSettings.m_MeshVertexSeparation;
        m_OceanSimulationCascadeSettings.m_FrequencyFilterMultiplier = m_TemporaryOceanSimulationCascadeSettings.m_FrequencyFilterMultiplier;

        for (int i = 0; i < CASCADE_COUNT; i++)
        {
            m_OceanSimulationCascadeSettings.m_OceanPatchSize[i] = m_TemporaryOceanSimulationCascadeSettings.m_OceanPatchSize[i];
            m_OceanSimulationCascadeSettings.m_CascadeAmplitudes[i] = m_TemporaryOceanSimulationCascadeSettings.m_CascadeAmplitudes[i];
        }
    }
    else
    {
        m_TemporaryOceanSimulationCascadeSettings.m_OceanTextureSize = m_OceanSimulationCascadeSettings.m_OceanTextureSize;
        m_TemporaryOceanSimulationCascadeSettings.m_SelectedTextureSizeIndex = m_OceanSimulationCascadeSettings.m_SelectedTextureSizeIndex;
        m_TemporaryOceanSimulationCascadeSettings.m_MeshVertexSeparation = m_OceanSimulationCascadeSettings.m_MeshVertexSeparation;
        m_TemporaryOceanSimulationCascadeSettings.m_FrequencyFilterMultiplier = m_OceanSimulationCascadeSettings.m_FrequencyFilterMultiplier;

        for (int i = 0; i < CASCADE_COUNT; i++)
        {
            m_TemporaryOceanSimulationCascadeSettings.m_OceanPatchSize[i] = m_OceanSimulationCascadeSettings.m_OceanPatchSize[i];
            m_TemporaryOceanSimulationCascadeSettings.m_CascadeAmplitudes[i] = m_OceanSimulationCascadeSettings.m_CascadeAmplitudes[i];
        }
    }
}

bool OceanComputeManager::CreateTextureAndViews()
{
    int textureSize = GetOceanTextureSize();

    m_InitialSpectrumTexture = new Texture2D(CASCADE_COUNT, textureSize, textureSize);
    m_XYDisplacementTexture = new Texture2D(CASCADE_COUNT, textureSize, textureSize);
    m_XYDisplacementPingPongTexture = new Texture2D(CASCADE_COUNT, textureSize, textureSize, false, true);
    m_ZDisplacementXXDerivativeTexture = new Texture2D(CASCADE_COUNT, textureSize, textureSize);
    m_ZDisplacementXXDerivativePingPongTexture = new Texture2D(CASCADE_COUNT, textureSize, textureSize, false, true);
    m_XZYXDerivativeTexture = new Texture2D(CASCADE_COUNT, textureSize, textureSize);
    m_XZYXDerivativePingPongTexture = new Texture2D(CASCADE_COUNT, textureSize, textureSize, false, true);
    m_YZZZDerivativeTexture = new Texture2D(CASCADE_COUNT, textureSize, textureSize);
    m_YZZZDerivativePingPongTexture = new Texture2D(CASCADE_COUNT, textureSize, textureSize, false, true);

    m_DisplacementTexture = new Texture2D(CASCADE_COUNT, textureSize, textureSize);
    m_SlopeTexture = new Texture2D(CASCADE_COUNT, textureSize, textureSize);

    if (!m_InitialSpectrumTexture->Initialize()) return false;
    if (!m_XYDisplacementTexture->Initialize()) return false;
    if (!m_XYDisplacementPingPongTexture->Initialize()) return false;
    if (!m_ZDisplacementXXDerivativeTexture->Initialize()) return false;
    if (!m_ZDisplacementXXDerivativePingPongTexture->Initialize()) return false;
    if (!m_XZYXDerivativeTexture->Initialize()) return false;
    if (!m_XZYXDerivativePingPongTexture->Initialize()) return false;
    if (!m_YZZZDerivativeTexture->Initialize()) return false;
    if (!m_YZZZDerivativePingPongTexture->Initialize()) return false;

    if (!m_DisplacementTexture->Initialize()) return false;
    if (!m_SlopeTexture->Initialize()) return false;

    return true;
}

bool OceanComputeManager::CreateBuffers()
{
    ID3D11Device* device = D3D11Application::GetInstance().GetDevice();

    D3D11_BUFFER_DESC constantBufferDesc = {};
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.ByteWidth = sizeof(OceanSimulationSettings);
    constantBufferDesc.CPUAccessFlags = 0;
    constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

    if (FAILED(device->CreateBuffer(
        &constantBufferDesc,
        nullptr,
        &m_d3dOceanSimulationSettingsBuffer)))
    {
        cout << "D3D11: Failed to create ocean simulation settings buffer\n";
        return false;
    }

    constantBufferDesc = {};
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.ByteWidth = sizeof(TimeEvolutionData);
    constantBufferDesc.CPUAccessFlags = 0;
    constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

    if (FAILED(device->CreateBuffer(
        &constantBufferDesc,
        nullptr,
        &m_d3dTimeEvolutionBuffer)))
    {
        cout << "D3D11: Failed to create time evolution buffer\n";
        return false;
    }

    constantBufferDesc = {};
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.ByteWidth = sizeof(DisplacementAndSlopeData);
    constantBufferDesc.CPUAccessFlags = 0;
    constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

    if (FAILED(device->CreateBuffer(
        &constantBufferDesc,
        nullptr,
        &m_d3dDisplacementAndSlopeBuffer)))
    {
        cout << "D3D11: Failed to create displacement and slope buffer\n";
        return false;
    }

    return true;
}

bool OceanComputeManager::CreateComputeShaders()
{
#if _DEBUG
    m_InitialSpectrumComputeShader = D3D11Application::GetInstance().CreateComputeShader(m_InitialSpectrumComputeShaderFile);
#else
    m_InitialSpectrumComputeShader = D3D11Application::GetInstance().CreateComputeShader(L"../../" + m_InitialSpectrumComputeShaderFile);
#endif

    if (m_InitialSpectrumComputeShader == nullptr)
    {
        return false;
    }

#if _DEBUG
    m_TimeEvolutionComputeShader = D3D11Application::GetInstance().CreateComputeShader(m_TimeEvolutionComputeShaderFile);
#else
    m_TimeEvolutionComputeShader = D3D11Application::GetInstance().CreateComputeShader(L"../../" + m_TimeEvolutionComputeShaderFile);
#endif

    if (m_TimeEvolutionComputeShader == nullptr)
    {
        return false;
    }

#if _DEBUG
	m_DisplacementAndSlopeComputeShader = D3D11Application::GetInstance().CreateComputeShader(m_DisplacementAndSlopeComputeShaderFile);
#else
	m_DisplacementAndSlopeComputeShader = D3D11Application::GetInstance().CreateComputeShader(L"../../" + m_DisplacementAndSlopeComputeShaderFile);
#endif

    if (m_DisplacementAndSlopeComputeShader == nullptr)
    {
        return false;
	}

	return true;
}

void OceanComputeManager::ReleaseTextureResources()
{
    DeleteTextureObject(m_InitialSpectrumTexture);
    DeleteTextureObject(m_XYDisplacementTexture);
    DeleteTextureObject(m_XYDisplacementPingPongTexture);
    DeleteTextureObject(m_ZDisplacementXXDerivativeTexture);
    DeleteTextureObject(m_ZDisplacementXXDerivativePingPongTexture);
    DeleteTextureObject(m_XZYXDerivativeTexture);
    DeleteTextureObject(m_XZYXDerivativePingPongTexture);
    DeleteTextureObject(m_YZZZDerivativeTexture);
    DeleteTextureObject(m_YZZZDerivativePingPongTexture);

    DeleteTextureObject(m_DisplacementTexture);
    DeleteTextureObject(m_SlopeTexture);

    if (m_d3dOceanSimulationSettingsBuffer) m_d3dOceanSimulationSettingsBuffer->Release(); m_d3dOceanSimulationSettingsBuffer = nullptr;
    if (m_d3dTimeEvolutionBuffer) m_d3dTimeEvolutionBuffer->Release(); m_d3dTimeEvolutionBuffer = nullptr; 
    if (m_d3dDisplacementAndSlopeBuffer) m_d3dDisplacementAndSlopeBuffer->Release(); m_d3dDisplacementAndSlopeBuffer = nullptr; 
}

void OceanComputeManager::DeleteTextureObject(Texture2D* &texture)
{
    if (texture)
    {
        delete texture;
        texture = nullptr;
    }
}