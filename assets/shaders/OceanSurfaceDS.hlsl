#define PI 3.14159265359

struct DSInput
{
    float3 Position : POSITION;
    float3 Color : COLOR;
};

struct DSOutput
{
    float3 WorldPosition : TEXCOORD0;
    float2 UVs : TEXCOORD1;
    float3 ViewVector : TEXCOORD2;
    float3 EyePos : TEXCOORD3;
    float3 Color : COLOR;
    float4 Position : SV_POSITION;
};

struct PatchTess
{
    float EdgeTess[4] : SV_TessFactor;
    float InsideTess[2] : SV_InsideTessFactor;
};

cbuffer PerObjectBuffer : register(b0)
{
    row_major matrix m_WorldMatrix;
    row_major matrix m_InverseTransposeWorldMatrix;
    row_major matrix m_ViewProjectionMatrix;
    
    float m_Time;
    float3 m_CameraPosition;
    
    int m_OceanTextureSize; // Size of the ocean texture (e.g., 256x256)
    float m_PatchSize; // Size of the ocean patch in world units
    float2 m_Padding; // Padding to align to 16 bytes
}

Texture2D DisplacementTexture : register(t0);
SamplerState LinearSampler : register(s0);

struct WaveResults
{
    float3 m_FinalPos;
    float3 m_Normal;
    float m_Jacobian;
};

WaveResults CalculateWavePosition(float3 originalPos)
{
    WaveResults results = (WaveResults) 0;
    
    return results;
    
    results.m_FinalPos = originalPos;
    results.m_FinalPos.y = 0;
    
    float2 initialDirection = normalize(float2(1.0, 0.5));
    float initialLength = 100.0f;
    float initialSteepness = 0.35f;
    float initialPhase = PI / 2;
    
    float w0 = 2 * PI / 200;
    
    float3 tangent = float3(1, 0, 0);
    float3 binormal = float3(0, 0, 1);
    
    float lambda = 1.2;

    for (int i = 0; i < 64; i++)
    {
        float2 direction = normalize(float2(cos(PI / 4.0 * (i + 1) - PI / 3.0), sin(PI / 4.0 * (i + 1) - PI / 3.0)));
        float length = initialLength * pow(0.6f, i / 1.0f);
        float steepness = initialSteepness * pow(0.7f, i / 2.0f);
        float phase = initialPhase * i;
        
        float k = 2 * PI / length;
        float w = sqrt(9.8 * k);
        w = floor(w / w0) * w0;
        float a = steepness / k;
        
        float2 wavevector = direction * k;
        
        float alpha = dot(wavevector, originalPos.xz) - w * m_Time + phase;
        float sinalpha, cosalpha;
        sincos(alpha, sinalpha, cosalpha);
        
        results.m_FinalPos.xz -= lambda * direction * a * sinalpha;
        results.m_FinalPos.y += a * cos(alpha);
        
        tangent -= float3(lambda * wavevector.x * wavevector.x / k * a * cosalpha, wavevector.x * a * sinalpha, lambda * wavevector.x * wavevector.y / k * a * cosalpha);
        binormal -= float3(lambda * wavevector.x * wavevector.y / k * a * cosalpha, wavevector.y * a * sinalpha, lambda * wavevector.y * wavevector.y / k * a * cosalpha);
    }
    
    float Jxx = tangent.x;
    float Jyy = binormal.z;
    float Jyx = tangent.z;
    float Jxy = binormal.x;
    
    results.m_Jacobian = Jxx * Jyy - Jxy * Jyx;
    
    results.m_Normal = normalize(cross(binormal, tangent));
    
    return results;
}

[domain("quad")]
DSOutput Main(PatchTess patchTess, float2 uv : SV_DomainLocation, const OutputPatch<DSInput, 4> quad)
{
    DSOutput output = (DSOutput) 0;

    // 1. Bilinear Interpolation using the generated UVs    
    float3 v1 = lerp(quad[0].Position, quad[2].Position, uv.x);
    float3 v2 = lerp(quad[1].Position, quad[3].Position, uv.x);
    float3 localPos = lerp(v1, v2, 1 - uv.y);

    float3 c1 = lerp(quad[0].Color, quad[2].Color, uv.x);
    float3 c2 = lerp(quad[1].Color, quad[3].Color, uv.x);
    output.Color = lerp(c1, c2, 1 - uv.y);
    
    float3 flatWorldPos = mul(float4(localPos, 1.0), m_WorldMatrix).xyz;
    
    output.UVs = (flatWorldPos.xz / m_OceanTextureSize + 0.5f) % 1.0f;
    
    // Sample the FFT texture you generated in the Compute Shader
    float4 displacementData = DisplacementTexture.SampleLevel(LinearSampler, output.UVs, 0);
    
    localPos += displacementData;
    
    float3 worldPosition = mul(float4(localPos, 1.0), m_WorldMatrix).xyz;
    
    output.WorldPosition = worldPosition;
    output.ViewVector = m_CameraPosition - worldPosition;
    output.EyePos = m_CameraPosition;
    output.Position = mul(float4(worldPosition, 1.0), m_ViewProjectionMatrix);
    
    return output;
}