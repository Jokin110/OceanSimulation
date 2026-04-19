#pragma once

#include <d3d11.h>
#include <string>
#include <DirectXMath.h>

using namespace std;
using namespace DirectX;

const int CASCADE_COUNT = 4;

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

	float m_LowPassFilter = 0.0f; // Simulation time in seconds
	float m_HighPassFilter = 0.0f; // Simulation time in seconds

	float m_CascadeAmplitude = 1.0f;
	XMFLOAT2 m_Padding = XMFLOAT2(0.0f, 0.0f); // Padding to ensure 16-byte alignment
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

struct DisplacementAndSlopeData
{
	float m_FoamBias;
	float m_DecayFactor;
	float m_DeltaTime;
	float m_FoamAddition;
	float m_ChoppinessFactor;
	XMFLOAT3 m_Padding;
};

struct TessellationSettingsData
{
	float m_MinTessellationDistance = 50.0f;
	float m_MaxTessellationDistance = 1200.0f;
	int m_TessellationExponent = 16;
};

struct OceanSimulationCascadeSettings
{
	UINT m_OceanTextureSize = 256; // Example grid size for the ocean surface
	int m_SelectedTextureSizeIndex = 2;
	float m_OceanPatchSize[CASCADE_COUNT] = { 2003.0f, 509.0f, 101.0f, 23.0f }; // Size of the ocean patch in world units
	float m_MeshVertexSeparation = 10.0f;
	float m_FrequencyFilterMultiplier = 0.5f;
	float m_CascadeAmplitudes[CASCADE_COUNT] = { 1.0f, 2.5f, 8.0f, 25.0f }; // Amplitude scaling for each cascade
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

	int GetOceanTextureSize() const { return m_OceanSimulationCascadeSettings.m_OceanTextureSize; }
	const float* GetOceanPatchSize() const { return m_OceanSimulationCascadeSettings.m_OceanPatchSize; } 
	float GetMeshVertexSeparation() const { return m_OceanSimulationCascadeSettings.m_MeshVertexSeparation; }

    ID3D11ShaderResourceView* const* GetInitialSpectrumSRV() const { return m_InitialSpectrumSRV; }
	ID3D11ShaderResourceView* const* GetDisplacementSRV() const { return m_DisplacementSRV; }
	ID3D11ShaderResourceView* const* GetSlopeSRV() const { return m_SlopeSRV; }

	TessellationSettingsData GetTessellationSettingsData() { return m_TessellationSettingsData; }

private:
	void InitializeOceanSimulationSettings(bool initial);

	void GenerateInitialSpectrum(bool initial);
	void UpdateTimeEvolutionTextures();
	void UpdateFFTTextures();
	void GenerateDisplacementAndSlopeFinalTextures();

	void UpdateUI();
	void UpdateSimulationSettingsUI();
	void UpdateGraphicSettingsUI();
	void UpdateCascadeSettingsUI();

	bool CreateTextureAndViews();
	bool CreateComputeShaders();

	bool ReinitializeTexturesMeshesAndSpectrum();
	void RecalculateFrequencyFilters();

	void ApplyCascadeSettings(bool apply);

    static OceanComputeManager* m_Instance;

	OceanSimulationCascadeSettings m_OceanSimulationCascadeSettings;
	OceanSimulationCascadeSettings m_TemporaryOceanSimulationCascadeSettings;

	const int m_TextureSizes[6] = { 64, 128, 256, 512, 1024, 2048 };
	const char* m_TextureDropdownNames[6] = { "64", "128", "256", "512", "1024", "2048" };

	float m_LowPassFilters[CASCADE_COUNT]; // Frequency limits for each cascade (in Hz)
	float m_HighPassFilters[CASCADE_COUNT]; // High-pass filters for each cascade (in Hz)

	float m_DensityOfWater = 1000.0f; // kg/m^3
	float m_SurfaceTension = 0.074f; // N/m
	float m_GravitationalConstant = 9.81f; // m/s^2
	float m_OceanDepth = 4000.0f; // meters
	float m_WindDirection = 0.0f; // Angle in degrees (0 = east, 90 = north, 180 = west, 270 = south)
	float m_AverageWindSpeed = 10.0f; // m/s
	float m_FetchLength = 1000.0f; // meters
	float m_PeakEnhancementFactor = 3.3f; // Dimensionless factor to enhance the peak of the spectrum
	float m_Swell = 0.0f; // Dimensionless factor to add swell to the spectrum

	TessellationSettingsData m_TessellationSettingsData;

