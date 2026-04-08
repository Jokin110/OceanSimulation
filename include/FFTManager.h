#pragma once

#include <d3d11.h>
#include <string>

using namespace std;

struct FFTParams
{
    int Size;
    int Step;
    int PingPong; // 0 for false, 1 for true
    int Padding;
};

class FFTManager
{
public:
    FFTManager();
    ~FFTManager();

    static FFTManager& GetInstance()
    {
        return *m_Instance;
    }

    // Pass the ocean texture size here so we can create the precomputed data texture
    static bool Initialize(int size);

    // The main functions your OceanComputeManager will call
    void PrecomputeTwiddleFactors();
    void ComputeIFFT2D(ID3D11UnorderedAccessView* inputUAV, ID3D11UnorderedAccessView* pingPongUAV, bool outputToInput, bool scale, bool permute);

private:
    bool CreateResources();
    bool CreateComputeShaders();

    static FFTManager* m_Instance;

    int m_Size;

    // Buffer to hold FFT parameters
    FFTParams m_FFTParamsData;
    ID3D11Buffer* m_d3dFFTParamsBuffer = nullptr;

    // Precomputed Twiddle Factors and Indices
    ID3D11Texture2D* m_PrecomputedDataTexture = nullptr;
    ID3D11UnorderedAccessView* m_PrecomputedDataUAV = nullptr;
    ID3D11ShaderResourceView* m_PrecomputedDataSRV = nullptr;

    wstring m_FFTComputeShaderFile = L"assets/shaders/FFTCS.hlsl";

    // Individual Compute Shader Kernels
    ID3D11ComputeShader* m_PrecomputeCS = nullptr;
    ID3D11ComputeShader* m_HorizontalStepFFTCS = nullptr;
    ID3D11ComputeShader* m_VerticalStepFFTCS = nullptr;
    ID3D11ComputeShader* m_HorizontalStepIFFTCS = nullptr;
    ID3D11ComputeShader* m_VerticalStepIFFTCS = nullptr;
    ID3D11ComputeShader* m_ScaleCS = nullptr;
    ID3D11ComputeShader* m_PermuteCS = nullptr;
};