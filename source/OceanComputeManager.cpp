#include "OceanComputeManager.h"
#include "D3D11Application.h"
#include <fstream>

#include "imgui.h"
#include "TimeManager.h"

OceanComputeManager* OceanComputeManager::m_Instance = nullptr;

OceanComputeManager::OceanComputeManager()
{
	m_InitialSpectrumTexture = nullptr;
    m_InitialSpectrumUAV = nullptr;
	m_InitialSpectrumSRV = nullptr;
	m_InitialSpectrumComputeShader = nullptr;

    m_XYDisplacementTexture = nullptr;
    m_XYDisplacementUAV = nullptr;
    m_XYDisplacementSRV = nullptr;
    m_ZDisplacementXXDerivativeTexture = nullptr;
    m_ZDisplacementXXDerivativeUAV = nullptr;
    m_ZDisplacementXXDerivativeSRV = nullptr;
    m_XZYXDerivativeTexture = nullptr;
    m_XZYXDerivativeUAV = nullptr;
    m_XZYXDerivativeSRV = nullptr;
    m_YZZZDerivativeTexture = nullptr;
    m_YZZZDerivativeUAV = nullptr;
    m_YZZZDerivativeSRV = nullptr;
	m_TimeEvolutionComputeShader = nullptr;

	m_FFTComputeShader = nullptr;

    m_d3dOceanSimulationSettingsBuffer = nullptr;
	m_d3dTimeEvolutionBuffer = nullptr;

    m_OceanSimulationSettingsBufferData = {};
	m_TimeEvolutionBufferData = {};
}

OceanComputeManager::~OceanComputeManager()
{
    if (m_InitialSpectrumUAV) m_InitialSpectrumUAV->Release();
    if (m_InitialSpectrumSRV) m_InitialSpectrumSRV->Release();
    if (m_InitialSpectrumTexture) m_InitialSpectrumTexture->Release();

	if (m_XYDisplacementSRV) m_XYDisplacementSRV->Release();
	if (m_XYDisplacementUAV) m_XYDisplacementUAV->Release();
	if (m_XYDisplacementTexture) m_XYDisplacementTexture->Release();
	if (m_ZDisplacementXXDerivativeSRV) m_ZDisplacementXXDerivativeSRV->Release();
	if (m_ZDisplacementXXDerivativeUAV) m_ZDisplacementXXDerivativeUAV->Release();
	if (m_ZDisplacementXXDerivativeTexture) m_ZDisplacementXXDerivativeTexture->Release();
	if (m_XZYXDerivativeSRV) m_XZYXDerivativeSRV->Release();
	if (m_XZYXDerivativeUAV) m_XZYXDerivativeUAV->Release();
	if (m_XZYXDerivativeTexture) m_XZYXDerivativeTexture->Release();
	if (m_YZZZDerivativeSRV) m_YZZZDerivativeSRV->Release();
	if (m_YZZZDerivativeUAV) m_YZZZDerivativeUAV->Release();
	if (m_YZZZDerivativeTexture) m_YZZZDerivativeTexture->Release();

    if (m_InitialSpectrumComputeShader) m_InitialSpectrumComputeShader->Release();
	if (m_TimeEvolutionComputeShader) m_TimeEvolutionComputeShader->Release();
	if (m_FFTComputeShader) m_FFTComputeShader->Release();

	if (m_d3dOceanSimulationSettingsBuffer) m_d3dOceanSimulationSettingsBuffer->Release();
	if (m_d3dTimeEvolutionBuffer) m_d3dTimeEvolutionBuffer->Release();

	if (m_Instance)
	{
		delete m_Instance;
		m_Instance = nullptr;
	}
}

bool OceanComputeManager::Initialize()
{
    if (!m_Instance)
    {
        m_Instance = new OceanComputeManager;

        if (!m_Instance->CreateTextureAndViews())
        {
            return false;
		}

        if (!m_Instance->CreateComputeShaders())
        {
            return false;
		}

		return true;
	}

	return false;
}

void OceanComputeManager::Start()
{
	GenerateInitialSpectrum(true);
}

