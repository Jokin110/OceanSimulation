
#define PI 3.14159265359

RWTexture2D<float4> InitialSpectrumTexture : register(u0);

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

uint PCGHash(uint state)
{
    // Hash function based on PCG algorithm to generate pseudo-random numbers
    state = state * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

float RandomFloat(inout uint state)
{
    // Generate a random float in the range [0, 1) using the PCG hash function
    state = PCGHash(state);
    
    return (float) state / 4294967295.0f;
}

float2 GenerateRandomGaussianNumbers(uint x, uint y)
{
    // Use the Box-Muller transform to generate two independent standard normally distributed random numbers
    uint state = x + y * m_OceanTextureSize + (uint)m_RandomSeed; // Create a unique state for each pixel
    
    float u1 = max(0.0001f, RandomFloat(state));
    float u2 = RandomFloat(state);
    
    float r = sqrt(-2.0 * log(u1));
    
    float z0 = r * cos(2.0 * PI * u2);
    float z1 = r * sin(2.0 * PI * u2);
    
    return float2(z0, z1);
}

float DispersionRelation(float kLength)
{
    // The dispersion relation for water waves, accounting for both gravity and surface tension effects as well as finite depth
    return sqrt((m_GravitationalConstant * kLength + m_SurfaceTension / m_DensityOfWater * pow(kLength, 3)) * tanh(min(kLength * m_OceanDepth, 20.0f)));
}

float DispersionRelationDerivative(float kLength, float omega)
{
    // Compute the derivative of the dispersion relation with respect to the wave number k, which is needed for the energy spectrum calculation
    float tanH = tanh(min(kLength * m_OceanDepth, 20.0f));
    float dOmega_dk = (m_GravitationalConstant + 3.0f * m_SurfaceTension / m_DensityOfWater * pow(kLength, 2)) * tanH + (m_GravitationalConstant * kLength + m_SurfaceTension / m_DensityOfWater * pow(kLength, 3)) * m_OceanDepth * (1 - tanH * tanH);
    
    return dOmega_dk / (2.0f * omega);
}

float JONSWAPSpectra(float omega)
{
    // The JONSWAP spectrum is a modification of the Pierson-Moskowitz spectrum that includes a peak enhancement factor to better match observed ocean wave spectra, especially in fetch-limited conditions. 
    // The sigma parameter controls the width of the peak and is typically smaller for frequencies below the peak frequency and larger for frequencies above it.
    float sigma = (omega <= m_PeakFrequency) ? 0.07f : 0.09f;
    float r = exp(-pow((omega - m_PeakFrequency) / (sigma * m_PeakFrequency), 2) / 2.0f);
    
    return m_Alpha * m_GravitationalConstant * m_GravitationalConstant / pow(omega, 5) * exp(-1.25f * pow(m_PeakFrequency / omega, 4)) * pow(m_PeakEnhancementFactor, r);
}

float TMACorrection(float omega)
{
    // The TMA correction accounts for the effects of finite water depth on the wave spectrum, which is important for accurately modeling coastal and shallow water waves. 
    // It modifies the energy distribution of the waves based on the ratio of the wave frequency to a characteristic frequency determined by the water depth and gravitational constant.
    float omegaH = omega * sqrt(m_OceanDepth / m_GravitationalConstant);
    
    if (omegaH <= 1.0f) // Very shallow water waves
    {
        return 0.5f * omegaH * omegaH;
    }
    else if (omegaH <= 2.0f) // Intermediate depth waves
    {
        return 1.0f - 0.5f * pow(2.0f - omegaH, 2.0f);
    }
    else // Deep water waves
    {
        return 1.0f;
    }
}

float DirectionalSpreadingFunction(float omega, float waveAngle)
{
    // The directional spreading function models how the energy of the waves is distributed across different directions, which is crucial for creating realistic wave patterns. 
    // It depends on the frequency of the wave and the angle between the wave direction and the wind direction. 
    // The function typically has a peak in the direction of the wind and decreases as the angle increases, with a shape that can be adjusted based on empirical observations.
    
    float omegaOverPeakOmega = omega / m_PeakFrequency;
    
    float beta = 0.0f;
    
    if (omegaOverPeakOmega < 0.95f)
    {
        beta = 2.61f * pow(omegaOverPeakOmega, 1.3f);
    }
    else if (omegaOverPeakOmega < 1.6f)
    {
        beta = 2.28f * pow(omegaOverPeakOmega, -1.3f);
    }
    else
    {
        float epsylon = -0.4f + 0.8393f * exp(-0.567f * log(omegaOverPeakOmega * omegaOverPeakOmega));
        
        beta = pow(10.0f, epsylon);
    }
    
    beta = max(beta, 0.0001f); // Ensure beta is not too small to avoid numerical issues
    
    float rawDelta = waveAngle - m_WindAngle;
    float shortestAngleDiff = atan2(sin(rawDelta), cos(rawDelta));
    
    return beta / (2.0f * tanh(min(beta * PI, 20.0f))) / pow(cosh(min(beta * shortestAngleDiff, 20.0f)), 2);
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
        InitialSpectrumTexture[uint2(x, y)] = float4(0.0f, 0.0f, 0.0f, 0.0f);
        return;
    }
    
    float waveAngle = atan2(ky, kx);
    
    float omega = DispersionRelation(kLength);
    
    float energyValue = JONSWAPSpectra(omega) * TMACorrection(omega) * DirectionalSpreadingFunction(omega, waveAngle) * DispersionRelationDerivative(kLength, omega) / kLength;
    
    float2 randomNumber = GenerateRandomGaussianNumbers(x, y);
    
    float2 h0 = sqrt(max(energyValue, 0.0f) / 2.0f) * randomNumber;
    
    // Calculate the conjugate index for the negative wave vector, which is needed to ensure that the resulting wave field is real-valued when transformed back to the spatial domain.
    uint oppX = (m_OceanTextureSize - x) % m_OceanTextureSize;
    uint oppY = (m_OceanTextureSize - y) % m_OceanTextureSize;
    
    float2 oppRandomNumber = GenerateRandomGaussianNumbers(oppX, oppY);
    
    float oppWaveAngle = waveAngle + PI;
    float oppEnergyValue = JONSWAPSpectra(omega) * TMACorrection(omega) * DirectionalSpreadingFunction(omega, oppWaveAngle) * DispersionRelationDerivative(kLength, omega) / kLength;
                           
    float2 h0_opp = sqrt(max(oppEnergyValue, 0.0f) / 2.0f) * oppRandomNumber;
    
    h0_opp.y = -h0_opp.y;
    
    InitialSpectrumTexture[uint2(x, y)] = float4(h0, h0_opp);
}