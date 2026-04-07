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

    ID3D11ShaderResourceView* GetInitialSpectrumSRV() const { return m_InitialSpectrumSRV; }
	ID3D11ShaderResourceView* GetXYDisplacementSRV() const { return m_XYDisplacementSRV; }
	ID3D11ShaderResourceView* GetZDisplacementXXDerivativeSRV() const { return m_ZDisplacementXXDerivativeSRV; }
	ID3D11ShaderResourceView* GetXZYXDerivativeSRV() const { return m_XZYXDerivativeSRV; }
	ID3D11ShaderResourceView* GetYZZZDerivativeSRV() const { return m_YZZZDerivativeSRV; }

private:
	void InitializeOceanSimulationSettings(bool initial);

	void GenerateInitialSpectrum(bool initial);
	void UpdateTimeEvolutionTextures();
	void UpdateFFTTextures();
	void UpdateUI();

	bool CreateTextureAndViews();
	bool CreateComputeShaders();

    static OceanComputeManager* m_Instance;

	UINT m_OceanTextureSize = 256; // Example grid size for the ocean surface
	UINT m_OceanPatchSize = 256; // Size of the ocean patch in world units

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
	wstring m_FFTComputeShaderFile = L"assets/shaders/FFTOceanSurfaceCS.hlsl";

	// Initial spectrum resources
	ID3D11Texture2D* m_InitialSpectrumTexture = nullptr;
    ID3D11UnorderedAccessView* m_InitialSpectrumUAV = nullptr;
    ID3D11ShaderResourceView* m_InitialSpectrumSRV = nullptr;

	// Displacement and slope resources
	ID3D11Texture2D* m_XYDisplacementTexture = nullptr;
	ID3D11UnorderedAccessView* m_XYDisplacementUAV = nullptr;
	ID3D11ShaderResourceView* m_XYDisplacementSRV = nullptr;

	ID3D11Texture2D* m_ZDisplacementXXDerivativeTexture = nullptr;
	ID3D11UnorderedAccessView* m_ZDisplacementXXDerivativeUAV = nullptr;
	ID3D11ShaderResourceView* m_ZDisplacementXXDerivativeSRV = nullptr;

	ID3D11Texture2D* m_XZYXDerivativeTexture = nullptr;
	ID3D11UnorderedAccessView* m_XZYXDerivativeUAV = nullptr;
	ID3D11ShaderResourceView* m_XZYXDerivativeSRV = nullptr;

	ID3D11Texture2D* m_YZZZDerivativeTexture = nullptr;
	ID3D11UnorderedAccessView* m_YZZZDerivativeUAV = nullptr;
	ID3D11ShaderResourceView* m_YZZZDerivativeSRV = nullptr;

	// Compute shaders pointers
    ID3D11ComputeShader* m_InitialSpectrumComputeShader = nullptr;
	ID3D11ComputeShader* m_TimeEvolutionComputeShader = nullptr;
    ID3D11ComputeShader* m_FFTComputeShader = nullptr;

	// Buffer to hold ocean simulation settings
	OceanSimulationSettings m_OceanSimulationSettingsBufferData;
	ID3D11Buffer* m_d3dOceanSimulationSettingsBuffer = nullptr;
	TimeEvolutionData m_TimeEvolutionBufferData;
	ID3D11Buffer* m_d3dTimeEvolutionBuffer = nullptr;
};

