
cbuffer PixelShaderBuffer : register(b0)
{
    float3 m_SunDir;
    float m_SunExponent;
    float3 m_SunColor;
    float m_SunBias;
}

struct PSInput
{
    float3 m_LocalPosition : TEXCOORD0;
    float4 m_Position : SV_POSITION;
};

TextureCube SkyBoxTexture : register(t0);
SamplerState LinearSampler : register(s0);

float4 Main(PSInput input) : SV_TARGET
{
    float4 skyColor = SkyBoxTexture.Sample(LinearSampler, input.m_LocalPosition);
    
    if (m_SunBias >= 1.0f)
        return skyColor;
    
    float sunValue = pow(saturate(dot(normalize(m_SunDir), normalize(-input.m_LocalPosition))), m_SunExponent);
    sunValue = saturate((sunValue - m_SunBias) / (1.0f - m_SunBias));
    
    float3 finalColor = skyColor.xyz + m_SunColor * sunValue;
    
    return float4(finalColor, 1.0f);
}