void OceanComputeManager::Update()
{
	UpdateUI();
    UpdateTimeEvolutionTextures();
	UpdateFFTTextures();
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
    ID3D11DeviceContext* context = D3D11Application::GetInstance().GetDeviceContext();

    context->CSSetShader(m_InitialSpectrumComputeShader, nullptr, 0);

    InitializeOceanSimulationSettings(initial);

	m_OceanSimulationSettingsBufferData.m_OceanTextureSize = m_OceanTextureSize;
	m_OceanSimulationSettingsBufferData.m_PatchSize = m_OceanPatchSize;

    m_OceanSimulationSettingsBufferData.m_PeakFrequency = max(22.0f * m_OceanSimulationSettingsBufferData.m_GravitationalConstant * m_OceanSimulationSettingsBufferData.m_GravitationalConstant / (m_OceanSimulationSettingsBufferData.m_AverageWindSpeed * m_OceanSimulationSettingsBufferData.m_FetchLength), 0.01f);
	m_OceanSimulationSettingsBufferData.m_Alpha = 0.076f * pow(m_OceanSimulationSettingsBufferData.m_AverageWindSpeed * m_OceanSimulationSettingsBufferData.m_AverageWindSpeed / (m_OceanSimulationSettingsBufferData.m_GravitationalConstant * m_OceanSimulationSettingsBufferData.m_FetchLength), 0.22f);
	m_OceanSimulationSettingsBufferData.m_WindAngle = m_OceanSimulationSettingsBufferData.m_WindDirection * (3.14159265f / 180.0f); // Convert to radians

    m_OceanSimulationSettingsBufferData.m_RandomSeed = rand();
	m_OceanSimulationSettingsBufferData.m_Time = TimeManager::GetInstance().GetTime();

    context->UpdateSubresource(m_d3dOceanSimulationSettingsBuffer, 0, nullptr, &m_OceanSimulationSettingsBufferData, 0, 0);
    context->CSSetConstantBuffers(0, 1, &m_d3dOceanSimulationSettingsBuffer);

    context->CSSetUnorderedAccessViews(0, 1, &m_InitialSpectrumUAV, nullptr);

    UINT groupsX = m_OceanTextureSize / 16;
    UINT groupsY = m_OceanTextureSize / 16;
    context->Dispatch(groupsX, groupsY, 1);

    ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
    context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
}

void OceanComputeManager::UpdateTimeEvolutionTextures()
{
    ID3D11DeviceContext* context = D3D11Application::GetInstance().GetDeviceContext();

    context->CSSetShader(m_TimeEvolutionComputeShader, nullptr, 0);

	m_TimeEvolutionBufferData.m_OceanTextureSize = m_OceanTextureSize;
	m_TimeEvolutionBufferData.m_PatchSize = m_OceanPatchSize;
	m_TimeEvolutionBufferData.m_DensityOfWater = m_DensityOfWater;
	m_TimeEvolutionBufferData.m_SurfaceTension = m_SurfaceTension;
	m_TimeEvolutionBufferData.m_GravitationalConstant = m_GravitationalConstant;
	m_TimeEvolutionBufferData.m_OceanDepth = m_OceanDepth;
    m_TimeEvolutionBufferData.m_Time = TimeManager::GetInstance().GetTime();

    context->UpdateSubresource(m_d3dTimeEvolutionBuffer, 0, nullptr, &m_TimeEvolutionBufferData, 0, 0);
    context->CSSetConstantBuffers(0, 1, &m_d3dTimeEvolutionBuffer);

    context->CSSetShaderResources(0, 1, &m_InitialSpectrumSRV);
    ID3D11UnorderedAccessView* allUAVs[4] = { m_XYDisplacementUAV, m_ZDisplacementXXDerivativeUAV, m_XZYXDerivativeUAV, m_YZZZDerivativeUAV };
    context->CSSetUnorderedAccessViews(0, 4, allUAVs, nullptr);

    UINT groupsX = m_OceanTextureSize / 16;
    UINT groupsY = m_OceanTextureSize / 16;
    context->Dispatch(groupsX, groupsY, 1);

    ID3D11UnorderedAccessView* nullUAVs[4] = { nullptr, nullptr, nullptr, nullptr };
    context->CSSetUnorderedAccessViews(0, 4, nullUAVs, nullptr);

    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    context->CSSetShaderResources(0, 1, nullSRV);
}

