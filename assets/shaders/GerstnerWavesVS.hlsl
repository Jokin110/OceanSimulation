
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

struct WaveResults
{
    float3 m_FinalPos;
    float3 m_Normal;
    float m_Jacobian;
};

WaveResults CalculateWavePosition(float3 originalPos)
{
    WaveResults results = (WaveResults) 0;
    
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

VSOutput Main(VSInput input)
{
    VSOutput output = (VSOutput) 0;
    
    float3 worldPosition = mul(float4(input.position, 1.0), m_WorldMatrix).xyz;
    
    WaveResults results = CalculateWavePosition(worldPosition);
    worldPosition = results.m_FinalPos;
    
    output.worldPosition = worldPosition;
    output.normalWS = results.m_Normal;
    output.viewVector = m_CameraPosition - worldPosition;
    output.eyePos = m_CameraPosition;
    output.jacobian = results.m_Jacobian;
    output.color = input.color;
    output.position = mul(float4(worldPosition, 1.0), m_ViewProjectionMatrix);
    
    return output;
}