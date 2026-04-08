#include "FFTManager.h"
#include "D3D11Application.h"
#include <cmath>
#include <iostream>

FFTManager* FFTManager::m_Instance = nullptr;

FFTManager::FFTManager()
{
    m_Size = 256; // Default
    m_FFTParamsData = {};
}

FFTManager::~FFTManager()
{
    if (m_d3dFFTParamsBuffer) m_d3dFFTParamsBuffer->Release();

    if (m_PrecomputedDataTexture) m_PrecomputedDataTexture->Release();
    if (m_PrecomputedDataUAV) m_PrecomputedDataUAV->Release();
    if (m_PrecomputedDataSRV) m_PrecomputedDataSRV->Release();

    if (m_PrecomputeCS) m_PrecomputeCS->Release();
    if (m_HorizontalStepFFTCS) m_HorizontalStepFFTCS->Release();
    if (m_VerticalStepFFTCS) m_VerticalStepFFTCS->Release();
    if (m_HorizontalStepIFFTCS) m_HorizontalStepIFFTCS->Release();
    if (m_VerticalStepIFFTCS) m_VerticalStepIFFTCS->Release();
    if (m_ScaleCS) m_ScaleCS->Release();
    if (m_PermuteCS) m_PermuteCS->Release();

    if (m_Instance)
    {
        delete m_Instance;
        m_Instance = nullptr;
    }
}

bool FFTManager::Initialize(int size)
{
    if (!m_Instance)
    {
        m_Instance = new FFTManager;
        m_Instance->m_Size = size;

        if (!m_Instance->CreateResources()) return false;
        if (!m_Instance->CreateComputeShaders()) return false;

        return true;
    }
    return false;
}

bool FFTManager::CreateResources()
{
    ID3D11Device* device = D3D11Application::GetInstance().GetDevice();

    // 1. Create Constant Buffer
    D3D11_BUFFER_DESC cbDesc = {};
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.ByteWidth = sizeof(FFTParams);
    cbDesc.Usage = D3D11_USAGE_DEFAULT;

    if (FAILED(device->CreateBuffer(&cbDesc, nullptr, &m_d3dFFTParamsBuffer)))
    {
        std::cout << "FFTManager: Failed to create constant buffer\n";
        return false;
    }

    // 2. Create Precomputed Data Texture (Width = log2(Size), Height = Size)
    int logSize = static_cast<int>(std::log2(m_Size));

    D3D11_TEXTURE2D_DESC texDesc = {};
    texDesc.Width = logSize;
    texDesc.Height = m_Size;
    texDesc.MipLevels = 1;
    texDesc.ArraySize = 1;
    texDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // ARGBFloat equivalent
    texDesc.SampleDesc.Count = 1;
    texDesc.Usage = D3D11_USAGE_DEFAULT;
    texDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;

    if (FAILED(device->CreateTexture2D(&texDesc, nullptr, &m_PrecomputedDataTexture))) return false;

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
    uavDesc.Format = texDesc.Format;
    uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
    if (FAILED(device->CreateUnorderedAccessView(m_PrecomputedDataTexture, &uavDesc, &m_PrecomputedDataUAV))) return false;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    if (FAILED(device->CreateShaderResourceView(m_PrecomputedDataTexture, &srvDesc, &m_PrecomputedDataSRV))) return false;

    return true;
}

bool FFTManager::CreateComputeShaders()
{
    D3D11Application& app = D3D11Application::GetInstance();

    wstring path = m_FFTComputeShaderFile;

#ifndef _DEBUG
    path = L"../../" + path;
#endif

    m_PrecomputeCS = app.CreateComputeShaderWithEntry(path, "PrecomputeTwiddleFactorsAndInputIndices");
    m_HorizontalStepFFTCS = app.CreateComputeShaderWithEntry(path, "HorizontalStepFFT");
    m_VerticalStepFFTCS = app.CreateComputeShaderWithEntry(path, "VerticalStepFFT");
    m_HorizontalStepIFFTCS = app.CreateComputeShaderWithEntry(path, "HorizontalStepInverseFFT");
    m_VerticalStepIFFTCS = app.CreateComputeShaderWithEntry(path, "VerticalStepInverseFFT");
    m_ScaleCS = app.CreateComputeShaderWithEntry(path, "Scale");
    m_PermuteCS = app.CreateComputeShaderWithEntry(path, "Permute");

    if (!m_PrecomputeCS || !m_HorizontalStepIFFTCS) return false;

    return true;
}

