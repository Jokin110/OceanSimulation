
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
    float3 eyePos : TEXCOORD3;
    float jacobian : JACOBIAN;
    float3 color : COLOR;
    float4 position : SV_POSITION;
};

struct WavesResults
{
    float height;
    float dx;
    float dz;
};

WavesResults CalculateWavePosition(float3 worldPosition)
{
    WavesResults results = (WavesResults) 0;
    
    float2 initialDirection = normalize(float2(1.0, 0.5));
    float initialAmplitude = 0.5;
    float initialFrequency = 0.3;
    float initialSpeed = 4;
    
    for (int i = 0; i < 32; i++)
    {
        float2 direction = normalize(float2(cos(PI / 4.0 * (i + 1) - PI / 3.0), sin(PI / 4.0 * (i + 1) - PI / 3.0)));
        float amplitude = initialAmplitude * pow(0.7, i);
        float frequency = initialFrequency * pow(1.18, i);
        float speed = initialSpeed * frequency;
        
        results.height += amplitude * sin(dot(direction, worldPosition.xz) * frequency + m_Time * speed);
    
        results.dx += amplitude * direction.x * frequency * cos(dot(direction, worldPosition.xz) * frequency + m_Time * speed);
        results.dz += amplitude * direction.y * frequency * cos(dot(direction, worldPosition.xz) * frequency + m_Time * speed);
    }

    return results;
}

VSOutput Main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    
    float3 worldPosition = mul(float4(input.position, 1.0), m_WorldMatrix).xyz;
    
    WavesResults results = CalculateWavePosition(worldPosition);
    
    worldPosition.y += results.height;
    
    float3 normal = normalize(float3(-results.dx, 1.0f, -results.dz));
    
    output.worldPosition = worldPosition;
    output.normalWS = normal;
    output.viewVector = m_CameraPosition - worldPosition;
    output.eyePos = m_CameraPosition;
    output.jacobian = 1;
    output.color = input.color;
    output.position = mul(float4(worldPosition, 1.0), m_ViewProjectionMatrix);
    
    return output;
}