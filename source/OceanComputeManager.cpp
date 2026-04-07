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
	m_FFTComputeShader = nullptr;

    m_d3dOceanSimulationSettingsBuffer = nullptr;

    m_OceanSimulationSettingsBufferData = {};
}

OceanComputeManager::~OceanComputeManager()
{
    if (m_InitialSpectrumTexture) m_InitialSpectrumTexture->Release();
    if (m_InitialSpectrumUAV) m_InitialSpectrumUAV->Release();
    if (m_InitialSpectrumSRV) m_InitialSpectrumSRV->Release();
    if (m_InitialSpectrumComputeShader) m_InitialSpectrumComputeShader->Release();

	if (m_XYDisplacementSRV) m_XYDisplacementSRV->Release();
	if (m_XYDisplacementUAV) m_XYDisplacementUAV->Release();
	if (m_XYDisplacementTexture) m_XYDisplacementTexture->Release();
	if (m_FFTComputeShader) m_FFTComputeShader->Release();

	if (m_d3dOceanSimulationSettingsBuffer) m_d3dOceanSimulationSettingsBuffer->Release();

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

        ID3D11Device* device = D3D11Application::GetInstance().GetDevice();

		// Create resources for the initial spectrum
        D3D11_TEXTURE2D_DESC texDesc = {};
        texDesc.Width = m_Instance->m_OceanTextureSize;
        texDesc.Height = m_Instance->m_OceanTextureSize;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;

        if (FAILED(device->CreateTexture2D(&texDesc, nullptr, &m_Instance->m_InitialSpectrumTexture)))
        {
            return false;
        }

        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.Format = texDesc.Format;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;

        if (FAILED(device->CreateUnorderedAccessView(m_Instance->m_InitialSpectrumTexture, &uavDesc, &m_Instance->m_InitialSpectrumUAV)))
        {
            return false;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.Format = texDesc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = 1;

        if (FAILED(device->CreateShaderResourceView(m_Instance->m_InitialSpectrumTexture, &srvDesc, &m_Instance->m_InitialSpectrumSRV)))
        {
            return false;
        }

#if _DEBUG
        m_Instance->m_InitialSpectrumComputeShader = D3D11Application::GetInstance().CreateComputeShader(m_Instance->m_InitialSpectrumComputeShaderFile);
#else
        m_Instance->m_InitialSpectrumComputeShader = D3D11Application::GetInstance().CreateComputeShader(L"../../" + m_Instance->m_InitialSpectrumComputeShaderFile);
#endif

        if (m_Instance->m_InitialSpectrumComputeShader == nullptr)
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

		// Create resources for the time evolution compute shader
#if _DEBUG
        m_Instance->m_TimeEvolutionComputeShader = D3D11Application::GetInstance().CreateComputeShader(m_Instance->m_TimeEvolutionComputeShaderFile);
#else
        m_Instance->m_TimeEvolutionComputeShader = D3D11Application::GetInstance().CreateComputeShader(L"../../" + m_Instance->m_TimeEvolutionComputeShaderFile);
#endif

        if (m_Instance->m_TimeEvolutionComputeShader == nullptr)
        {
            return false;
        }

        // Create resources for the fft displacement
        texDesc = {};
        texDesc.Width = m_Instance->m_OceanTextureSize;
        texDesc.Height = m_Instance->m_OceanTextureSize;
        texDesc.MipLevels = 1;
        texDesc.ArraySize = 1;
        texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
        texDesc.SampleDesc.Count = 1;
        texDesc.Usage = D3D11_USAGE_DEFAULT;
        texDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;

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

	return false;
}

void OceanComputeManager::Start()
{
	GenerateInitialSpectrum(true);
}

void OceanComputeManager::Update()
{
    UpdateTimeEvolutionTextures();
	UpdateFFTTextures();
	UpdateUI();
}

void OceanComputeManager::GenerateInitialSpectrum(bool initial)
{
    ID3D11DeviceContext* context = D3D11Application::GetInstance().GetDeviceContext();

    context->CSSetShader(m_InitialSpectrumComputeShader, nullptr, 0);

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
    }

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

    m_OceanSimulationSettingsBufferData.m_Time = TimeManager::GetInstance().GetTime();

    context->UpdateSubresource(m_d3dOceanSimulationSettingsBuffer, 0, nullptr, &m_OceanSimulationSettingsBufferData, 0, 0);
    context->CSSetConstantBuffers(0, 1, &m_d3dOceanSimulationSettingsBuffer);

    context->CSSetShaderResources(0, 1, &m_InitialSpectrumSRV);
    context->CSSetUnorderedAccessViews(0, 1, &m_XYDisplacementUAV, nullptr);

    UINT groupsX = m_OceanTextureSize / 16;
    UINT groupsY = m_OceanTextureSize / 16;
    context->Dispatch(groupsX, groupsY, 1);

    ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
    context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
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

    ImGui::SliderFloat("Wind Direction (degrees)", &m_OceanSimulationSettingsBufferData.m_WindDirection, 0.0f, 359.9f);
    ImGui::SliderFloat("Average Wind Speed (m/s)", &m_OceanSimulationSettingsBufferData.m_AverageWindSpeed, 0.0f, 100.0f);
    ImGui::SliderFloat("Fetch Length (m)", &m_OceanSimulationSettingsBufferData.m_FetchLength, 0.0f, 500000.0f);
    ImGui::SliderFloat("Peak Enhancement Factor", &m_OceanSimulationSettingsBufferData.m_PeakEnhancementFactor, 0.0f, 7.0f);
    ImGui::SliderFloat("Swell", &m_OceanSimulationSettingsBufferData.m_Swell, 0.0f, 1.0f);
    ImGui::SliderFloat("Ocean Depth (m)", &m_OceanSimulationSettingsBufferData.m_OceanDepth, 0.0f, 10000.0f);
    ImGui::SliderFloat("Density of Water (kg/m^3)", &m_OceanSimulationSettingsBufferData.m_DensityOfWater, 500.0f, 2000.0f);
    ImGui::SliderFloat("Surface Tension (N/m)", &m_OceanSimulationSettingsBufferData.m_SurfaceTension, 0.0f, 0.2f);
    ImGui::SliderFloat("Gravitational Constant (m/s^2)", &m_OceanSimulationSettingsBufferData.m_GravitationalConstant, 0.0f, 20.0f);

    if (ImGui::Button("Apply Changes"))
    {
        GenerateInitialSpectrum(false);
    }

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