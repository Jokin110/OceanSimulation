#define PI 3.14159265359

struct DSInput
{
    float3 Position : POSITION;
    float3 Color : COLOR;
};

struct DSOutput
{
    float3 WorldPosition : TEXCOORD0;
    float2 UVs[4] : TEXCOORD1;
    float3 ViewVector : TEXCOORD5;
    float3 EyePos : TEXCOORD6;
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
    row_major matrix m_ViewProjectionMatrix;
    
    float4 m_PatchSize; // Size of the ocean patch in world units
    
    float3 m_CameraPosition;
    
    float m_Padding;
}

Texture2D DisplacementTextureCascade[4] : register(t0);
SamplerState LinearSampler : register(s0);

struct WaveResults
{
    float3 m_FinalPos;
    float3 m_Normal;
    float m_Jacobian;
};

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
    
    // Sample the FFT texture you generated in the Compute Shader
    for (int i = 0; i < 4; i++)
    {
        output.UVs[i] = flatWorldPos.xz / m_PatchSize[i];
        
        float4 displacementData = DisplacementTextureCascade[i].SampleLevel(LinearSampler, output.UVs[i], 0);
    
        localPos += displacementData.xyz;
    }
    
    float3 worldPosition = mul(float4(localPos, 1.0), m_WorldMatrix).xyz;
    
    output.WorldPosition = worldPosition;
    output.ViewVector = m_CameraPosition - worldPosition;
    output.EyePos = m_CameraPosition;
    output.Position = mul(float4(worldPosition, 1.0), m_ViewProjectionMatrix);
    
    return output;
}