void OceanComputeManager::UpdateFFTTextures()
{
    ID3D11DeviceContext* context = D3D11Application::GetInstance().GetDeviceContext();

    context->CSSetShader(m_FFTComputeShader, nullptr, 0);

    context->CSSetUnorderedAccessViews(0, 1, &m_XYDisplacementUAV, nullptr);

    UINT groupsX = m_OceanTextureSize / 16;
    UINT groupsY = m_OceanTextureSize / 16;
    context->Dispatch(groupsX, groupsY, 1);

    // 4. CRITICAL: Unbind the UAV!
    // A resource CANNOT be bound as a UAV (for writing) and an SRV (for reading) at the same time.
    // We must unbind it from the CS stage so the Domain Shader can read it safely later.
    ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
    context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
}

void OceanComputeManager::UpdateUI()
{
    ImGui::Begin("Ocean Simulation Settings");

    ImGui::SliderFloat("Wind Direction (degrees)", &m_WindDirection, 0.0f, 359.9f);
    ImGui::SliderFloat("Average Wind Speed (m/s)", &m_AverageWindSpeed, 0.0f, 100.0f);
    ImGui::SliderFloat("Fetch Length (m)", &m_FetchLength, 0.0f, 500000.0f);
    ImGui::SliderFloat("Peak Enhancement Factor", &m_PeakEnhancementFactor, 0.0f, 7.0f);
    ImGui::SliderFloat("Swell", &m_Swell, 0.0f, 1.0f);
    ImGui::SliderFloat("Ocean Depth (m)", &m_OceanDepth, 0.0f, 10000.0f);
    ImGui::SliderFloat("Density of Water (kg/m^3)", &m_DensityOfWater, 500.0f, 2000.0f);
    ImGui::SliderFloat("Surface Tension (N/m)", &m_SurfaceTension, 0.0f, 0.2f);
    ImGui::SliderFloat("Gravitational Constant (m/s^2)", &m_GravitationalConstant, 0.0f, 20.0f);

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
        std::ifstream inFile("OceanSimulationSettings.bin", std::ios::binary);
        if (inFile.is_open())
        {
            inFile.read(reinterpret_cast<char*>(&m_OceanSimulationSettingsBufferData), sizeof(m_OceanSimulationSettingsBufferData));
            inFile.close();

            GenerateInitialSpectrum(false);
        }
    }

    ImGui::End();
}

