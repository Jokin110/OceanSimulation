#pragma once

#include <d3d11.h>
#include <string>

using namespace std;

struct OceanSimulationSettings
{
	int m_OceanTextureSize = 256; // Size of the ocean texture (e.g., 256x256)
	float m_PatchSize = 1280.0f; // Size of the ocean patch in world units

	float m_DensityOfWater = 1000.0f; // kg/m^3
	float m_SurfaceTension = 0.074f; // N/m
	float m_GravitationalConstant = 9.81f; // m/s^2
	float m_OceanDepth = 4000.0f; // meters
	float m_WindDirection = 0.0f; // Angle in degrees (0 = east, 90 = north, 180 = west, 270 = south)
	float m_AverageWindSpeed = 10.0f; // m/s
	float m_FetchLength = 1000.0f; // meters
	float m_PeakEnhancementFactor = 3.3f; // Dimensionless factor to enhance the peak of the spectrum
	float m_Swell = 0.0f; // Dimensionless factor to add swell to the spectrum

	float m_PeakFrequency = 0.2f; // Peak frequency of the spectrum in Hz
	float m_Alpha = 0.0081f; // Phillips spectrum alpha parameter
	float m_WindAngle = 0.0f; // Wind direction in radians

	int m_RandomSeed = 0; // Random seed to generate the random gaussian numbers
	float m_Time = 0.0f; // Simulation time in seconds
};

struct TimeEvolutionData
{
	int m_OceanTextureSize; // Size of the ocean texture (e.g., 256x256)
	float m_PatchSize; // Size of the ocean patch in world units
	float m_DensityOfWater; // kg/m^3
	float m_SurfaceTension; // N/m
	float m_GravitationalConstant; // m/s^2
	float m_OceanDepth; // meters
	float m_Time; // Simulation time in seconds
	float m_Padding; // Padding to ensure 16-byte alignment
};

class OceanComputeManager
{
public:
    OceanComputeManager();
    ~OceanComputeManager();

    static OceanComputeManager& GetInstance()
    {
        return *m_Instance;
    }

    static bool Initialize();
    void Start();
    void Update();

	int GetOceanTextureSize() const { return m_OceanTextureSize; }
	float GetOceanPatchSize() const { return m_OceanPatchSize; }

    ID3D11ShaderResourceView* GetInitialSpectrumSRV() const { return m_InitialSpectrumSRV; }
	ID3D11ShaderResourceView* GetDisplacementSRV() const { return m_DisplacementSRV; }
	ID3D11ShaderResourceView* GetSlopeSRV() const { return m_SlopeSRV; }

private:
	void InitializeOceanSimulationSettings(bool initial);

	void GenerateInitialSpectrum(bool initial);
	void UpdateTimeEvolutionTextures();
	void UpdateFFTTextures();
	void GenerateDisplacementAndSlopeFinalTextures();
	void UpdateUI();

	bool CreateTextureAndViews();
	bool CreateComputeShaders();

    static OceanComputeManager* m_Instance;

	UINT m_OceanTextureSize = 1024; // Example grid size for the ocean surface
	float m_OceanPatchSize = 100.0f; // Size of the ocean patch in world units

	float m_DensityOfWater = 1000.0f; // kg/m^3
	float m_SurfaceTension = 0.074f; // N/m
	float m_GravitationalConstant = 9.81f; // m/s^2
	float m_OceanDepth = 4000.0f; // meters
	float m_WindDirection = 0.0f; // Angle in degrees (0 = east, 90 = north, 180 = west, 270 = south)
	float m_AverageWindSpeed = 10.0f; // m/s
	float m_FetchLength = 1000.0f; // meters
	float m_PeakEnhancementFactor = 3.3f; // Dimensionless factor to enhance the peak of the spectrum
	float m_Swell = 0.0f; // Dimensionless factor to add swell to the spectrum

	// Compute shader file paths
	wstring m_InitialSpectrumComputeShaderFile = L"assets/shaders/InitialSpectrumCS.hlsl";
	wstring m_TimeEvolutionComputeShaderFile = L"assets/shaders/TimeEvolutionCS.hlsl";
	wstring m_DisplacementAndSlopeComputeShaderFile = L"assets/shaders/DisplacementAndSlopeCS.hlsl";

	// Initial spectrum resources
	ID3D11Texture2D* m_InitialSpectrumTexture = nullptr;
    ID3D11UnorderedAccessView* m_InitialSpectrumUAV = nullptr;
    ID3D11ShaderResourceView* m_InitialSpectrumSRV = nullptr;

	// Displacement and slope calculations resources
	ID3D11Texture2D* m_XYDisplacementTexture = nullptr;
	ID3D11UnorderedAccessView* m_XYDisplacementUAV = nullptr;
	ID3D11ShaderResourceView* m_XYDisplacementSRV = nullptr;
	ID3D11Texture2D* m_XYDisplacementPingPongTexture = nullptr;
	ID3D11UnorderedAccessView* m_XYDisplacementPingPongUAV = nullptr;

	ID3D11Texture2D* m_ZDisplacementXXDerivativeTexture = nullptr;
	ID3D11UnorderedAccessView* m_ZDisplacementXXDerivativeUAV = nullptr;
	ID3D11ShaderResourceView* m_ZDisplacementXXDerivativeSRV = nullptr;
	ID3D11Texture2D* m_ZDisplacementXXDerivativePingPongTexture = nullptr;
	ID3D11UnorderedAccessView* m_ZDisplacementXXDerivativePingPongUAV = nullptr;

	ID3D11Texture2D* m_XZYXDerivativeTexture = nullptr;
	ID3D11UnorderedAccessView* m_XZYXDerivativeUAV = nullptr;
	ID3D11ShaderResourceView* m_XZYXDerivativeSRV = nullptr;
	ID3D11Texture2D* m_XZYXDerivativePingPongTexture = nullptr;
	ID3D11UnorderedAccessView* m_XZYXDerivativePingPongUAV = nullptr;

	ID3D11Texture2D* m_YZZZDerivativeTexture = nullptr;
	ID3D11UnorderedAccessView* m_YZZZDerivativeUAV = nullptr;
	ID3D11ShaderResourceView* m_YZZZDerivativeSRV = nullptr;
	ID3D11Texture2D* m_YZZZDerivativePingPongTexture = nullptr;
	ID3D11UnorderedAccessView* m_YZZZDerivativePingPongUAV = nullptr;

	// Displacement and slope resources
	ID3D11Texture2D* m_DisplacementTexture = nullptr;
	ID3D11UnorderedAccessView* m_DisplacementUAV = nullptr;
	ID3D11ShaderResourceView* m_DisplacementSRV = nullptr;

	ID3D11Texture2D* m_SlopeTexture = nullptr;
	ID3D11UnorderedAccessView* m_SlopeUAV = nullptr;
	ID3D11ShaderResourceView* m_SlopeSRV = nullptr;

	// Compute shaders pointers
    ID3D11ComputeShader* m_InitialSpectrumComputeShader = nullptr;
	ID3D11ComputeShader* m_TimeEvolutionComputeShader = nullptr;
	ID3D11ComputeShader* m_DisplacementAndSlopeComputeShader = nullptr;

	// Buffer to hold ocean simulation settings
	OceanSimulationSettings m_OceanSimulationSettingsBufferData;
	ID3D11Buffer* m_d3dOceanSimulationSettingsBuffer = nullptr;
	TimeEvolutionData m_TimeEvolutionBufferData;
	ID3D11Buffer* m_d3dTimeEvolutionBuffer = nullptr;
};