	// Compute shader file paths
	wstring m_InitialSpectrumComputeShaderFile = L"assets/shaders/InitialSpectrumCS.hlsl";
	wstring m_TimeEvolutionComputeShaderFile = L"assets/shaders/TimeEvolutionCS.hlsl";
	wstring m_DisplacementAndSlopeComputeShaderFile = L"assets/shaders/DisplacementAndSlopeCS.hlsl";

	// Initial spectrum resources
	ID3D11Texture2D* m_InitialSpectrumTexture[CASCADE_COUNT] = { nullptr };
    ID3D11UnorderedAccessView* m_InitialSpectrumUAV[CASCADE_COUNT] = { nullptr };
    ID3D11ShaderResourceView* m_InitialSpectrumSRV[CASCADE_COUNT] = { nullptr };

	// Displacement and slope calculations resources
	ID3D11Texture2D* m_XYDisplacementTexture[CASCADE_COUNT] = { nullptr };
	ID3D11UnorderedAccessView* m_XYDisplacementUAV[CASCADE_COUNT] = { nullptr };
	ID3D11ShaderResourceView* m_XYDisplacementSRV[CASCADE_COUNT] = { nullptr };
	ID3D11Texture2D* m_XYDisplacementPingPongTexture[CASCADE_COUNT] = { nullptr };
	ID3D11UnorderedAccessView* m_XYDisplacementPingPongUAV[CASCADE_COUNT] = { nullptr };

	ID3D11Texture2D* m_ZDisplacementXXDerivativeTexture[CASCADE_COUNT] = { nullptr };
	ID3D11UnorderedAccessView* m_ZDisplacementXXDerivativeUAV[CASCADE_COUNT] = { nullptr };
	ID3D11ShaderResourceView* m_ZDisplacementXXDerivativeSRV[CASCADE_COUNT] = { nullptr };
	ID3D11Texture2D* m_ZDisplacementXXDerivativePingPongTexture[CASCADE_COUNT] = { nullptr };
	ID3D11UnorderedAccessView* m_ZDisplacementXXDerivativePingPongUAV[CASCADE_COUNT] = { nullptr };

	ID3D11Texture2D* m_XZYXDerivativeTexture[CASCADE_COUNT] = { nullptr };
	ID3D11UnorderedAccessView* m_XZYXDerivativeUAV[CASCADE_COUNT] = { nullptr };
	ID3D11ShaderResourceView* m_XZYXDerivativeSRV[CASCADE_COUNT] = { nullptr };
	ID3D11Texture2D* m_XZYXDerivativePingPongTexture[CASCADE_COUNT] = { nullptr };
	ID3D11UnorderedAccessView* m_XZYXDerivativePingPongUAV[CASCADE_COUNT] = { nullptr };

	ID3D11Texture2D* m_YZZZDerivativeTexture[CASCADE_COUNT] = { nullptr };
	ID3D11UnorderedAccessView* m_YZZZDerivativeUAV[CASCADE_COUNT] = { nullptr };
	ID3D11ShaderResourceView* m_YZZZDerivativeSRV[CASCADE_COUNT] = { nullptr };
	ID3D11Texture2D* m_YZZZDerivativePingPongTexture[CASCADE_COUNT] = { nullptr };
	ID3D11UnorderedAccessView* m_YZZZDerivativePingPongUAV[CASCADE_COUNT] = { nullptr };

	// Displacement and slope resources
	ID3D11Texture2D* m_DisplacementTexture[CASCADE_COUNT] = { nullptr };
	ID3D11UnorderedAccessView* m_DisplacementUAV[CASCADE_COUNT] = { nullptr };
	ID3D11ShaderResourceView* m_DisplacementSRV[CASCADE_COUNT] = { nullptr };

	ID3D11Texture2D* m_SlopeTexture[CASCADE_COUNT] = { nullptr };
	ID3D11UnorderedAccessView* m_SlopeUAV[CASCADE_COUNT] = { nullptr };
	ID3D11ShaderResourceView* m_SlopeSRV[CASCADE_COUNT] = { nullptr };

	// Compute shaders pointers
    ID3D11ComputeShader* m_InitialSpectrumComputeShader = nullptr;
	ID3D11ComputeShader* m_TimeEvolutionComputeShader = nullptr;
	ID3D11ComputeShader* m_DisplacementAndSlopeComputeShader = nullptr;

	// Buffer to hold ocean simulation settings
	OceanSimulationSettings m_OceanSimulationSettingsBufferData;
	ID3D11Buffer* m_d3dOceanSimulationSettingsBuffer = nullptr;
	TimeEvolutionData m_TimeEvolutionBufferData;
	ID3D11Buffer* m_d3dTimeEvolutionBuffer = nullptr;
	DisplacementAndSlopeData m_DisplacementAndSlopeBufferData;
	ID3D11Buffer* m_d3dDisplacementAndSlopeBuffer = nullptr;
};