void FFTManager::PrecomputeTwiddleFactors()
{
    ID3D11DeviceContext* context = D3D11Application::GetInstance().GetDeviceContext();

    int logSize = static_cast<int>(std::log2(m_Size));

    m_FFTParamsData.Size = m_Size;
    context->UpdateSubresource(m_d3dFFTParamsBuffer, 0, nullptr, &m_FFTParamsData, 0, 0);

    context->CSSetShader(m_PrecomputeCS, nullptr, 0);
    context->CSSetConstantBuffers(0, 1, &m_d3dFFTParamsBuffer);
    context->CSSetUnorderedAccessViews(0, 1, &m_PrecomputedDataUAV, nullptr); // Map to register(u0)

    // Dispatch (logSize, Size / 2 / 8, 1)
    context->Dispatch(logSize, (m_Size / 2) / 8, 1);

    // Unbind UAV
    ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
    context->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
}

void FFTManager::ComputeIFFT2D(ID3D11UnorderedAccessView* inputUAV, ID3D11UnorderedAccessView* pingPongUAV, bool outputToInput, bool scale, bool permute)
{
    ID3D11DeviceContext* context = D3D11Application::GetInstance().GetDeviceContext();

    int logSize = static_cast<int>(std::log2(m_Size));
    bool pingPong = false;

    // Set shared resources (Twiddle factors SRV to t0)
    context->CSSetShaderResources(0, 1, &m_PrecomputedDataSRV);

    // Bind both UAVs simultaneously (u0 and u1 in HLSL)
    ID3D11UnorderedAccessView* uavs[2] = { inputUAV, pingPongUAV };
    context->CSSetUnorderedAccessViews(1, 2, uavs, nullptr);

    // --- HORIZONTAL PASS ---
    context->CSSetShader(m_HorizontalStepIFFTCS, nullptr, 0);
    for (int i = 0; i < logSize; i++)
    {
        pingPong = !pingPong;
        m_FFTParamsData.Step = i;
        m_FFTParamsData.PingPong = pingPong ? 1 : 0;
        m_FFTParamsData.Size = m_Size;
        context->UpdateSubresource(m_d3dFFTParamsBuffer, 0, nullptr, &m_FFTParamsData, 0, 0);
        context->CSSetConstantBuffers(0, 1, &m_d3dFFTParamsBuffer);

        context->Dispatch(m_Size / 8, m_Size / 8, 1);
    }

    // --- VERTICAL PASS ---
    context->CSSetShader(m_VerticalStepIFFTCS, nullptr, 0);
    for (int i = 0; i < logSize; i++)
    {
        pingPong = !pingPong;
        m_FFTParamsData.Step = i;
        m_FFTParamsData.PingPong = pingPong ? 1 : 0;
        context->UpdateSubresource(m_d3dFFTParamsBuffer, 0, nullptr, &m_FFTParamsData, 0, 0);

        context->Dispatch(m_Size / 8, m_Size / 8, 1);
    }

    // --- SCALE & PERMUTE PASSES ---
    if (permute)
    {
        context->CSSetShader(m_PermuteCS, nullptr, 0);
        context->Dispatch(m_Size / 8, m_Size / 8, 1);
    }

    if (scale)
    {
        context->CSSetShader(m_ScaleCS, nullptr, 0);
        context->Dispatch(m_Size / 8, m_Size / 8, 1);
    }

    // --- CLEANUP & OPTIONAL BLIT ---
    ID3D11UnorderedAccessView* nullUAVs[2] = { nullptr, nullptr };
    context->CSSetUnorderedAccessViews(1, 2, nullUAVs, nullptr);
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    context->CSSetShaderResources(0, 1, nullSRV);

    // If the final result ended up in the pingPong buffer, but the user requested it in the input buffer:
    if (pingPong && outputToInput)
    {
        // Extract the ID3D11Resource (Texture2D) from the UAVs and copy them
        ID3D11Resource* srcRes = nullptr;
        ID3D11Resource* destRes = nullptr;
        pingPongUAV->GetResource(&srcRes);
        inputUAV->GetResource(&destRes);

        context->CopyResource(destRes, srcRes);

        srcRes->Release();
        destRes->Release();
    }
}