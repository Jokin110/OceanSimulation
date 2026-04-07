
#define PI 3.14159265359

//Input: The initial spectrum created at start up
Texture2D<float4> InitialSpectrumTexture : register(t0);

// Displacement and slope output textures
RWTexture2D<float4> DisplacementTexture : register(u0);

cbuffer OceanSimulationSettings : register(b0)
{
    int m_OceanTextureSize; // Size of the ocean texture (e.g., 256x256)
    float m_PatchSize; // Size of the ocean patch in world units
    float m_DensityOfWater; // kg/m^3
    float m_SurfaceTension; // N/m
    float m_GravitationalConstant; // m/s^2
    float m_OceanDepth; // meters
    float m_WindDirection; // Angle in degrees (0 = east, 90 = north, 180 = west, 270 = south)
    float m_AverageWindSpeed; // m/s
    float m_FetchLength; // meters
    float m_PeakEnhancementFactor; // Dimensionless factor to enhance the peak of the spectrum
    float m_Swell; // Dimensionless factor to add swell to the spectrum

    float m_PeakFrequency; // Peak frequency of the spectrum in Hz
    float m_Alpha; // Phillips spectrum alpha parameter
    float m_WindAngle; // Wind direction in radians
    
    int m_RandomSeed; // Random seed to generate the random gaussian numbers
    float m_Time; // Simulation time in seconds
}

float DispersionRelation(float kLength)
{
    // The dispersion relation for water waves, accounting for both gravity and surface tension effects as well as finite depth
    return sqrt((m_GravitationalConstant * kLength + m_SurfaceTension / m_DensityOfWater * pow(kLength, 3)) * tanh(min(kLength * m_OceanDepth, 20.0f)));
}

float2 CalculateVerticalDisplacement(float omega, float4 h0)
{
    float omegaT = omega * m_Time;
    
    float cosOmegaT = cos(omegaT);
    float sinOmegaT = sin(omegaT);
    
    float h0KRealPart = h0.r * cosOmegaT - h0.g * sinOmegaT;
    float h0KImagPart = h0.r * sinOmegaT + h0.g * cosOmegaT;
    
    float h0MinusKConjRealPart = h0.b * cosOmegaT + h0.a * sinOmegaT;
    float h0MinusKConjImagPart = -h0.b * sinOmegaT + h0.a * cosOmegaT;
    
    return float2(h0KRealPart + h0MinusKConjRealPart, h0KImagPart + h0MinusKConjImagPart);

}

[numthreads(16, 16, 1)]
void Main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    uint x = dispatchThreadID.x;
    uint y = dispatchThreadID.y;
    
    // Convert the thread ID to a wave vector (kx, ky) in the frequency domain. The wave vector components are centered around zero, so we need to shift them accordingly.
    float nx = x - m_OceanTextureSize / 2.0f; // (x < m_OceanTextureSize / 2.0f) ? x : (float) x - m_OceanTextureSize;
    float ny = y - m_OceanTextureSize / 2.0f; //(y < m_OceanTextureSize / 2.0f) ? y : (float) y - m_OceanTextureSize;

    float kx = nx * (2.0f * PI / m_PatchSize);
    float ky = ny * (2.0f * PI / m_PatchSize);
    
    float kLength = sqrt(kx * kx + ky * ky);
    
    if (kLength < 0.00001f)
    {
        DisplacementTexture[uint2(x, y)] = float4(0, 0, 0, 0);
        return;
    }
    
    float4 h0 = InitialSpectrumTexture[uint2(x, y)];
    
    float omega = DispersionRelation(kLength);
    
    float2 hKT = CalculateVerticalDisplacement(omega, h0);
    
    float2 ihKT = float2(-hKT.g, hKT.r); // i * h(k, t) = (-imaginary, real)
    
    float2 displacementX = ihKT * kx / kLength;
    float2 displacementY = hKT;
    float2 displacementZ = ihKT * ky /kLength;
    
    DisplacementTexture[uint2(x, y)] = float4(displacementX, displacementY); // Placeholder: Replace with actual time evolution logic
}