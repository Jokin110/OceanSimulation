
#define PI 3.14159265359

cbuffer PerObjectBuffer : register(b0)
{
    row_major matrix m_WorldMatrix;
    row_major matrix m_InverseTransposeWorldMatrix;
    row_major matrix m_ViewProjectionMatrix;
    
    float m_Time;
    float3 m_CameraPosition;
}

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float3 color : COLOR;
};

struct VSOutput
{
    float3 worldPosition : TEXCOORD0;
    float3 normalWS : TEXCOORD1;
    float3 viewVector : TEXCOORD2;
    float3 color : COLOR;
    float4 position : SV_POSITION;
};

struct WaveVariables
{
    float2 m_Direction;
    float m_Length;
    float m_Steepness;
};

float3 CalculateWavePosition(float3 originalPos)
{
    // Accumulators - Start with the base position
    float3 finalPosition = originalPos;
    finalPosition.y = 0; // Reset Y to accumulate height
    
    WaveVariables waves[3];
    
    waves[0].m_Direction = normalize(float2(1.0, 0.5));
    waves[0].m_Length = 35.0f;
    waves[0].m_Steepness = 0.35f;
    
    waves[1].m_Direction = normalize(float2(0.6, 1.0));
    waves[1].m_Length = 14.5f;
    waves[1].m_Steepness = 0.25f;
    
    waves[2].m_Direction = normalize(float2(-0.4, 0.8));
    waves[2].m_Length = 6.0f;
    waves[2].m_Steepness = 0.15f;
    
    float w0 = 2 * PI / 200;

    for (int i = 0; i < 3; i++)
    {
        float k = 2 * PI / waves[i].m_Length;
        float w = sqrt(9.8 * k);
        w = floor(w / w0) * w0;
        float a = waves[i].m_Steepness / k;
        
        float2 wavevector = normalize(waves[i].m_Direction) * k;
        
        finalPosition.xz -= normalize(waves[i].m_Direction) * a * sin(dot(wavevector, originalPos.xz) - w * m_Time);
        finalPosition.y += a * cos(dot(wavevector, originalPos.xz) - w * m_Time);
    }
    
    return finalPosition;
}

VSOutput Main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    
    float3 worldPosition = mul(float4(input.position, 1.0), m_WorldMatrix).xyz;
    
    float3 binormal = CalculateWavePosition(worldPosition + float3(0.1, 0, 0));
    float3 tangent = CalculateWavePosition(worldPosition + float3(0, 0, 0.1));
    
    worldPosition = CalculateWavePosition(worldPosition);
    float3 normal = normalize(cross(normalize(tangent - worldPosition), normalize(binormal - worldPosition)));
    
    output.worldPosition = worldPosition;
    output.normalWS = normalize(mul(float4(normal, 0.0), m_InverseTransposeWorldMatrix).xyz);
    output.viewVector = normalize(m_CameraPosition - worldPosition);
    output.color = input.color;
    output.position = mul(float4(worldPosition, 1.0), m_ViewProjectionMatrix);
    
    return output;
}