bool OceanComputeManager::CreateTextureAndViews()
{
    ID3D11Device* device = D3D11Application::GetInstance().GetDevice();

    // Resource descriptors
    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = m_Instance->m_OceanTextureSize;
    texDesc.Height = m_Instance->m_OceanTextureSize;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = texDesc.Format;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    // Create resources for the initial spectrum
    if (FAILED(device->CreateTexture2D(&texDesc, nullptr, &m_Instance->m_InitialSpectrumTexture)))
    {
        return false;
    }

    if (FAILED(device->CreateUnorderedAccessView(m_Instance->m_InitialSpectrumTexture, &uavDesc, &m_Instance->m_InitialSpectrumUAV)))
    {
        return false;
    }

    if (FAILED(device->CreateShaderResourceView(m_Instance->m_InitialSpectrumTexture, &srvDesc, &m_Instance->m_InitialSpectrumSRV)))
    {
        return false;
    }

    D3D11_BUFFER_DESC constantBufferDesc = {};
    constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.ByteWidth = sizeof(OceanSimulationSettings);
    constantBufferDesc.CPUAccessFlags = 0;
    constantBufferDesc.Usage = D3D11_USAGE_DEFAULT;

    if (FAILED(device->CreateBuffer(
        &constantBufferDesc,
        nullptr,
        &m_Instance->m_d3dOceanSimulationSettingsBuffer)))
    {
        cout << "D3D11: Failed to create ocean simulation settings buffer\n";
        return false;
    }

	// Create resources for the fft displacement and slope textures
    if (FAILED(device->CreateTexture2D(&texDesc, nullptr, &m_Instance->m_XYDisplacementTexture)))
    {
        return false;
    }

    if (FAILED(device->CreateUnorderedAccessView(m_Instance->m_XYDisplacementTexture, &uavDesc, &m_Instance->m_XYDisplacementUAV)))
    {
        return false;
    }

    if (FAILED(device->CreateShaderResourceView(m_Instance->m_XYDisplacementTexture, &srvDesc, &m_Instance->m_XYDisplacementSRV)))
    {
        return false;
    }

    if (FAILED(device->CreateTexture2D(&texDesc, nullptr, &m_Instance->m_ZDisplacementXXDerivativeTexture)))
    {
        return false;
    }

    if (FAILED(device->CreateUnorderedAccessView(m_Instance->m_ZDisplacementXXDerivativeTexture, &uavDesc, &m_Instance->m_ZDisplacementXXDerivativeUAV)))
    {
        return false;
    }

    if (FAILED(device->CreateShaderResourceView(m_Instance->m_ZDisplacementXXDerivativeTexture, &srvDesc, &m_Instance->m_ZDisplacementXXDerivativeSRV)))
    {
        return false;
    }

    if (FAILED(device->CreateTexture2D(&texDesc, nullptr, &m_Instance->m_XZYXDerivativeTexture)))
    {
        return false;
    }

    if (FAILED(device->CreateUnorderedAccessView(m_Instance->m_XZYXDerivativeTexture, &uavDesc, &m_Instance->m_XZYXDerivativeUAV)))
    {
        return false;
    }

    if (FAILED(device->CreateShaderResourceView(m_Instance->m_XZYXDerivativeTexture, &srvDesc, &m_Instance->m_XZYXDerivativeSRV)))
    {
        return false;
    }

    if (FAILED(device->CreateTexture2D(&texDesc, nullptr, &m_Instance->m_YZZZDerivativeTexture)))
    {
        return false;
    }

    if (FAILED(device->CreateUnorderedAccessView(m_Instance->m_YZZZDerivativeTexture, &uavDesc, &m_Instance->m_YZZZDerivativeUAV)))
    {
        return false;
    }

    if (FAILED(device->CreateShaderResourceView(m_Instance->m_YZZZDerivativeTexture, &srvDesc, &m_Instance->m_YZZZDerivativeSRV)))
    {
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
        &m_Instance->m_d3dTimeEvolutionBuffer)))
    {
        cout << "D3D11: Failed to create time evolution buffer\n";
        return false;
    }

    return true;
}

bool OceanComputeManager::CreateComputeShaders()
{

#if _DEBUG
    m_Instance->m_InitialSpectrumComputeShader = D3D11Application::GetInstance().CreateComputeShader(m_Instance->m_InitialSpectrumComputeShaderFile);
#else
    m_Instance->m_InitialSpectrumComputeShader = D3D11Application::GetInstance().CreateComputeShader(L"../../" + m_Instance->m_InitialSpectrumComputeShaderFile);
#endif

    if (m_Instance->m_InitialSpectrumComputeShader == nullptr)
    {
        return false;
    }

#if _DEBUG
    m_Instance->m_TimeEvolutionComputeShader = D3D11Application::GetInstance().CreateComputeShader(m_Instance->m_TimeEvolutionComputeShaderFile);
#else
    m_Instance->m_TimeEvolutionComputeShader = D3D11Application::GetInstance().CreateComputeShader(L"../../" + m_Instance->m_TimeEvolutionComputeShaderFile);
#endif

    if (m_Instance->m_TimeEvolutionComputeShader == nullptr)
    {
        return false;
    }

#if _DEBUG
    m_Instance->m_FFTComputeShader = D3D11Application::GetInstance().CreateComputeShader(m_Instance->m_FFTComputeShaderFile);
#else
    m_Instance->m_FFTComputeShader = D3D11Application::GetInstance().CreateComputeShader(L"../../" + m_Instance->m_FFTComputeShaderFile);
#endif

    if (m_Instance->m_FFTComputeShader == nullptr)
    {
        return false;
    }

	return true;
}